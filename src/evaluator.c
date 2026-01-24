#include <evaluator.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define _USE_MATH_DEFINES
#include <math.h>

#define EPS	1e-13

static s32	evaluator_memcmp(token_t* t1, token_t* t2);
static u64	evaluator_hash(token_t* t);
static void	return_value_cast_to_float(return_value_t* v);

SWISSMAP_DEFINE_FUNCTIONS(variables_map, token_t*, return_value_t*, evaluator_hash, evaluator_memcmp)

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

// START TRIGONOMETRY

static inline void
evaluator_cos(return_value_t* val)
{
	return_value_cast_to_float(val);
	val->f = cos(val->f);
}

static inline void
evaluator_sin(return_value_t* val)
{
	return_value_cast_to_float(val);
	val->f = sin(val->f);
}

static inline void
evaluator_tan(return_value_t* val)
{
	return_value_cast_to_float(val);
	val->f = tan(val->f);
}

static inline void
evaluator_arccos(return_value_t* val)
{
	return_value_cast_to_float(val);
	val->f = acos(val->f);
}

static inline void
evaluator_arcsin(return_value_t* val)
{
	return_value_cast_to_float(val);
	val->f = asin(val->f);
}

static inline void
evaluator_arctan(return_value_t* val)
{
	return_value_cast_to_float(val);
	val->f = atan(val->f);
}

// END TRIGONOMETRY

static inline void
evaluator_sqrt(return_value_t* val)
{
	return_value_cast_to_float(val);
	val->f = sqrt(val->f);
}

static inline void
evaluator_cbrt(return_value_t* val)
{
	return_value_cast_to_float(val);
	val->f = cbrt(val->f);
}

static inline void
evaluator_print_res_val(return_value_t* res)
{
	switch (res->type)
	{
	case RET_INT:
		printf("%ld", res->i);
		break;
	case RET_FLOAT:
		printf("%g", res->f);
		break;
	case RET_COMPLEX:
		printf("%g + %gi", creal(res->c), cimag(res->c));
		break;
	case RET_ERR:
		printf("Error while evaluating expression");
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
	case U_UNKNOWN:
		printf(" unknown function");
		break;
	case U_ERR:
		printf(" erronous unit");
		break;
	}
}

// START CAST & CONV RET_VAL

static inline f64
return_value_as_float(return_value_t* v)
{
	if (v->type == RET_FLOAT)
		return v->f;
	else if (v->type == RET_COMPLEX)
		return creal(v->c);

	return (f64)v->i;
}

static inline s64
return_value_as_int(return_value_t* v)
{
	if (v->type == RET_FLOAT)
		return (s64)v->f;
	else if (v->type == RET_COMPLEX)
		return (s64)creal(v->c);

	return v->i;
}

static inline void
return_value_cast_to_float(return_value_t* v)
{
	if (v->type == RET_INT)
		v->f = (f64)v->i;
	else if (v->type == RET_COMPLEX)
		v->f = creal(v->c);

	v->type = RET_FLOAT;
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

// END CAST & CONV RET_VAL

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
		res = res * 10 + (token->start[i] - '0');

	return res;
}

// START POLYNOMIALS

static void
evaluator_polynom_one(return_value_t* v)
{
	order_of_magnetude_t	m;

	if (!v->next || v->oom < v->next->oom)
		m = v->oom;
	else
		m = v->next->oom;

	return_value_convert_oom(v, OOM_BASE);

	if (v->next)
		return_value_convert_oom(v->next, OOM_BASE);

	f64	a = return_value_as_float(v);
	f64	b = v->next ? -return_value_as_float(v->next) : 0;

	v->f	= b / a;
	v->next	= 0;
	v->type = RET_FLOAT;
	v->unit	= U_NONE;
	
	return_value_convert_oom(v, m);
}

