#include <builtins.h>
#include <math.h>

__always_inline void
trigo_cos(return_value_t* val)
{
	if (val->type == RET_COMPLEX)
		val->c = ccos(val->c);
	else
		val->f = cos(val->f);
}

__always_inline void
trigo_sin(return_value_t* val)
{
	if (val->type == RET_COMPLEX)
		val->c = csin(val->c);
	else
		val->f = sin(val->f);
}

__always_inline void
trigo_tan(return_value_t* val)
{
	if (val->type == RET_COMPLEX)
		val->c = ctan(val->c);
	else
		val->f = tan(val->f);
}

__always_inline void
trigo_arccos(return_value_t* val)
{
	if (val->type == RET_COMPLEX)
		val->c = acos(val->c);
	else
		val->f = acos(val->f);
}

__always_inline void
trigo_arcsin(return_value_t* val)
{
	if (val->type == RET_COMPLEX)
		val->c = asin(val->c);
	else
		val->f = asin(val->f);
}

__always_inline void
trigo_arctan(return_value_t* val)
{
	if (val->type == RET_COMPLEX)
		val->c = atan(val->c);
	else
		val->f = atan(val->f);
}
