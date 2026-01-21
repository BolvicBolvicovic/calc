#include <evaluator.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define _USE_MATH_DEFINES
#include <math.h>

static s32	evaluator_memcmp(token_t* t1, token_t* t2);
static u64	evaluator_hash(token_t* t);

SWISSMAP_DEFINE_FUNCTIONS(variables_map, token_t*, return_value_t, evaluator_hash, evaluator_memcmp)

static inline s32
evaluator_memcmp(token_t* t1, token_t* t2)
{
	if (t1->length != t2->length)
		return 0;

	if (memcmp(t1->start, t2->start, t1->length) != 0)
		return 0;

	return 1;
}

static inline u64
evaluator_hash(token_t* t)
{
	u64	hash = 5381;

	for (u64 i = 0; i < t->length; i++)
		hash = ((hash << 5) + hash) + t->start[i];

	return hash;
}

static inline f64
return_value_as_float(return_value_t* v)
{
	if (v->type == RET_FLOAT)
		return v->f;

	return (f64)v->i;
}

static inline s64
return_value_as_int(return_value_t* v)
{
	if (v->type == RET_FLOAT)
		return (s64)v->f;

	return v->i;
}

static inline void
return_value_cast_to_float(return_value_t* v)
{
	if (v->type == RET_INT)
	{
		v->type = RET_FLOAT;
		v->f = (f64)v->i;
	}
}

static inline void
return_value_convert_to_nano(return_value_t* v)
{
	return_value_cast_to_float(v);

	switch (v->oom)
	{
	case OOM_BASE:
		v->f *= 1000000000;
		break;
	case OOM_DECI:
		v->f *= 100000000;
		break;
	case OOM_CENTI:
		v->f *= 10000000;
		break;
	case OOM_MILLI:
		v->f *= 1000000;
		break;
	case OOM_MICRO:
		v->f *= 1000;
		break;
	default:
	}
	
	v->oom = OOM_NANO;
}

static inline void
return_value_convert_to_micro(return_value_t* v)
{
	return_value_cast_to_float(v);

	switch (v->oom)
	{
	case OOM_BASE:
		v->f *= 1000000;
		break;
	case OOM_DECI:
		v->f *= 100000;
		break;
	case OOM_CENTI:
		v->f *= 10000;
		break;
	case OOM_MILLI:
		v->f *= 1000;
		break;
	case OOM_NANO:
		v->f *= 0.001;
		break;
	default:
	}
	
	v->oom = OOM_MICRO;
}

static inline void
return_value_convert_to_milli(return_value_t* v)
{
	return_value_cast_to_float(v);

	switch (v->oom)
	{
	case OOM_BASE:
		v->f *= 1000;
		break;
	case OOM_DECI:
		v->f *= 100;
		break;
	case OOM_CENTI:
		v->f *= 10;
		break;
	case OOM_MICRO:
		v->f *= 0.001;
		break;
	case OOM_NANO:
		v->f *= 0.000001;
		break;
	default:
	}
	
	v->oom = OOM_MILLI;
}

static inline void
return_value_convert_to_centi(return_value_t* v)
{
	return_value_cast_to_float(v);

	switch (v->oom)
	{
	case OOM_BASE:
		v->f *= 100;
		break;
	case OOM_DECI:
		v->f *= 10;
		break;
	case OOM_MILLI:
		v->f *= 100;
		break;
	case OOM_MICRO:
		v->f *= 0.0001;
		break;
	case OOM_NANO:
		v->f *= 0.0000001;
		break;
	default:
	}
	
	v->oom = OOM_CENTI;
}

static inline void
return_value_convert_to_deci(return_value_t* v)
{
	return_value_cast_to_float(v);

	switch (v->oom)
	{
	case OOM_BASE:
		v->f *= 10;
		break;
	case OOM_CENTI:
		v->f *= 0.1;
		break;
	case OOM_MILLI:
		v->f *= 0.01;
		break;
	case OOM_MICRO:
		v->f *= 0.00001;
		break;
	case OOM_NANO:
		v->f *= 0.00000001;
		break;
	default:
	}
	
	v->oom = OOM_DECI;
}