static void
evaluator_polynom_two(arena_t* arena, return_value_t* v)
{
	return_value_t*		a	= v;
	return_value_t*		b	= v ? v->next : 0;
	return_value_t*		c	= b ? b->next : 0;
	order_of_magnetude_t	ma	= a ? a->oom : OOM_BASE;
	order_of_magnetude_t	mb	= b ? b->oom : OOM_BASE;
	order_of_magnetude_t	mc	= c ? c->oom : OOM_BASE;
	order_of_magnetude_t	min	= ((ma ^ mb) & -(ma > mb)) ^ ma;

	min = ((min ^ mc) & -(min > mc)) ^ min;

	return_value_convert_oom(a, OOM_BASE);

	if (b)
		return_value_convert_oom(b, OOM_BASE);

	if (c)
		return_value_convert_oom(c, OOM_BASE);

	f64	af = a ? return_value_as_float(a) : 0;
	f64	bf = b ? return_value_as_float(b) : 0;
	f64	cf = c ? return_value_as_float(c) : 0;
	f64	x1 = (-bf + sqrt(pow(bf, 2) - 4 * af * cf)) / (2 * af);
	f64	x2 = (-bf - sqrt(pow(bf, 2) - 4 * af * cf)) / (2 * af);

	v->f		= x1;
	v->type		= RET_FLOAT;
	v->unit		= U_NONE;
	v->next		= ARENA_PUSH_STRUCT(arena, return_value_t);

	v->next->f	= x2;
	v->next->type	= RET_FLOAT;
	v->next->oom	= OOM_BASE;
	v->next->unit	= U_NONE;
	v->next->token	= 0;
	v->next->next	= 0;

	return_value_convert_oom(v, min);
	return_value_convert_oom(v->next, min);
}
	
static void
evaluator_polynom_three(arena_t* arena, return_value_t* val)
{
	return_value_t*		a	= val;
	return_value_t*		b	= a ? a->next : 0;
	return_value_t*		c	= b ? b->next : 0;
	return_value_t*		d	= c ? c->next : 0;

	return_value_convert_oom(a, OOM_BASE);

	if (b)
		return_value_convert_oom(b, OOM_BASE);

	if (c)
		return_value_convert_oom(c, OOM_BASE);
	
	if (d)
		return_value_convert_oom(d, OOM_BASE);
	
	f64	af = a ? return_value_as_float(a) : 0;
	f64	bf = b ? return_value_as_float(b) : 0;
	f64	cf = c ? return_value_as_float(c) : 0;
	f64	df = d ? return_value_as_float(d) : 0;

	if (af == 0)
	{
		val->type = RET_ERR;
		return;
	}

	f64	p 	= (3 * af * cf - pow(bf, 2)) / (3 * pow(af, 2));
	f64	q 	= (2 * pow(bf, 3) - 9 * af * bf * cf + 27 * pow(af, 2) * df) / (27 * pow(af, 3));

	double complex	delta	= pow(q / 2, 2) + pow(p / 3, 3);
	double complex	u3	= -q / 2 + csqrt(delta);
	double complex	r	= cabs(u3);
	double complex	theta	= carg(u3);
	double complex	u	= cbrt(r) * cexp(I * theta / 3);
	double complex	v	= -p / (3 * u);
	double complex	w	= cexp(2 * M_PI * I / 3);
	double complex	y1	= u + v;
	double complex	y2	= w * u + cpow(w, 2) * v;
	double complex	y3	= cpow(w, 2) * u + w * v;

	double complex	x1	= y1 - bf / (af * 3);
	double complex	x2	= y2 - bf / (af * 3);
	double complex	x3	= y3 - bf / (af * 3);

	if (fabs(cimag(x1)) < EPS)
	{
		val->f		= x1;
		val->type	= RET_FLOAT;
	}
	else
	{
		val->c		= x1;
		val->type	= RET_COMPLEX;
	}
	val->unit	= U_NONE;
	val->oom	= OOM_BASE;
	val->next	= ARENA_PUSH_STRUCT(arena, return_value_t);

	if (fabs(cimag(x2)) < EPS)
	{
		val->next->f	= x2;
		val->next->type	= RET_FLOAT;
	}
	else
	{
		val->next->c	= x2;
		val->next->type	= RET_COMPLEX;
	}
	val->next->oom	= OOM_BASE;
	val->next->unit	= U_NONE;
	val->next->token= 0;
	val->next->next	= ARENA_PUSH_STRUCT(arena, return_value_t);

	if (fabs(cimag(x3)) < EPS)
	{
		val->next->next->f	= x3;
		val->next->next->type	= RET_FLOAT;
	}
	else
	{
		val->next->next->c	= x3;
		val->next->next->type	= RET_COMPLEX;
	}
	val->next->next->oom	= OOM_BASE;
	val->next->next->unit	= U_NONE;
	val->next->next->token	= 0;
	val->next->next->next	= 0;
}

