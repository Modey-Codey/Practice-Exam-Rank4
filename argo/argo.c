#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h> // change this to <stdlib.h>


typedef struct	json {
	enum {
		MAP,
		INTEGER,
		STRING
	} type;
	union {
		struct {
			struct pair	*data;
			size_t		size;
		} map;
		int	integer;
		char	*string;
	};
}	json;

typedef struct	pair {
	char	*key;
	json	value;
}	pair;

void	free_json(json j);
int	argo(json *dst, FILE *stream);

int	peek(FILE *stream)
{
	int	c = getc(stream);
	ungetc(c, stream);
	return c;
}

void	unexpected(FILE *stream)
{
	if (peek(stream) != EOF)
		printf("unexpected token '%c'\n", peek(stream));
	else
		printf("unexpected end of input\n");
}

int	accept(FILE *stream, char c)
{
	if (peek(stream) == c)
	{
		(void)getc(stream);
		return 1;
	}
	return 0;
}

int	expect(FILE *stream, char c)
{
	if (accept(stream, c))
		return 1;
	unexpected(stream);
	return 0;
}

void	free_json(json j)
{
	switch (j.type)
	{
		case MAP:
			for (size_t i = 0; i < j.map.size; i++)
			{
				free(j.map.data[i].key);
				free_json(j.map.data[i].value);
			}
			free(j.map.data);
			break ;
		case STRING:
			free(j.string);
			break ;
		default:
			break ;
	}
}

int parse_int(json *j, FILE *f)
{
	if (fscanf(f, "%d", &j->integer) != 1)
	{
		unexpected(f);
		return (-1);
	}
	j->type = INTEGER;
	return (1);
}

char *get_str(FILE *f)
{
	char *s, *tmp;
	size_t i = 0;
	int c, n;

	if (!expect(f, '"') || !(s = malloc(1)))
		return NULL;
	while ((c = getc(f)) != '"') 
	{
		if (c == EOF) 
		{
			printf("unexpected end of input\n");
			free(s);
			return NULL;
		}
		if (c == '\\') 
		{
			n = getc(f);
			if (n != '\\' && n != '"') 
			{
				if (n == EOF)
					printf("unexpected end of input\n");
				else 
				{
					ungetc(n, f);
					unexpected(f);
				}
				free(s);
				return NULL;
			}
			c = n;
		}
		if (!(tmp = realloc(s, i + 2))) 
		{
			free(s);
			return NULL;
		}
		s = tmp;
		s[i++] = c;
	}
	s[i] = '\0';
	return s;
}

int parse_map(json *j, FILE *f)
{
	pair	*tmp, *p;
	int	c;

	j->type = MAP;
	j->map.data = NULL;
	j->map.size = 0;
	if (!expect(f, '{'))
		return (-1);
	if (accept(f, '}'))
		return (1);
	while (1)
	{
		if (peek(f) != '"')
		{
			unexpected(f);
			return (-1);
		}
		tmp = realloc(j->map.data, (j->map.size + 1) * sizeof(pair));
		if (!tmp)
			return (-1);
		j->map.data = tmp;
		p = &j->map.data[j->map.size];
		p->key = get_str(f);
		if (!p->key)
			return (-1);
		j->map.size++;
		if (!expect(f, ':') || argo(&p->value, f) == -1)
			return (-1);
		c = peek(f);
		if (c == '}')
		{
			accept(f, '}');
			return (1);
		} 
		if (!accept(f, ','))
		{
			unexpected(f);
			return(-1);
		}
	}
}

int	argo(json *j, FILE *f)
{
	int c = peek(f);

	if (c == EOF)
	{
		unexpected(f);
		return (-1);
	}
	if (isdigit(c) || c == '-')
		return (parse_int(j, f));
	if (c == '"')
	{
		j->type = STRING;
		j->string = get_str(f);
		return (j->string ? 1 : -1);
	}
	if (c == '{')
		return (parse_map(j, f));
	unexpected(f);
	return (-1);
}

void	serialize(json j)
{
	switch (j.type)
	{
		case INTEGER:
			printf("%d", j.integer);
			break ;
		case STRING:
			putchar('"');
			for (int i = 0; j.string[i]; i++)
			{
				if (j.string[i] == '\\' || j.string[i] == '"')
					putchar('\\');
				putchar(j.string[i]);
			}
			putchar('"');
			break ;
		case MAP:
			putchar('{');
			for (size_t i = 0; i < j.map.size; i++)
			{
				if (i != 0)
					putchar(',');
				serialize((json){.type = STRING, .string = j.map.data[i].key});
				putchar(':');
				serialize(j.map.data[i].value);
			}
			putchar('}');
			break ;
	}
}

int	main(int argc, char **argv)
{
	if (argc != 2)
		return 1;
	char *filename = argv[1];
	FILE *stream = fopen(filename, "r");
	json	file;
	if (argo (&file, stream) != 1)
	{
		free_json(file);
		return 1;
	}
	serialize(file);
	printf("\n");
}
