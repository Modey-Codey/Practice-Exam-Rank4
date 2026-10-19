int parse_int(json *j, FILE *f)
{
	if (fscanf(f, "%d", &j->integer) != 1) {
		unexpected(f);
		return -1;
	}
	j->type = INTEGER;
	return 1;
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
	pair *tmp, *p;
	int c;

	j->type = MAP;
	j->map.data = NULL;
	j->map.size = 0;
	if (!expect(f, '{'))
		return -1;
	if (accept(f, '}'))
		return 1;
	while (1) 
	{
		if (peek(f) != '"') 
		{
			unexpected(f);
			return -1;
		}
		tmp = realloc(j->map.data, (j->map.size + 1) * sizeof(pair));
		if (!tmp)
			return -1;
		j->map.data = tmp;
		p = &j->map.data[j->map.size];
		p->key = get_str(f);
		if (!p->key)
			return -1;
		j->map.size++;
		if (!expect(f, ':') || argo(&p->value, f) == -1)
			return -1;
		c = peek(f);
		if (c == '}') 
		{
			accept(f, '}');
			return 1;
		}
		if (!accept(f, ',')) 
		{
			unexpected(f);
			return -1;
		}
	}
}

int argo(json *j, FILE *f)
{
	int c = peek(f);

	if (c == EOF) 
	{
		unexpected(f);
		return -1;
	}
	if (isdigit(c) || c == '-')
		return parse_int(j, f);
	if (c == '"') 
	{
		j->type = STRING;
		j->string = get_str(f);
		return j->string ? 1 : -1;
	}
	if (c == '{')
		return parse_map(j, f);
	unexpected(f);
	return -1;
}