// Credit for the algorithm: https://github.com/sasamil/Quartic/blob/master/quartic.cpp
static void
evaluator_polynom_four(arena_t* arena, return_value_t* val)
{
	return_value_t*		aval	= val;
	return_value_t*		bval	= aval ? aval->next : 0;
	return_value_t*		cval	= bval ? bval->next : 0;
	return_value_t*		dval	= cval ? cval->next : 0;
	return_value_t*		eval	= dval ? dval->next : 0;

	return_value_convert_oom(aval, OOM_BASE);

	if (bval)
		return_value_convert_oom(bval, OOM_BASE);

	if (cval)
		return_value_convert_oom(cval, OOM_BASE);
	
	if (dval)
		return_value_convert_oom(dval, OOM_BASE);
	
	if (eval)
		return_value_convert_oom(eval, OOM_BASE);
	
	f64	af = aval ? return_value_as_float(aval) : 0;
	f64	bf = bval ? return_value_as_float(bval) : 0;
	f64	cf = cval ? return_value_as_float(cval) : 0;
	f64	df = dval ? return_value_as_float(dval) : 0;
	f64	ef = eval ? return_value_as_float(eval) : 0;

	if (af == 0)
	{
		val->type = RET_ERR;
		return;
	}

	// Solve quartic equation x^4 + a*x^3 + b*x^2 + c*x + d
	// with a, b, c and d defined bellow:
	f64	a = bf / af;
	f64	b = cf / af;
	f64	c = df / af;
	f64	d = ef / af;

	// Cubic resolvent
	// y^3 − b*y^2 + (ac−4d)*y − a^2*d−c^2+4*b*d = 0
	f64	a3 = -b;
	f64	b3 = a*c - 4.0*d;
	f64	c3 = -a*a*d - c*c + 4.0*b*d;

	f64	a2= a3*a3;
	f64	q = (a2 - 3*b3)/9;
	f64	r = (a3*(2*a2 - 9*b3) + 27*c3)/54;
	f64	r2= r*r;
	f64	q3= q*q*q;
	// Note: rr stands for real roots and is the number of real roots found.
	// z0, z1, z2 are the roots.
	f64	rr,z0,z1,z2;

	if (r2 < q3)
	{
		f64	t = r/sqrt(q3);
		
		if (t < -1) t = -1;
		if (t > 1) t = 1;

		t = acos(t);
		a3/= 3;
		q = -2*sqrt(q);
		z0= q*cos(t/3) - a3;
		z1= q*cos((t + M_PI*2)/3) - a3;
		z2= q*cos((t - M_PI*2)/3) - a3;

		rr = 3;
	}
	else
	{
		f64	A = -pow(fabs(r) + sqrt(r2 - q3), 1.0/3);

		if (r < 0) A = -A;

		f64	B = A == 0 ? 0 : q/A;
	
		a3/= 3;
		z0= (A + B) - a3;
		z1= -0.5*(A + B) - a3;
		z2= 0.5*sqrt(3.0)*(A - B);

		if (fabs(z2) < EPS)
		{
			z2 = z1;
			rr = 2;
		}
		else
			rr = 1;
	}

	// THE ESSENCE - choosing Y with maximal absolute value !
	f64	y = z0;

	if (rr != 1)
	{
		if (fabs(z1) > fabs(y)) y = z1;
		if (fabs(z2) > fabs(y)) y = z2;
	}

	// h1+h2 = y && h1*h2 = d  <=>  h^2 -y*h + d = 0    (h == q)
	f64	D = y*y - 4*d;
	f64	q1, q2, p1, p2, sqD;

	if (fabs(D) < EPS)
	{
		q1= q2 = y*0.5;
		D = q*q - 4*(b - y);

		// g1+g2 = a && g1+g2 = b-y   <=>   g^2 - a*g + b-y = 0    (p == g)
		if (fabs(D) < EPS)
			p1 = p2 = a*0.5;
		else
		{
			sqD= sqrt(D);
			p1 = (a + sqD)*0.5;
			p2 = (a - sqD)*0.5;
		}
	}
	else
	{
		sqD= sqrt(D);
		q1 = (y + sqD)*0.5;
		q2 = (y - sqD)*0.5;

		// g1+g2 = a && g1*h2 + g2*h1 = c       (h == q && g == p )  Krammer
		p1 = (a*q1 - c)/(q1 - q2);
		p2 = (c - a*q2)/(q1 - q2);
	}


	double complex	x1, x2, x3, x4;
	x1 = x2 = x3 = x4 = NAN;

	// solving quadratic eq. - x^2 + p1*x + q1 = 0
	D = p1*p1 - 4*q1;

	if (D < 0.0)
	{
		x1 = CMPLX(-p1*0.5, sqrt(-D)*0.5);
		x2 = x1*(-I);
	}
	else
	{
		sqD= sqrt(D);
		x1 = (-p1 + sqD)*0.5;
		x2 = (-p1 - sqD)*0.5;
	}
	
	// solving quadratic eq. - x^2 + p2*x + q2 = 0
	D = p2*p2 - 4*q2;
	
	if (D < 0.0)
	{
		x3 = CMPLX(-p2*0.5, sqrt(-D)*0.5);
		x4 = x3*(-I);
	}
	else
	{
		sqD= sqrt(D);
		x3 = (-p2 + sqD)*0.5;
		x4 = (-p2 - sqD)*0.5;
	}

	if (fabs(cimag(x1)) < EPS)
	{
		val->f		= x1;
		val->type	= RET_FLOAT;
	}
	else
	{
		val->c		= x1;
		val->type	= RET_COMPLEX;
	}
	val->oom	= OOM_BASE;
	val->unit	= U_NONE;
	val->next	= ARENA_PUSH_STRUCT(arena, return_value_t);

	if (fabs(cimag(x2)) < EPS)
	{
		val->next->f	= x2;
		val->next->type	= RET_FLOAT;
	}
	else
	{
		val->next->c	= x2;
		val->next->type	= RET_COMPLEX;
	}
	val->next->oom	= OOM_BASE;
	val->next->unit	= U_NONE;
	val->next->token= 0;
	val->next->next	= ARENA_PUSH_STRUCT(arena, return_value_t);

	if (fabs(cimag(x3)) < EPS)
	{
		val->next->next->f	= x3;
		val->next->next->type	= RET_FLOAT;
	}
	else
	{
		val->next->next->c	= x3;
		val->next->next->type	= RET_COMPLEX;
	}
	val->next->next->oom	= OOM_BASE;
	val->next->next->unit	= U_NONE;
	val->next->next->token	= 0;
	val->next->next->next	= ARENA_PUSH_STRUCT(arena, return_value_t);
	
	if (fabs(cimag(x4)) < EPS)
	{
		val->next->next->next->f	= x4;
		val->next->next->next->type	= RET_FLOAT;
	}
	else
	{
		val->next->next->next->c	= x4;
		val->next->next->next->type	= RET_COMPLEX;
	}
	val->next->next->next->oom	= OOM_BASE;
	val->next->next->next->unit	= U_NONE;
	val->next->next->next->token	= 0;
	val->next->next->next->next	= 0;
}

