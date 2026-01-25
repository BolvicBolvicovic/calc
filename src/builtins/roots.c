#include <builtins.h>
#include <math.h>

inline void
roots_sqrt(return_value_t* val)
{
	if (val->type == RET_COMPLEX)
		val->c = csqrt(val->c);
	else
	{
		return_value_cast_to_float(val);
		val->f = sqrt(val->f);
	}
}

inline void
roots_cbrt(return_value_t* val)
{
	return_value_cast_to_float(val);
	val->f = cbrt(val->f);
}

