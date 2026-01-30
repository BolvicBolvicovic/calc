#include <evaluator.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <builtins.h>

#define _USE_MATH_DEFINES
#include <math.h>

static s32	evaluator_hash_memcmp(token_t* t1, token_t* t2);
static u64	evaluator_hash(token_t* t);

SWISSMAP_DEFINE_FUNCTIONS(variables_map, token_t*, return_value_t*, evaluator_hash, evaluator_hash_memcmp)

static inline void
evaluator_memcpy_value(arena_t* arena, return_value_t* dst, return_value_t* src)
{
	memcpy(dst, src, sizeof(return_value_t));
	
	while (src->next)
	{
		dst->next = ARENA_PUSH_STRUCT(arena, return_value_t);
		memcpy(dst->next, src->next, sizeof(return_value_t));
		dst = dst->next;
		src = src->next;
	}
}

static inline s32
evaluator_hash_memcmp(token_t* t1, token_t* t2)
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
	const u8*	buf = (const u8*)t->start;
	const u8*	end = buf + t->length;
	u64		hash= 5381;

	while (buf + 4 <= end)
	{
		hash = ((hash << 5) + hash) + buf[0];
		hash = ((hash << 5) + hash) + buf[1];
		hash = ((hash << 5) + hash) + buf[2];
		hash = ((hash << 5) + hash) + buf[3];

		buf += 4;
	}

	while (buf < end)
		hash = ((hash << 5) + hash) + *buf++;

	return hash;
}

static inline void
evaluator_print_res_val(return_value_t* res)
{
	switch (res->type)
	{
	case RET_FLOAT:
		printf("%g", res->f);
		break;
	case RET_COMPLEX:
		printf("%g + %gI", creal(res->c), cimag(res->c));
		break;
	case RET_ERR:
		error_print(res->token, res->err_code);
		return;
	default:
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
		break;
	case U_JOULE:
		printf(" joule");
		break;
	case U_SECOND:
		printf(" second");
		break;
	case U_VOLT:
		printf(" volt");
		break;
	case U_AMPERE:
		printf(" ampere");
		break;
	case U_WATT:
		printf(" watt");
		break;
	case U_OHM:
		printf(" ohm");
		break;
	case U_UNSUPPORTED:
		printf(" unsupported unit");
		break;
	case U_ERR:
		printf(" erronous unit");
		break;
	}
}

// START CAST & CONV RET_VAL

__always_inline f64
return_value_as_float(return_value_t* v)
{
	return v->type == RET_COMPLEX ? creal(v->c) : v->f;
}

static __always_inline double complex
return_value_as_complex(return_value_t* v)
{
	return v->type == RET_FLOAT ? v->f : v->c;
}