static inline void
return_value_convert_to_base(return_value_t* v)
{
	return_value_cast_to_float(v);

	switch (v->oom)
	{
	case OOM_DECI:
		v->f *= 0.1;
       		break;
	case OOM_CENTI:
       		v->f *= 0.01;
       		break;
	case OOM_MILLI:
		v->f *= 0.001;
		break;
	case OOM_MICRO:
		v->f *= 0.000001;
		break;
	case OOM_NANO:
		v->f *= 0.000000001;
		break;
	default:
	}
	
	v->oom = OOM_BASE;
}
static inline void
return_value_convert_oom(return_value_t* v, order_of_magnetude_t oom)
{
	switch (oom)
	{
	case OOM_BASE:
		return_value_convert_to_base(v);
		break;
	case OOM_DECI:
		return_value_convert_to_deci(v);
       		break;
	case OOM_CENTI:
       		return_value_convert_to_centi(v);
       		break;
	case OOM_MILLI:
		return_value_convert_to_milli(v);
		break;
	case OOM_MICRO:
		return_value_convert_to_micro(v);
		break;
	case OOM_NANO:
		return_value_convert_to_nano(v);
		break;
	default:
	}
}

static inline void
return_value_convert_unit(return_value_t* v, unit_t u)
{
	if (v->unit == U_NONE)
		v->unit = u;
	else
		v->unit = U_ERR;
}

static f64
evaluator_atof(token_t* token)
{
	f64	res = 0.0;
	u32	i = 0;

	for (; i < token->length && token->start[i] != '.'; i++)
		res = res * 10.0 + (token->start[i] - '0');

	if (i >= token->length || token->start[i] != '.')
		return res;

	i++;

	f64	frac = 0.1;

	for (; i < token->length; i++)
	{
		res += frac * (token->start[i] - '0');
		frac *= 0.1;
	}

	return res;
}

static s64
evaluator_atoi(token_t* token)
{
	s64	res = 0;

	for (u32 i = 0; i < token->length; i++)
		res = res * 10.0 + (token->start[i] - '0');

	return res;
}