// END POLYNOMIALS

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
	ret->next	= 0;
	
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
				else if (memcmp("cos", node->token.start, 3) == 0)
					evaluator_cos(ret);
				else if (memcmp("sin", node->token.start, 3) == 0)
					evaluator_sin(ret);
				else if (memcmp("tan", node->token.start, 3) == 0)
					evaluator_tan(ret);
				else
					goto def1;
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
				else if (memcmp("sqrt", node->token.start, 4) == 0)
					evaluator_sqrt(ret);
				else if (memcmp("cbrt", node->token.start, 4) == 0)
					evaluator_cbrt(ret);
				else
					goto def1;
				break;
			case 5:
				if (memcmp("micro", node->token.start, 5) == 0)
					return_value_convert_to_micro(ret);
				else if (memcmp("milli", node->token.start, 5) == 0)
					return_value_convert_to_milli(ret);
				else if (memcmp("centi", node->token.start, 5) == 0)
					return_value_convert_to_centi(ret);
				else
					goto def1;
				break;
			case 6:
				if (memcmp("ampere", node->token.start, 6) == 0)
					return_value_convert_unit(ret, U_AMPERE);
				else if (memcmp("arccos", node->token.start, 6) == 0)
					evaluator_arccos(ret);
				else if (memcmp("arcsin", node->token.start, 6) == 0)
					evaluator_arcsin(ret);
				else if (memcmp("arctan", node->token.start, 6) == 0)
					evaluator_arctan(ret);
				else
					goto def1;
				break;
			case 11:
				if (memcmp("polynom_one", node->token.start, 11) == 0)
					evaluator_polynom_one(ret);
				else if (memcmp("polynom_two", node->token.start, 11) == 0)
					evaluator_polynom_two(arena, ret);
				else
					goto def1;
				break;
			case 12:
				if (memcmp("polynom_four", node->token.start, 12) == 0)
					evaluator_polynom_four(arena, ret);
				else
					goto def1;
				break;
			case 13:
				if (memcmp("polynom_three", node->token.start, 13) == 0)
					evaluator_polynom_three(arena, ret);
				else
					goto def1;
				break;
			case 14:
				if (memcmp("base_magnitude", node->token.start, 14) == 0)
					return_value_convert_to_base(ret);
				else
					goto def1;
				break;
			default:
			def1:
				return_value_t**	var = variables_map_get(vmap, &node->token);
				
				if (!var || !*var)
				{
					ret->unit = U_UNKNOWN;
					break;
				}

				s64		idx	= 0;
				s64		i	= 0;
				return_value_t*	tmp	= *var;

				switch (ret->token->symbol)
				{
				case TK_INT:
					idx = ret->i;
					break;
				case TK_FLOAT:
					idx = (s64)ret->f;
					break;
				default:
				}

				while (i < idx && tmp->next)
				{
					tmp = tmp->next;
					i++;
				}
				
				if (idx != i)
					ret->type = RET_ERR;
				else
					memcpy(ret, tmp, sizeof(return_value_t));

				ret->next = 0;
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
			case 5:
				if (memcmp("clear", node->token.start, 5) == 0)
				{
					printf("\033[2J\033[H");
					return 0;
				}
				else
					goto def;
				break;
			// Variables
			default:
			def:
				return_value_t**	var = variables_map_get(vmap, &node->token);
				
				if (var)
					ret = *var;
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
		
		if (node->left && ret->type != RET_ERR)
		{
			s64	idx	= 0;
			s64	i	= 0;

			switch (node->token.symbol)
			{
			case TK_INT:
				idx = ret->i;
				break;
			case TK_FLOAT:
				idx = (s64)ret->f;
				break;
			default:
			}

			ret = evaluate(arena, node->left, arena_vmap, vmap);

			for (; i < idx && ret->next; i++, ret = ret->next);
			
			if (idx != i)
				ret->type = RET_ERR;
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
		case TK_LIST:
			// TODO: maybe add a tail to return_value_t
			return_value_t*	tail = l;

			while (tail->next)
				tail = tail->next;

			tail->next = r;
			ret = l;
			break;
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

				return_value_t*	mapped_val = ARENA_PUSH_STRUCT(
								arena_vmap, return_value_t);
				evaluator_memcpy_value(arena_vmap, mapped_val, val);
				variables_map_put(vmap, tok, mapped_val);
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

	assert(res->type == RET_INT);
	assert(res->i == 5);
	assert(res->unit == U_AMPERE);
	assert(res->oom == OOM_NONE);
	
	char*	test4 = "milli(volt(5)) + base_magnitude(volt(5))";
	
	lexer_init(&lexer, test4, 40);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_FLOAT);
	assert(res->f >= 5005.0 - EPS && res->f <= 5005.0 + EPS);
	assert(res->unit == U_VOLT);
	assert(res->oom == OOM_MILLI);

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
	token_t	x	= { TK_ID, test7, 1, 1 };

	lexer_init(&lexer, test7, 7);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);
	
	assert(res->type == RET_INT);
	assert(res->i == 10);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);
	assert((*variables_map_get(vmap, &x))->i == 10);

	char*	test8 = "x + 10";

	lexer_init(&lexer, test8, 6);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);
	
	assert(res->type == RET_INT);
	assert(res->i == 20);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);

	char*	test9 = "10 + x";

	lexer_init(&lexer, test9, 6);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_INT);
	assert(res->i == 20);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);

	char*	test10	= "5 :: y";
	token_t	y	= { TK_ID, test10 + 5, 1, 1 };

	lexer_init(&lexer, test10, 6);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);
	
	assert(res->type == RET_INT);
	assert(res->i == 5);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);
	assert((*variables_map_get(vmap, &y))->i == 5);

	char*	test11 = "x + y";

	lexer_init(&lexer, test11, 5);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_INT);
	assert(res->i == 15);
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

	assert(res->type == RET_INT);
	assert(res->i == 1);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);

	assert(res->next);
	assert(res->next->type == RET_INT);
	assert(res->next->i == 2);
	assert(res->next->unit == U_NONE);
	assert(res->next->oom == OOM_NONE);

	assert(res->next->next);
	assert(res->next->next->type == RET_INT);
	assert(res->next->next->i == 3);
	assert(res->next->next->unit == U_NONE);
	assert(res->next->next->oom == OOM_NONE);

	char*	test2	= "x :: (1, 2, 3)";
	token_t	x	= { TK_ID, test2, 1, 1 };

	lexer_init(&lexer, test2, 14);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_INT);
	assert(res->i == 1);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);

	assert(res->next);
	assert(res->next->type == RET_INT);
	assert(res->next->i == 2);
	assert(res->next->unit == U_NONE);
	assert(res->next->oom == OOM_NONE);

	assert(res->next->next);
	assert(res->next->next->type == RET_INT);
	assert(res->next->next->i == 3);
	assert(res->next->next->unit == U_NONE);
	assert(res->next->next->oom == OOM_NONE);
	
	assert(res->next->next->next == 0);
	
	return_value_t**	val = variables_map_get(vmap, &x);
	assert((*val)->i == 1);

	assert((*val)->type == RET_INT);
	assert((*val)->i == 1);
	assert((*val)->unit == U_NONE);
	assert((*val)->oom == OOM_NONE);

	assert((*val)->next);
	assert((*val)->next != res->next);
	assert((*val)->next->type == RET_INT);
	assert((*val)->next->i == 2);
	assert((*val)->next->unit == U_NONE);
	assert((*val)->next->oom == OOM_NONE);

	assert((*val)->next->next != res->next->next);
	assert((*val)->next->next);
	assert((*val)->next->next->type == RET_INT);
	assert((*val)->next->next->i == 3);
	assert((*val)->next->next->unit == U_NONE);
	assert((*val)->next->next->oom == OOM_NONE);

	assert((*val)->next->next->next == 0);

	char*	test3	= "x(1)";

	lexer_init(&lexer, test3, 4);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);

	assert(res->type == RET_INT);
	assert(res->i == 2);
	assert(res->unit == U_NONE);
	assert(res->oom == OOM_NONE);
	assert(res->next == 0);

	arena_release(arena);
}