static inline void
return_value_convert_to_nano(return_value_t* v)
{
	if (v->type == RET_COMPLEX)
		return;

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
	if (v->type == RET_COMPLEX)
		return;

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
	if (v->type == RET_COMPLEX)
		return;

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
	if (v->type == RET_COMPLEX)
		return;

	switch (v->oom)
	{
	case OOM_BASE:
		v->f *= 100;
		break;
	case OOM_DECI:
		v->f *= 10;
		break;
	case OOM_MILLI:
		v->f *= 0.1;
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
	if (v->type == RET_COMPLEX)
		return;

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
	if (v->type == RET_COMPLEX)
		return;

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

inline void
return_value_convert_oom(return_value_t* v, order_of_magnetude_t oom)
{
	if (v->type == RET_COMPLEX)
		return;

	switch (oom)
	{
	case OOM_BASE:
		return_value_convert_to_base(v);
		break;
	case OOM_NONE:
		return_value_convert_to_base(v);
		v->oom = OOM_NONE;
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

// END CAST & CONV RET_VAL

static f64
evaluator_atof(token_t* token)
{
	f64		res = 0.0;
	u32		i   = 0;
	u32		len = token->length;
	const char*	buf = token->start;

	for (; i < len && buf[i] != '.'; i++)
		res = res * 10.0 + (buf[i] - '0');

	if (i >= len || buf[i] != '.')
		return res;

	i++;

	f64	frac = 0.1;

	for (; i < len; i++)
	{
		res += frac * (buf[i] - '0');
		frac *= 0.1;
	}

	return res;
}

return_value_t*
evaluate(arena_t* arena, ast_node_t* node, arena_t* arena_vmap, variables_map* vmap)
{
	return_value_t*	ret = ARENA_PUSH_STRUCT(arena, return_value_t);
	
	ret->unit	= U_NONE;
	ret->oom	= OOM_NONE;
	ret->next	= 0;

	if (!node)
	{
		ret->type	= RET_ERR;
		ret->err_code	= ERR_MISSING_OPERATOR_ARGUMENT;
		ret->token	= 0;
		return ret;
	}

	ret->type	= RET_FLOAT;
	ret->token	= &node->token;

	token_t*	token		= &node->token;
	const char*	token_buf	= token->start;
	u32		token_len	= token->length;
	
	switch (node->type)
	{
	case EXPR_ID:

		if (node->left)
		{
			// Built-in or function
			ret = evaluate(arena, node->left, arena_vmap, vmap);

			switch (token_len)
			{
			case 3:
				if (memcmp("ohm", token_buf, 3) == 0)
				{
					return_value_convert_unit(ret, U_OHM);
					return ret;
				}
				else if (memcmp("cos", token_buf, 3) == 0)
				{
					trigo_cos(ret);
					return ret;
				}
				else if (memcmp("sin", token_buf, 3) == 0)
				{
					trigo_sin(ret);
					return ret;
				}
				else if (memcmp("tan", token_buf, 3) == 0)
				{
					trigo_tan(ret);
					return ret;
				}
				break;
			case 4:
				if (memcmp("volt", token_buf, 4) == 0)
				{
					return_value_convert_unit(ret, U_VOLT);
					return ret;
				}
				else if (memcmp("watt", token_buf, 4) == 0)
				{
					return_value_convert_unit(ret, U_WATT);
					return ret;
				}
				else if (memcmp("nano", token_buf, 4) == 0)
				{
					return_value_convert_to_nano(ret);
					return ret;
				}
				else if (memcmp("deci", token_buf, 4) == 0)
				{
					return_value_convert_to_deci(ret);
					return ret;
				}
				else if (memcmp("sqrt", token_buf, 4) == 0)
				{
					roots_sqrt(ret);
					return ret;
				}
				else if (memcmp("cbrt", token_buf, 4) == 0)
				{
					roots_cbrt(ret);
					return ret;
				}
				break;
			case 5:
				if (memcmp("micro", token_buf, 5) == 0)
				{
					return_value_convert_to_micro(ret);
					return ret;
				}
				else if (memcmp("milli", token_buf, 5) == 0)
				{
					return_value_convert_to_milli(ret);
					return ret;
				}
				else if (memcmp("centi", token_buf, 5) == 0)
				{
					return_value_convert_to_centi(ret);
					return ret;
				}
				else if (memcmp("joule", token_buf, 5) == 0)
				{
					return_value_convert_unit(ret, U_JOULE);
					return ret;
				}
				break;
			case 6:
				if (memcmp("ampere", token_buf, 6) == 0)
				{
					return_value_convert_unit(ret, U_AMPERE);
					return ret;
				}
				else if (memcmp("arccos", token_buf, 6) == 0)
				{
					trigo_arccos(ret);
					return ret;
				}
				else if (memcmp("arcsin", token_buf, 6) == 0)
				{
					trigo_arcsin(ret);
					return ret;
				}
				else if (memcmp("arctan", token_buf, 6) == 0)
				{
					trigo_arctan(ret);
					return ret;
				}
				else if (memcmp("second", token_buf, 6) == 0)
				{
					return_value_convert_unit(ret, U_SECOND);
					return ret;
				}
				break;
			case 7:
				if (memcmp("current", token_buf, 7) == 0)
				{
					current(ret);
					return ret;
				}
				break;
			case 11:
				if (memcmp("polynom_one", token_buf, 11) == 0)
				{
					polynom_one(ret);
					return ret;
				}
				else if (memcmp("polynom_two", token_buf, 11) == 0)
				{
					polynom_two(arena, ret);
					return ret;
				}
				else if (memcmp("amp_divider", token_buf, 11) == 0)
				{
					current_divider(ret, U_AMPERE);
					return ret;
				}
				break;
			case 12:
				if (memcmp("polynom_four", token_buf, 12) == 0)
				{
					polynom_four(arena, ret);
					return ret;
				}
				else if (memcmp("res_parallel", token_buf, 12) == 0)
				{
					res_parallel(ret);
					return ret;
				}
				else if (memcmp("volt_divider", token_buf, 12) == 0)
				{
					current_divider(ret, U_VOLT);
					return ret;
				}
				break;
			case 13:
				if (memcmp("polynom_three", token_buf, 13) == 0)
				{
					polynom_three(arena, ret);
					return ret;
				}
				break;
			case 14:
				if (memcmp("base_magnitude", token_buf, 14) == 0)
				{
					return_value_convert_to_base(ret);
					return ret;
				}
				break;
			default:
			} // switch (node->token.length)

			return_value_t**	var = variables_map_get(vmap, token);
			
			if (!var || !*var)
			{
				ret->type	= RET_ERR;
				ret->err_code	= ERR_UNKNOWN_FUNC;
				return ret;
			}

			s64		idx	= 0;
			s64		i	= 0;
			return_value_t*	tmp	= *var;

			switch (ret->type)
			{
			case RET_FLOAT:
				idx = (s64)ret->f;
				break;
			case RET_COMPLEX:
				idx = (s64)creal(ret->c);
				break;
			default:
			}

			while (i < idx && tmp->next)
			{
				tmp = tmp->next;
				i++;
			}
			
			if (idx != i)
			{
				ret->type	= RET_ERR;
				ret->err_code	= ERR_OUT_OF_BOUND;
			}
			else
				memcpy(ret, tmp, sizeof(return_value_t));

			ret->next = 0;
			
			break;
		}
		else
		{
			// Constants
			switch (token_len)
			{
			case 1:
				if (*token_buf == 'E')
				{
					ret->type = RET_FLOAT;
					ret->f = M_E;
					return ret;
				}
				else if (*token_buf == 'I')
				{
					ret->type = RET_COMPLEX;
					ret->c = I;
					return ret;
				}
				break;
			case 2:
				if (memcmp("PI", token_buf, 2) == 0)
				{
					ret->type = RET_FLOAT;
					ret->f = M_PI;
					return ret;
				}
				break;
			case 4:
				if (memcmp("exit", token_buf, 4) == 0)
					exit(0);
				break;
			case 5:
				if (memcmp("clear", token_buf, 5) == 0)
				{
					printf("\033[2J\033[H");
					return 0;
				}
				break;
			default:
			}
			// Variables
			return_value_t**	var = variables_map_get(vmap, token);
				
			if (var)
				memcpy(ret, *var, sizeof(return_value_t));
			else
				ret->type = RET_BINDABLE;
		}

		break;
	case EXPR_CONST:
		ret->type	= RET_FLOAT;
		ret->f		= evaluator_atof(token);
		
		if (node->left)
		{
			s64	idx	= (s64)ret->f;
			s64	i	= 0;

			ret = evaluate(arena, node->left, arena_vmap, vmap);

			for (; i < idx && ret->next; i++, ret = ret->next);
			
			if (idx != i)
			{
				ret->type	= RET_ERR;
				ret->err_code	= ERR_OUT_OF_BOUND;
				return ret;
			}
		}
		break;
	case EXPR_UOP:
		switch (token->symbol)
		{
		case TK_SUB:
			ret = evaluate(arena, node->left, arena_vmap, vmap);

			if (ret->type == RET_FLOAT)
				ret->f = -ret->f;
			else if (ret->type == RET_COMPLEX)
				ret->c = -ret->c;
			break;
		default:
			ret->type	= RET_ERR;
			ret->err_code	= ERR_BINARY_OP_MISSING_LEFT;
			return ret;
		}
		break;
	case EXPR_BOP:
		return_value_t*	l = evaluate(arena, node->left, arena_vmap, vmap);
		return_value_t* r = evaluate(arena, node->right, arena_vmap, vmap);

		if (l->type == RET_ERR || r->type == RET_ERR)
		{
			l->token = &node->token;
			r->token = &node->token;
			return	l->type == RET_ERR ? l : r;
		}

		if (token->symbol != TK_BIND
			&& (l->type == RET_BINDABLE  || r->type == RET_BINDABLE))
		{
			ret->type	= RET_ERR;
			ret->err_code	= ERR_OPERATION_UNBOUND_VAR;
			return ret;
		}

		
		if (l->type == RET_COMPLEX || r->type == RET_COMPLEX)
			ret->type = RET_COMPLEX;

		switch (token->symbol)
		{
		case TK_LIST:
			// TODO: maybe add a tail to return_value_t
			return_value_t*	tail = l;

			while (tail->next)
				tail = tail->next;

			tail->next = r;
			ret = l;
			break;
		case TK_BIND:
			if (l->type == RET_BINDABLE && r->type == RET_BINDABLE)
			{
				ret->type	= RET_ERR;
				ret->err_code	= ERR_OPERATION_UNBOUND_VAR;
				return ret;
			}
			else if (l->type == RET_BINDABLE || r->type == RET_BINDABLE)
			{
				return_value_t*	var	= l->type == RET_BINDABLE ? l : r;
				return_value_t*	val	= l->type != RET_BINDABLE ? l : r;
				token_t*	tok	= ARENA_PUSH_STRUCT(arena_vmap, token_t);
				char*		tok_str	= ARENA_PUSH_ARRAY(
								arena_vmap, char, var->token->length);

				memcpy(tok, var->token, sizeof(token_t));
				memcpy(tok_str, tok->start, tok->length);
				tok->start = tok_str;

				return_value_t*	mapped_val = ARENA_PUSH_STRUCT(
								arena_vmap, return_value_t);
				evaluator_memcpy_value(arena_vmap, mapped_val, val);
				variables_map_put(vmap, tok, mapped_val);
				ret = val;
			}
			else
			{
				ret->type	= RET_ERR;
				ret->err_code	= ERR_BINDING_ALREADY_DEFINED;
				return ret;
			}

			break;
		case TK_ADD:
			order_of_magnetude_t	m_add = OOM_NONE;

			if (ret->type != RET_COMPLEX)
			{
				if ((l->unit && r->unit && l->unit != r->unit) || l->unit >= U_ERR)
					ret->unit = U_ERR;
				else if (l->unit == r->unit)
					ret->unit = l->unit;
				else
					ret->unit = l->unit + r->unit;

				if (!l->unit && l->oom == OOM_NONE)
					l->oom = r->oom;
				if (!r->unit && r->oom == OOM_NONE)
					r->oom = l->oom;

				if (l->oom < r->oom)
					m_add = l->oom;
				else
					m_add = r->oom;
			}

			return_value_convert_oom(l, OOM_BASE);
			return_value_convert_oom(r, OOM_BASE);

			switch (ret->type)
			{
			case RET_FLOAT:
				ret->f	= l->f + r->f;
				ret->oom= OOM_BASE;
				return_value_convert_oom(ret, m_add);
				break;
			case RET_COMPLEX:
				ret->c = return_value_as_complex(l) + return_value_as_complex(r);
				if (fabs(cimag(ret->c)) < EPS)
				{
					ret->type	= RET_FLOAT;
					ret->f		= creal(ret->c);
				}
				break;
			default:
			}
			break;
		case TK_SUB:
			order_of_magnetude_t	m_sub = OOM_NONE;

			if (ret->type != RET_COMPLEX)
			{
				if ((l->unit && r->unit && l->unit != r->unit) || l->unit >= U_ERR)
					ret->unit = U_ERR;
				else if (l->unit == r->unit)
					ret->unit = l->unit;
				else
					ret->unit = l->unit + r->unit;

				if (!l->unit && l->oom == OOM_NONE)
					l->oom = r->oom;
				if (!r->unit && r->oom == OOM_NONE)
					r->oom = l->oom;

				if (l->oom < r->oom)
					m_sub = l->oom;
				else
					m_sub = r->oom;
			}

			return_value_convert_oom(l, OOM_BASE);
			return_value_convert_oom(r, OOM_BASE);

			switch (ret->type)
			{
			case RET_FLOAT:
				ret->f	= l->f - r->f;
				ret->oom= OOM_BASE;
				return_value_convert_oom(ret, m_sub);
				break;
			case RET_COMPLEX:
				ret->c = return_value_as_complex(l) - return_value_as_complex(r);

				if (fabs(cimag(ret->c)) < EPS)
				{
					ret->type	= RET_FLOAT;
					ret->f		= creal(ret->c);
				}
				break;
			default:
			}
			break;
		case TK_DIV:
			if ((r->type == RET_FLOAT && fabs(r->f) < EPS)
				|| (r->type == RET_COMPLEX && fabs(creal(r->c)) < EPS && fabs(cimag(r->c)) < EPS))
			{
				ret->type	= RET_ERR;
				ret->err_code	= ERR_DIV_BY_ZERO;
				return ret;
			}

			order_of_magnetude_t	m_div = OOM_NONE;

			if (ret->type != RET_COMPLEX)
			{
				if (l->oom < r->oom)
					m_div = l->oom;
				else
					m_div = r->oom;
			}

			return_value_convert_oom(l, OOM_BASE);
			return_value_convert_oom(r, OOM_BASE);
			
			switch (l->unit)
			{
			case U_ERR:
			case U_UNSUPPORTED:
				ret->unit = l->unit;
				break;
			case U_NONE:
				ret->unit = r->unit;
				break;
			case U_JOULE:
				switch (r->unit)
				{
				case U_SECOND:
					ret->unit = U_WATT;
				case U_JOULE:
					break;
				case U_NONE:
					ret->unit = U_JOULE;
					break;
				case U_ERR:
					ret->unit = r->unit;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			case U_SECOND:
				switch (r->unit)
				{
				case U_SECOND:
					break;
				case U_NONE:
					ret->unit = U_SECOND;
					break;
				case U_ERR:
					ret->unit = r->unit;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			case U_AMPERE:
				switch (r->unit)
				{
				case U_AMPERE:
					break;
				case U_NONE:
					ret->unit = U_AMPERE;
					break;
				case U_ERR:
					ret->unit = r->unit;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			case U_OHM:
				switch (r->unit)
				{
				case U_OHM:
					break;
				case U_NONE:
					ret->unit = U_OHM;
					break;
				case U_ERR:
					ret->unit = r->unit;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			case U_VOLT:
				switch (r->unit)
				{
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
				case U_ERR:
					ret->unit = r->unit;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			case U_WATT:
				switch (r->unit)
				{
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
				case U_ERR:
					ret->unit = r->unit;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			default:
				ret->unit = U_UNSUPPORTED;
			} //switch (l->unit)

			switch (ret->type)
			{
			case RET_FLOAT:
				ret->f	= l->f / r->f;
				ret->oom= OOM_BASE;
				return_value_convert_oom(ret, m_div);
				break;
			case RET_COMPLEX:
				if (l->type == RET_COMPLEX && r->type == RET_COMPLEX)
					ret->c = l->c / r->c;
				else if (l->type == RET_COMPLEX)
					ret->c = l->c / r->f;
				else
					ret->c = l->f / r->c;
				break;
				if (fabs(cimag(ret->c)) < EPS)
				{
					ret->type	= RET_FLOAT;
					ret->f		= creal(ret->c);
				}
			default:
			}

			break;
		case TK_POW:
			order_of_magnetude_t	m_pow = OOM_NONE;

			if (ret->type != RET_COMPLEX)
			{
				if (l->oom < r->oom)
					m_pow = l->oom;
				else
					m_pow = r->oom;
			}

			return_value_convert_oom(l, OOM_BASE);
			return_value_convert_oom(r, OOM_BASE);

			if ((l->unit && r->unit && l->unit != r->unit) || l->unit >= U_ERR)
				ret->unit = U_ERR;
			else
				ret->unit = l->unit + r->unit;

			
			switch (ret->type)
			{
			case RET_FLOAT:
				ret->f	= pow(l->f, r->f);
				ret->oom= OOM_BASE;
				return_value_convert_oom(ret, m_pow);
				break;
			case RET_COMPLEX:
				ret->c = cpow(return_value_as_complex(l), return_value_as_complex(r));
				break;
			default:
			}
			break;
		case TK_MUL:
			order_of_magnetude_t	m_mul = OOM_NONE;

			if (ret->type != RET_COMPLEX)
			{
				if (l->oom < r->oom)
					m_mul = l->oom;
				else
					m_mul = r->oom;
			}

			return_value_convert_oom(l, OOM_BASE);
			return_value_convert_oom(r, OOM_BASE);

			switch (l->unit)
			{
			case U_ERR:
			case U_UNSUPPORTED:
				ret->unit = l->unit;
				break;
			case U_NONE:
				ret->unit = r->unit;
				break;
			case U_JOULE:
				switch (r->unit)
				{
				case U_NONE:
					ret->unit = U_JOULE;
					break;
				case U_ERR:
					ret->unit = r->unit;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			case U_SECOND:
				switch (r->unit)
				{
				case U_WATT:
					ret->unit = U_JOULE;
					break;
				case U_NONE:
					ret->unit = U_SECOND;
					break;
				case U_ERR:
					ret->unit = r->unit;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			case U_VOLT:
				switch (r->unit)
				{
				case U_AMPERE:
					ret->unit = U_WATT;
					break;
				case U_NONE:
					ret->unit = U_VOLT;
					break;
				case U_ERR:
					ret->unit = r->unit;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			case U_AMPERE:
				switch (r->unit)
				{
				case U_VOLT:
					ret->unit = U_WATT;
					break;
				case U_OHM:
					ret->unit = U_VOLT;
					break;
				case U_NONE:
					ret->unit = U_AMPERE;
					break;
				case U_ERR:
					ret->unit = r->unit;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			case U_OHM:
				switch (r->unit)
				{
				case U_AMPERE:
					ret->unit = U_VOLT;
					break;
				case U_NONE:
					ret->unit = U_AMPERE;
					break;
				case U_ERR:
					ret->unit = r->unit;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			case U_WATT:
				switch (r->unit)
				{
				case U_SECOND:
					ret->unit = U_JOULE;
					break;
				case U_NONE:
					ret->unit = U_WATT;
					break;
				case U_ERR:
					ret->unit = r->unit;
					break;
				default:
					ret->unit = U_UNSUPPORTED;
				}
				break;
			default:
				ret->unit = U_UNSUPPORTED;
			} // switch (l->unit)

			switch (ret->type)
			{
			case RET_FLOAT:
				ret->f	= l->f * r->f;
				ret->oom= OOM_BASE;
				return_value_convert_oom(ret, m_mul);
				break;
			case RET_COMPLEX:
				if (l->type == RET_COMPLEX && r->type == RET_COMPLEX)
					ret->c = l->c * r->c;
				else if (l->type == RET_COMPLEX)
					ret->c = l->c * r->f;
				else
					ret->c = l->f * r->c;
				
				if (fabs(cimag(ret->c)) < EPS)
				{
					ret->type	= RET_FLOAT;
					ret->f		= creal(ret->c);
				}
				break;
			default:
			}
			break;
		default:
			ret->type	= RET_ERR;
			ret->err_code	= ERR_TOKEN_IS_NOT_BIN_OP;
		} // switch (node->token.symbol)
		break;
	case EXPR_ERR:
		ret->type	= RET_ERR;
		ret->err_code	= node->err_code;
	} // switch (node->type)

	return ret;
}

void
evaluator_print_res(return_value_t* res)
{
	if (res->next)
	{
		printf("(");
		
		evaluator_print_res_val(res);
		res = res->next;

		while (res)
		{
			printf(", ");
			evaluator_print_res_val(res);
			res = res->next;
		}

		printf(")");
	}
	else
		evaluator_print_res_val(res);
	
	printf("\n");
}

#ifdef TESTER

static void
test_evaluator_atof(void)
{
	char*	test	= "256.34";
	token_t	tok	=
	{
		.start	= test,
		.length	= 6,
	};

	assert(evaluator_atof(&tok) >= 256.34 - EPS && evaluator_atof(&tok) <= 256.34 + EPS);
}

static void
test_evaluate_base(void)
{
	arena_t*	arena	= ARENA_ALLOC();
	variables_map*	vmap	= variables_map_new(arena, 100);
	lexer_t		lexer;
	ast_node_t*	tree	= 0;
	return_value_t*	res	= 0;

	char*	test = "-5 - 10.0 * 5";

	lexer_init(&lexer, test, 13);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_FLOAT);
	assert(res->f >= -55.0 - EPS && res->f <= -55.0 + EPS);
	
	char*	test2 = "-5 - 10.0 * (5 ^ 2)";
	
	lexer_init(&lexer, test2, 19);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_FLOAT);
	assert(res->f >= -255.0 - EPS && res->f <= -255.0 + EPS);
	
	arena_release(arena);
}

static void
test_evaluate_units_oom(void)
{
	arena_t*	arena	= ARENA_ALLOC();
	variables_map*	vmap	= variables_map_new(arena, 100);
	lexer_t		lexer;
	ast_node_t*	tree	= 0;
	return_value_t*	res	= 0;

	char*	test3 = "ampere(5)";
	
	lexer_init(&lexer, test3, 9);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_FLOAT);
	assert(res->f == 5);
	assert(res->unit == U_AMPERE);
	assert(res->oom == OOM_NONE);
	
	char*	test4 = "milli(volt(5)) + base_magnitude(volt(5))";
	
	lexer_init(&lexer, test4, 40);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_FLOAT);
	assert(res->oom == OOM_MILLI);
	assert(res->f >= 5005.0 - EPS && res->f <= 5005.0 + EPS);
	assert(res->unit == U_VOLT);

	char*	test5 = "milli(volt(5)) + volt(5)";
	
	lexer_init(&lexer, test5, 24);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_FLOAT);
	assert(res->f >= 5005.0 - EPS && res->f <= 5005.0 + EPS);
	assert(res->unit == U_VOLT);
	assert(res->oom == OOM_MILLI);

	char*	test6 = "milli(volt(5)) * milli(ampere(5))";
	
	lexer_init(&lexer, test6, 33);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_FLOAT);
	assert(res->f >= 0.025 - EPS && res->f <= 0.025 + EPS);
	assert(res->unit == U_WATT);
	assert(res->oom == OOM_MILLI);

	arena_release(arena);
}

static void
test_evaluate_binding(void)
{
	arena_t*	arena	= ARENA_ALLOC();
	variables_map*	vmap	= variables_map_new(arena, 100);
	lexer_t		lexer;
	ast_node_t*	tree	= 0;
	return_value_t*	res	= 0;

	char*	test7	= "x :: 10";
	token_t	x	= { test7, TK_ID, 1, 1 };

	lexer_init(&lexer, test7, 7);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);
	
	assert(res->type == RET_FLOAT);
	assert(res->f == 10);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);
	assert((*variables_map_get(vmap, &x))->f == 10);

	char*	test8 = "x + 10";

	lexer_init(&lexer, test8, 6);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);
	
	assert(res->type == RET_FLOAT);
	assert(res->f == 20);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);

	char*	test9 = "10 + x";

	lexer_init(&lexer, test9, 6);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_FLOAT);
	assert(res->f == 20);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);

	char*	test10	= "5 :: y";
	token_t	y	= { test10 + 5, TK_ID, 1, 1 };

	lexer_init(&lexer, test10, 6);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);
	
	assert(res->type == RET_FLOAT);
	assert(res->f == 5);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);
	assert((*variables_map_get(vmap, &y))->f == 5);

	char*	test11 = "x + y";

	lexer_init(&lexer, test11, 5);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_FLOAT);
	assert(res->f == 15);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);

	arena_release(arena);
}

