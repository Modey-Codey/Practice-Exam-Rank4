#include "vbc.h"

int	accept(char **s, char c)
{
	if (**s == c)	//add == c
		(*s)++;
		return (1);
	}
	return (0);
}

node	*join(int type, node *l, node *r)
{
	node	tmp = {type, 0, l, r};
	node	*ret = new_node(tmp);

	if (!ret)
	{
		destroy_tree(l);
		destroy_tree(r);
	}
	return (ret);
}

node	*parse_add(char **s);

node	*parse_factor(char **s)
{
	node	*ret;
	node	tmp = {VAL, 0, NULL, NULL};

	if (isdigit(**s))
	{
		tmp.val = **s - '0';
		(*s)++;
		return (new_node(tmp));
	}
	if (accept(s, '('))
	{
		ret = parse_add(s);
		if (!ret)
			return (NULL);
		if (!expect(s, ')'))
			return (destroy_tree(ret), NULL);
		return (ret);
	}
	unexpected(**s);
	return (NULL);
}

node	*parse_mul(char **s)
{
	node	*l = parse_factor(s);
	node	*r;

	while (l && accept(s, '*'))
	{
		r = parse_factor(s);
		if (!r)
			return (destroy_tree(l), NULL);
		l = join(MULTI, l, r);
	}
	return (l);
}

node	*parse_add(char **s)
{
	node	*l = parse_mul(s);
	node	*r;

	while (l && accept(s, '+'))
	{
		r = parse_mul(s);
		if (!r)
			return (destroy_tree(l), NULL);
		l = join(ADD, l, r);
	}
	return (l);
}

node	*parse_expr(char *s)
{
	node	*ret = parse_add(&s); //add

	if (!ret) //add
		return (NULL); //add
	if (*s)
	{
		unexpected(*s); //add
		destroy_tree(ret);
		return (NULL);
	}
	return (ret);
}

int	eval_tree(node *tree)
{
	switch (tree->type)
	{
		case ADD:
			return (eval_tree(tree->l) + eval_tree(tree->r));
		case MULTI:
			return (eval_tree(tree->l) * eval_tree(tree->r));
		case VAL:
			return (tree->val);
	}
	return (0); //add
}