static void
test_evaluate_polynomials(void)
{
	char*	linear		= "polynom_one(5, 3)";
	char*	quadratic	= "polynom_two(5, 3, -1)";
	char*	cubic		= "polynom_three(1, -7, 7, 15)";
	char*	quartic		= "polynom_four(3, 6, -123, -126, 1080)";
	arena_t*	arena	= ARENA_ALLOC();
	variables_map*	vmap	= variables_map_new(arena, 100);
	lexer_t		lexer;
	ast_node_t*	tree	= 0;
	return_value_t*	res	= 0;

	lexer_init(&lexer, linear, 17);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);
	
	assert(res->type == RET_FLOAT);
	assert(res->f < -0.6 + EPS && res->f > -0.6 - EPS);
	assert(res->next == 0);

	lexer_init(&lexer, quadratic, 21);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);
	
	f64	x1 = (-3+sqrt(29))/10;
	f64	x2 = (-3-sqrt(29))/10;

	assert(res->type == RET_FLOAT);
	assert(res->f < x1 + EPS && res->f > x1 - EPS);
	assert(res->next != 0);
	
	assert(res->next->type == RET_FLOAT);
	assert(res->next->f < x2 + EPS && res->next->f > x2 - EPS);
	assert(res->next->next == 0);
	
	lexer_init(&lexer, cubic, 27);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);
	

	x1 = 5;
	x2 = -1;
	f64	x3 = 3;

	assert(res->type == RET_FLOAT);
	assert(res->f < x1 + EPS && res->f > x1 - EPS);
	assert(res->next != 0);
	
	assert(res->next->type == RET_FLOAT);
	assert(res->next->f < x2 + EPS && res->next->f > x2 - EPS);
	assert(res->next->next != 0);
	
	assert(res->next->next->type == RET_FLOAT);
	assert(res->next->next->f < x3 + EPS && res->next->next->f > x3 - EPS);
	assert(res->next->next->next == 0);

	// TODO: quartic test
	lexer_init(&lexer, quartic, 36);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, tree, arena, vmap);
	

	x1 = 3;
	x2 = -4;
	x3 = 5;
	f64	x4 = -6;

	assert(res->type == RET_FLOAT);
	assert(res->f < x1 + EPS && res->f > x1 - EPS);
	assert(res->next != 0);
	
	assert(res->next->type == RET_FLOAT);
	assert(res->next->f < x2 + EPS && res->next->f > x2 - EPS);
	assert(res->next->next != 0);
	
	assert(res->next->next->type == RET_FLOAT);
	assert(res->next->next->f < x3 + EPS && res->next->next->f > x3 - EPS);
	assert(res->next->next->next != 0);
	
	assert(res->next->next->next->type == RET_FLOAT);
	assert(res->next->next->next->f < x4 + EPS && res->next->next->next->f > x4 - EPS);
	assert(res->next->next->next->next == 0);
}

void
test_evaluate(void)
{
	test_evaluate_base();
	test_evaluate_units_oom();
	test_evaluate_binding();
	test_evaluate_list();
	test_evaluate_polynomials();
}

#endif // TESTER