static void
test_evaluate_list(void)
{
	arena_t*	arena	= ARENA_ALLOC();
	variables_map*	vmap	= variables_map_new(arena, 100);
	lexer_t		lexer;
	ast_node_t*	tree	= 0;
	return_value_t*	res	= 0;
	
	char*	test1	= "1, 2, 3";

	lexer_init(&lexer, test1, 7);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_FLOAT);
	assert(res->f == 1);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);

	assert(res->next);
	assert(res->next->type == RET_FLOAT);
	assert(res->next->f == 2);
	assert(res->next->unit == U_NONE);
	assert(res->next->oom == OOM_NONE);

	assert(res->next->next);
	assert(res->next->next->type == RET_FLOAT);
	assert(res->next->next->f == 3);
	assert(res->next->next->unit == U_NONE);
	assert(res->next->next->oom == OOM_NONE);

	char*	test2	= "x :: (1, 2, 3)";
	token_t	x	= { test2, TK_ID, 1, 1 };

	lexer_init(&lexer, test2, 14);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_FLOAT);
	assert(res->f == 1);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);

	assert(res->next);
	assert(res->next->type == RET_FLOAT);
	assert(res->next->f == 2);
	assert(res->next->unit == U_NONE);
	assert(res->next->oom == OOM_NONE);

	assert(res->next->next);
	assert(res->next->next->type == RET_FLOAT);
	assert(res->next->next->f == 3);
	assert(res->next->next->unit == U_NONE);
	assert(res->next->next->oom == OOM_NONE);
	
	assert(res->next->next->next == 0);
	
	return_value_t**	val = variables_map_get(vmap, &x);
	assert((*val)->f == 1);

	assert((*val)->type == RET_FLOAT);
	assert((*val)->f == 1);
	assert((*val)->unit == U_NONE);
	assert((*val)->oom == OOM_NONE);

	assert((*val)->next);
	assert((*val)->next != res->next);
	assert((*val)->next->type == RET_FLOAT);
	assert((*val)->next->f == 2);
	assert((*val)->next->unit == U_NONE);
	assert((*val)->next->oom == OOM_NONE);

	assert((*val)->next->next != res->next->next);
	assert((*val)->next->next);
	assert((*val)->next->next->type == RET_FLOAT);
	assert((*val)->next->next->f == 3);
	assert((*val)->next->next->unit == U_NONE);
	assert((*val)->next->next->oom == OOM_NONE);

	assert((*val)->next->next->next == 0);

	char*	test3	= "x(1)";

	lexer_init(&lexer, test3, 4);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_FLOAT);
	assert(res->f == 2);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);
	assert(res->next == 0);

	arena_release(arena);
}

static void
test_evaluate_complex(void)
{
	arena_t*	arena	= ARENA_ALLOC();
	variables_map*	vmap	= variables_map_new(arena, 100);
	lexer_t		lexer;
	ast_node_t*	tree	= 0;
	return_value_t*	res	= 0;
	
	char*	test1	= "5 + 5 * I";

	lexer_init(&lexer, test1, 9);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_COMPLEX);
	assert(fabs(creal(res->c) - 5) < EPS);
	assert(fabs(cimag(res->c) - 5) < EPS);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);
}

void
test_evaluate(void)
{
	test_evaluator_atof();
	test_evaluate_base();
	test_evaluate_units_oom();
	test_evaluate_binding();
	test_evaluate_list();
	test_evaluate_complex();
}

#endif // TESTER
