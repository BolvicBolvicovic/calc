#include <builtins.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <errors.h>

void
polynom_one(return_value_t* v)
{
	order_of_magnetude_t	m;

	if (!v->next || v->oom < v->next->oom)
		m = v->oom;
	else
		m = v->next->oom;

	return_value_convert_oom(v, OOM_BASE);

	if (v->next)
		return_value_convert_oom(v->next, OOM_BASE);

	f64	a = v->f;
	f64	b = v->next ? -v->next->f : 0;

	v->f	= b / a;
	v->next	= 0;
	v->type = RET_FLOAT;
	v->unit	= U_NONE;
	
	return_value_convert_oom(v, m);
}

void
polynom_two(arena_t* arena, return_value_t* v)
{
	return_value_t*		a	= v;
	return_value_t*		b	= v ? v->next : 0;
	return_value_t*		c	= b ? b->next : 0;

	return_value_convert_oom(a, OOM_BASE);

	if (b)
		return_value_convert_oom(b, OOM_BASE);

	if (c)
		return_value_convert_oom(c, OOM_BASE);

	f64	af = a ? a->f : 0;
	f64	bf = b ? b->f : 0;
	f64	cf = c ? c->f : 0;
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
}
	
void
polynom_three(arena_t* arena, return_value_t* val)
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
	
	f64	af = a ? a->f : 0;
	f64	bf = b ? b->f : 0;
	f64	cf = c ? c->f : 0;
	f64	df = d ? d->f : 0;

	if (af == 0)
	{
		val->type	= RET_ERR;
		val->err_code	= ERR_DIV_BY_ZERO;
		val->next	= 0;
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

	val->next	= ARENA_PUSH_ARRAY(arena, return_value_t, 2);
	val->next->next	= val->next + 1;

	val->unit	= U_NONE;
	val->oom	= OOM_BASE;

	val->next->oom	= OOM_BASE;
	val->next->unit	= U_NONE;
	val->next->token= 0;

	val->next->next->oom	= OOM_BASE;
	val->next->next->unit	= U_NONE;
	val->next->next->token	= 0;
	val->next->next->next	= 0;

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
}

// Credit for the algorithm: https://github.com/sasamil/Quartic/blob/master/quartic.cpp
void
polynom_four(arena_t* arena, return_value_t* val)
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
	
	f64	af = aval ? aval->f : 0;
	f64	bf = bval ? bval->f : 0;
	f64	cf = cval ? cval->f : 0;
	f64	df = dval ? dval->f : 0;
	f64	ef = eval ? eval->f : 0;

	if (af == 0)
	{
		val->type	= RET_ERR;
		val->err_code	= ERR_DIV_BY_ZERO;
		val->next	= 0;
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

	val->next		= ARENA_PUSH_ARRAY(arena, return_value_t, 3);
	val->next->next		= val->next + 1;
	val->next->next->next	= val->next + 2;

	val->oom	= OOM_BASE;
	val->unit	= U_NONE;

	val->next->oom	= OOM_BASE;
	val->next->unit	= U_NONE;
	val->next->token= 0;

	val->next->next->oom	= OOM_BASE;
	val->next->next->unit	= U_NONE;
	val->next->next->token	= 0;

	val->next->next->next->oom	= OOM_BASE;
	val->next->next->next->unit	= U_NONE;
	val->next->next->next->token	= 0;
	val->next->next->next->next	= 0;

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
}

#ifdef TESTER

#include <assert.h>

void
test_polynomials(void)
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
	res	= evaluate(arena, vmap, tree, arena, vmap);
	
	assert(res->type == RET_FLOAT);
	assert(res->f < -0.6 + EPS && res->f > -0.6 - EPS);
	assert(res->next == 0);

	lexer_init(&lexer, quadratic, 21);

	tree	= parser_parse_expression(arena, &lexer, 0);
	res	= evaluate(arena, vmap, tree, arena, vmap);
	
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
	res	= evaluate(arena, vmap, tree, arena, vmap);
	

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
	res	= evaluate(arena, vmap, tree, arena, vmap);
	

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

#endif
