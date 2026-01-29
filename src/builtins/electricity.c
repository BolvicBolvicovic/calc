#include <builtins.h>
#include <math.h>
#include <string.h>
#include <errors.h>

static inline f64
current_formula(unit_t au, unit_t bu, unit_t cu, f64 a, f64 b)
{
	// Note: no need to handle default since checks have been performed in the caller.
	switch (cu)
	{
	case U_AMPERE:
		switch (au)
		{
		case U_OHM:
			return bu == U_WATT ? sqrt(b / a) : b / a;
		case U_WATT:
			return bu == U_OHM ? sqrt(a / b) : a / b;
		case U_VOLT:
			return bu == U_OHM ? a / b : b / a;
		default:
		}
		break;
	case U_OHM:
		switch (au)
		{
		case U_AMPERE:
			return bu == U_WATT ? b / (a * a) : b / a;
		case U_WATT:
			return bu == U_AMPERE ? a / (b * b) : (b * b) / a;
		case U_VOLT:
			return bu == U_AMPERE ? a / b : (a * a) / b;
		default:
		}
		break;
	case U_VOLT:
		switch (au)
		{
		case U_OHM:
			return bu == U_WATT ? sqrt(a * b) : a * b;
		case U_WATT:
			return bu == U_AMPERE ? a / b : sqrt(a * b);
		case U_AMPERE:
			return bu == U_WATT ? b / a : a * b;
		default:
		}
		break;
	case U_WATT:
		switch (au)
		{
		case U_OHM:
			return bu == U_AMPERE ? b * b * a : (b * b) / a;
		case U_AMPERE:
			return bu == U_OHM ? a * a * b : a * b;
		case U_VOLT:
			return bu == U_OHM ? (a * a) / b : a * b;
		default:
		}
		break;
	default:
	}

	// Note: This should never be reached.
	return -1;
}

void
current(return_value_t* val)
{
	if (!val)
		return;

	return_value_t* av = val;
	return_value_t* bv = av->next;

	if (!bv)
	{
		val->type	= RET_ERR;
		val->f		= ERR_WRONG_ARG;
		val->next	= 0;
		return;
	}

	unit_t	au = av->unit;
	unit_t	bu = bv->unit;

	if (au == bu || au < U_AMPERE || au > U_OHM || bu < U_AMPERE || bu > U_OHM)
	{
		val->type	= RET_ERR;
		val->f		= ERR_WRONG_ARG;
		return;
	}
	
	order_of_magnetude_t	mag = av->oom < bv->oom ? av->oom : bv->oom;

	return_value_convert_oom(av, OOM_BASE);
	return_value_convert_oom(bv, OOM_BASE);
	
	// Note: This only works because units are bitmasks.
	unit_t	mu= U_CURRENT ^ au ^ bu;
	unit_t	cu= 1 << __builtin_ctz(mu);
	unit_t	du= 1 << __builtin_ctz(mu &= mu - 1);
	f64	a = av->f;
	f64	b = bv->f;
	f64	c = current_formula(au, bu, cu, a, b);
	f64	d = current_formula(au, bu, du, a, b);

	av->f 	= c;
	av->unit= cu;
	bv->f 	= d;
	bv->unit= du;

	return_value_convert_oom(av, mag);
	return_value_convert_oom(bv, mag);
}

void
res_parallel(return_value_t* val)
{
	if (!val)
		return;

	return_value_convert_oom(val, OOM_BASE);

	return_value_t* 	tmp = val->next;
	order_of_magnetude_t	mag = val->oom;	
	f64			acc = 1. / val->f;

	while (tmp)
	{
		if (mag > tmp->oom)
			mag = tmp->oom;

		return_value_convert_oom(tmp, OOM_BASE);
		acc += 1. / tmp->f;
		tmp = tmp->next;
	}
	
	val->next	= 0;
	val->f		= 1 / acc;
	val->unit	= U_OHM;

	return_value_convert_oom(val, mag);
}

void
current_divider(return_value_t* val, unit_t unit)
{
	if (!val || !val->next)
		return;

	return_value_convert_oom(val, OOM_BASE);

	return_value_t* 	res 		= val->next;
	order_of_magnetude_t	mag 		= val->oom;	
	f64			unit_val	= val->f;
	f64			res_total	= 0;

	while (res)
	{
		if (mag > res->oom)
			mag = res->oom;

		return_value_convert_oom(res, OOM_BASE);
		res_total += res->f;
		res = res->next;
	}

	res = val->next;

	while (res)
	{
		res->f		= unit_val * res->f / res_total;
		res->unit	= unit;
		return_value_convert_oom(res, mag);
		res 		= res->next;
	}
	
	memcpy(val, val->next, sizeof(return_value_t));
}
