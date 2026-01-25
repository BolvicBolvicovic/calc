#include <builtins.h>
#include <math.h>

inline void
roots_sqrt(return_value_t* val)
{
	if (val->type == RET_COMPLEX)
		val->c = csqrt(val->c);
	else
		val->f = sqrt(val->f);
}

inline void
roots_cbrt(return_value_t* val)
{
	if (val->type == RET_COMPLEX)
		val->c = cpow(val->c, 1./3);
	else
		val->f = cbrt(val->f);
}