return_value_t*
evaluate(arena_t* arena, ast_node_t* node, arena_t* arena_vmap, variables_map* vmap)
{
	assert(arena);
	assert(node);

	return_value_t*	ret = ARENA_PUSH_STRUCT(arena, return_value_t);
	
	ret->type	= RET_INT;
	ret->unit	= U_NONE;
	ret->oom	= OOM_NONE;
	ret->token	= &node->token;
	
	switch (node->type)
	{
	case EXPR_ID:
		if (node->left)
		{
			// Built-in or function
			ret = evaluate(arena, node->left, arena_vmap, vmap);

			switch (node->token.length)
			{
			case 3:
				if (memcmp("ohm", node->token.start, 3) == 0)
					return_value_convert_unit(ret, U_OHM);
				else
					ret->unit = U_UNKNOWN;
				break;
			case 4:
				if (memcmp("volt", node->token.start, 4) == 0)
					return_value_convert_unit(ret, U_VOLT);
				else if (memcmp("watt", node->token.start, 4) == 0)
					return_value_convert_unit(ret, U_WATT);
				else if (memcmp("nano", node->token.start, 4) == 0)
					return_value_convert_to_nano(ret);
				else if (memcmp("deci", node->token.start, 4) == 0)
					return_value_convert_to_deci(ret);
				else
					ret->unit = U_UNKNOWN;
				break;
			case 5:
				if (memcmp("micro", node->token.start, 5) == 0)
					return_value_convert_to_micro(ret);
				else if (memcmp("milli", node->token.start, 5) == 0)
					return_value_convert_to_milli(ret);
				else if (memcmp("centi", node->token.start, 5) == 0)
					return_value_convert_to_centi(ret);
				else
					ret->unit = U_UNKNOWN;
				break;
			case 6:
				if (memcmp("ampere", node->token.start, 6) == 0)
					return_value_convert_unit(ret, U_AMPERE);
				else
					ret->unit = U_UNKNOWN;
				break;
			case 14:
				if (memcmp("base_magnitude", node->token.start, 14) == 0)
					return_value_convert_to_base(ret);
				else
					ret->unit = U_UNKNOWN;
				break;
			default:
				ret->unit = U_UNKNOWN;
			} // switch (node->token.length)
			
			break;
		}
		else
		{
			// Constants
			switch (node->token.length)
			{
			case 1:
				if (*node->token.start == 'E')
				{
					ret->type = RET_FLOAT;
					ret->f = M_E;
				}
				else
					goto def;
				break;
			case 2:
				if (memcmp("PI", node->token.start, 2) == 0)
				{
					ret->type = RET_FLOAT;
					ret->f = M_PI;
				}
				else
					goto def;
				break;
			case 4:
				if (memcmp("exit", node->token.start, 4) == 0)
					exit(0);
				else
					goto def;
				break;
			// Variables
			default:
			def:
				return_value_t*	var = variables_map_get(vmap, &node->token);

				if (var)
					ret = var;
				else
					ret->type = RET_BINDABLE;
			}
		}

		break;
	case EXPR_CONST:
		switch (node->token.symbol)
		{
		case TK_INT:
			ret->i = evaluator_atoi(&node->token);
			break;
		case TK_FLOAT:
			ret->type = RET_FLOAT;
			ret->f = evaluator_atof(&node->token);
			break;
		default:
			ret->type = RET_ERR;
			break;
		}
		break;
	case EXPR_UOP:
		switch (node->token.symbol)
		{
		case TK_SUB:
			ret = evaluate(arena, node->left, arena_vmap, vmap);

			if (ret->type == RET_INT)
				ret->i = -ret->i;
			else if (ret->type == RET_FLOAT)
				ret->f = -ret->f;
			break;
		default:
			ret->type = RET_ERR;
			break;
		}
		break;
	case EXPR_BOP:
		return_value_t*	l = evaluate(arena, node->left, arena_vmap, vmap);
		return_value_t* r = evaluate(arena, node->right, arena_vmap, vmap);

		if (l->type == RET_ERR || r->type == RET_ERR)
		{
			ret->type = RET_ERR;
			break;
		}
		
		if (l->type == RET_FLOAT || r->type == RET_FLOAT)
			ret->type = RET_FLOAT;

		switch (node->token.symbol)
		{
		case TK_BIND:
			if (l->type == RET_BINDABLE || r->type == RET_BINDABLE)
			{
				return_value_t*	var	= l->type == RET_BINDABLE ? l : r;
				return_value_t*	val	= l->type != RET_BINDABLE ? l : r;
				token_t*	tok	= ARENA_PUSH_STRUCT(arena_vmap, token_t);
				char*		tok_str	= ARENA_PUSH_ARRAY(
								arena_vmap, char, var->token->length);

				memcpy(tok, var->token, sizeof(token_t));
				memcpy(tok_str, tok->start, tok->length);
				tok->start = tok_str;
				variables_map_put(vmap, tok, *val);
				ret = val;
			}
			else
				ret->type = RET_ERR;

			break;
		case TK_ADD:
			if ((l->unit && r->unit && l->unit != r->unit) || l->unit >= U_ERR)
				ret->unit = U_ERR;
			else if (l->unit == r->unit)
				ret->unit = l->unit;
			else
				ret->unit = l->unit + r->unit;

			if (l->oom < r->oom)
			{
				if (r->oom == OOM_NONE && r->unit != U_NONE)
					r->oom = OOM_BASE;

				if (r->oom != OOM_NONE)
					return_value_convert_oom(r, l->oom);

				ret->oom = l->oom;
			}
			else
			{
				if (l->oom == OOM_NONE && l->unit != U_NONE)
					l->oom = OOM_BASE;

				if (l->oom != OOM_NONE)
					return_value_convert_oom(l, r->oom);

				ret->oom = r->oom;
			}

			if (ret->type == RET_INT)
				ret->i = return_value_as_int(l) + return_value_as_int(r);
			else
				ret->f = return_value_as_float(l) + return_value_as_float(r);
			break;
		case TK_SUB:
			if ((l->unit && r->unit && l->unit != r->unit) || l->unit >= U_ERR)
				ret->unit = U_ERR;
			else if (l->unit == r->unit)
				ret->unit = l->unit;
			else
				ret->unit = l->unit + r->unit;

			if (l->oom < r->oom)
			{
				if (r->oom == OOM_NONE && r->unit != U_NONE)
					r->oom = OOM_BASE;

				if (r->oom != OOM_NONE)
					return_value_convert_oom(r, l->oom);

				ret->oom = l->oom;
			}
			else
			{
				if (l->oom == OOM_NONE && l->unit != U_NONE)
					l->oom = OOM_BASE;

				if (l->oom != OOM_NONE)
					return_value_convert_oom(l, r->oom);

				ret->oom = r->oom;
			}

			if (ret->type == RET_INT)
				ret->i = return_value_as_int(l) - return_value_as_int(r);
			else
				ret->f = return_value_as_float(l) - return_value_as_float(r);
			break;
		case TK_DIV:
			if (r->i == 0)
			{
				ret->type = RET_ERR;
				break;
			}

			order_of_magnetude_t	m_div;

			if (l->oom < r->oom)
				m_div = l->oom;
			else
				m_div = r->oom;

			return_value_convert_oom(l, OOM_BASE);
			return_value_convert_oom(r, OOM_BASE);
			ret->oom = OOM_BASE;
			
			switch (l->unit)
			{
			case U_ERR:
			case U_UNSUPPORTED:
				ret->unit = l->unit;
				break;
			case U_NONE:
				ret->unit = r->unit;
				break;
			case U_AMPERE:
				switch (r->unit)
				{
				case U_ERR:
				case U_UNSUPPORTED:
					ret->unit = r->unit;
					break;
				case U_AMPERE:
					break;
				case U_NONE:
					ret->unit = U_AMPERE;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			case U_OHM:
				switch (r->unit)
				{
				case U_ERR:
				case U_UNSUPPORTED:
					ret->unit = r->unit;
					break;
				case U_OHM:
					break;
				case U_NONE:
					ret->unit = U_OHM;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			case U_VOLT:
				switch (r->unit)
				{
				case U_ERR:
				case U_UNSUPPORTED:
					ret->unit = r->unit;
					break;
				case U_AMPERE:
					ret->unit = U_OHM;
					break;
				case U_OHM:
					ret->unit = U_AMPERE;
					break;
				case U_VOLT:
					break;
				case U_NONE:
					ret->unit = U_VOLT;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			case U_WATT:
				switch (r->unit)
				{
				case U_ERR:
				case U_UNSUPPORTED:
					ret->unit = r->unit;
					break;
				case U_AMPERE:
					ret->unit = U_VOLT;
					break;
				case U_VOLT:
					ret->unit = U_AMPERE;
					break;
				case U_WATT:
					break;
				case U_NONE:
					ret->unit = U_WATT;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			default:
				ret->unit = U_UNSUPPORTED;
			} //switch (l->unit)

			if (ret->type == RET_INT)
				ret->i = return_value_as_int(l) / return_value_as_int(r);
			else
				ret->f = return_value_as_float(l) / return_value_as_float(r);
			
			return_value_convert_oom(ret, m_div);

			break;
		case TK_POW:
			order_of_magnetude_t	m_pow;

			if (l->oom < r->oom)
				m_pow = l->oom;
			else
				m_pow = r->oom;

			return_value_convert_oom(l, OOM_BASE);
			return_value_convert_oom(r, OOM_BASE);
			ret->oom = OOM_BASE;

			if ((l->unit && r->unit && l->unit != r->unit) || l->unit >= U_ERR)
				ret->unit = U_ERR;
			else
				ret->unit = l->unit + r->unit;

			if (ret->type == RET_INT)
				ret->i = (s64)pow(return_value_as_int(l), return_value_as_int(r));
			else
				ret->f = pow(return_value_as_float(l), return_value_as_float(r));

			return_value_convert_oom(ret, m_pow);
			break;
		case TK_MUL:
			order_of_magnetude_t	m_mul;

			if (l->oom < r->oom)
				m_mul = l->oom;
			else
				m_mul = r->oom;

			return_value_convert_oom(l, OOM_BASE);
			return_value_convert_oom(r, OOM_BASE);
			ret->oom = OOM_BASE;

			switch (l->unit)
			{
			case U_ERR:
			case U_UNSUPPORTED:
				ret->unit = l->unit;
				break;
			case U_NONE:
				ret->unit = r->unit;
				break;
			case U_VOLT:
				switch (r->unit)
				{
				case U_ERR:
				case U_UNSUPPORTED:
					ret->unit = r->unit;
					break;
				case U_AMPERE:
					ret->unit = U_WATT;
					break;
				case U_NONE:
					ret->unit = U_VOLT;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			case U_AMPERE:
				switch (r->unit)
				{
				case U_ERR:
				case U_UNSUPPORTED:
					ret->unit = r->unit;
					break;
				case U_VOLT:
					ret->unit = U_WATT;
					break;
				case U_OHM:
					ret->unit = U_VOLT;
					break;
				case U_NONE:
					ret->unit = U_AMPERE;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			case U_OHM:
				switch (r->unit)
				{
				case U_ERR:
				case U_UNSUPPORTED:
					ret->unit = r->unit;
					break;
				case U_AMPERE:
					ret->unit = U_VOLT;
					break;
				case U_NONE:
					ret->unit = U_AMPERE;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			case U_WATT:
				switch (r->unit)
				{
				case U_ERR:
				case U_UNSUPPORTED:
					ret->unit = r->unit;
					break;
				case U_NONE:
					ret->unit = U_WATT;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			default:
				ret->unit = U_UNSUPPORTED;
			} // switch (l->unit)

			if (ret->type == RET_INT)
				ret->i = return_value_as_int(l) * return_value_as_int(r);
			else
				ret->f = return_value_as_float(l) * return_value_as_float(r);
			
			return_value_convert_oom(ret, m_mul);
			break;
		default:
			ret->type = RET_ERR;
			break;
		} // switch (node->token.symbol)
		break;
	default:
	} // switch (node->type)

	return ret;
}

void
evaluator_print_res(return_value_t* res)
{
	switch (res->type)
	{
	case RET_INT:
		printf("%ld", res->i);
		break;
	case RET_FLOAT:
		printf("%g", res->f);
		break;
	case RET_ERR:
		printf("Error while evaluating expression\n");
		return;
	default:
		printf("\n");
		return;
	}

	switch (res->oom)
	{
		case OOM_NANO:
			printf(" nano");
			break;
		case OOM_MICRO:
			printf(" micro");
			break;
		case OOM_MILLI:
			printf(" milli");
			break;
		case OOM_CENTI:
			printf(" centi");
			break;
		case OOM_DECI:
			printf(" deci");
			break;
		default:
	}

	switch (res->unit)
	{
	case U_NONE:
		printf("\n");
		break;
	case U_VOLT:
		printf(" volt\n");
		break;
	case U_AMPERE:
		printf(" ampere\n");
		break;
	case U_WATT:
		printf(" watt\n");
		break;
	case U_OHM:
		printf(" ohm\n");
		break;
	case U_UNSUPPORTED:
		printf(" unsupported unit\n");
		break;
	case U_UNKNOWN:
		printf(" unknown function\n");
		break;
	case U_ERR:
		printf(" erronous unit\n");
		break;
	}
}

#ifdef TESTER

void
test_evaluator_atoi(void)
{
	char*	test	= "25634";
	token_t	tok	=
	{
		.start	= test,
		.length	= 5,
	};

	assert(evaluator_atoi(&tok) == 25634);
}

void
test_evaluator_atof(void)
{
	char*	test	= "256.34";
	f64	eps	= 0.000001;
	token_t	tok	=
	{
		.start	= test,
		.length	= 6,
	};

	assert(evaluator_atof(&tok) >= 256.34 - eps && evaluator_atof(&tok) <= 256.34 + eps);
}

void
test_evaluate(void)
{
	char*		test	= "-5 - 10.0 * 5";
	arena_t*	arena	= ARENA_ALLOC();
	variables_map*	vmap	= variables_map_new(arena, 100);
	lexer_t		lexer;
	ast_node_t*	tree	= 0;
	return_value_t*	res	= 0;
	f64		eps	= 0.000001;

	lexer_init(&lexer, test, 13);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_FLOAT);
	assert(res->f >= -55.0 - eps && res->f <= -55.0 + eps);
	
	char*		test2	= "-5 - 10.0 * (5 ^ 2)";
	
	lexer_init(&lexer, test2, 19);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_FLOAT);
	assert(res->f >= -255.0 - eps && res->f <= -255.0 + eps);
	
	char*		test3	= "ampere(5)";
	
	lexer_init(&lexer, test3, 9);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_INT);
	assert(res->i == 5);
	assert(res->unit == U_AMPERE);
	assert(res->oom == OOM_NONE);
	
	char*		test4	= "milli(volt(5)) + base_magnitude(volt(5))";
	
	lexer_init(&lexer, test4, 40);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_FLOAT);
	assert(res->f >= 5005.0 - eps && res->f <= 5005.0 + eps);
	assert(res->unit == U_VOLT);
	assert(res->oom == OOM_MILLI);

	char*		test5	= "milli(volt(5)) + volt(5)";
	
	lexer_init(&lexer, test5, 24);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_FLOAT);
	assert(res->f >= 5005.0 - eps && res->f <= 5005.0 + eps);
	assert(res->unit == U_VOLT);
	assert(res->oom == OOM_MILLI);

	char*		test6	= "milli(volt(5)) * milli(ampere(5))";
	
	lexer_init(&lexer, test6, 33);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_FLOAT);
	assert(res->f >= 0.025 - eps && res->f <= 0.025 + eps);
	assert(res->unit == U_WATT);
	assert(res->oom == OOM_MILLI);
	
	char*	test7	= "x :: 10";
	token_t	x	= { TK_ID, test7, 1, 1 };

	lexer_init(&lexer, test7, 33);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);
	
	assert(res->type == RET_INT);
	assert(res->i == 10);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);
	assert(variables_map_get(vmap, &x)->i == 10);

	char*		test8	= "x + 10";

	lexer_init(&lexer, test8, 33);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);
	
	assert(res->type == RET_INT);
	assert(res->i == 20);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);

	char*		test9	= "10 + x";

	lexer_init(&lexer, test9, 33);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_INT);
	assert(res->i == 20);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);

	char*	test10	= "5 :: y";
	token_t	y	= { TK_ID, test10 + 5, 1, 1 };

	lexer_init(&lexer, test10, 33);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);
	
	assert(res->type == RET_INT);
	assert(res->i == 5);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);
	assert(variables_map_get(vmap, &y)->i == 5);

	char*	test11	= "x + y";

	lexer_init(&lexer, test11, 33);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_INT);
	assert(res->i == 15);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);

	arena_release(arena);
}

#endif // TESTER
