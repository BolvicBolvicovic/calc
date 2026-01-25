#include <builtins.h>
#include <math.h>

inline void
trigo_cos(return_value_t* val)
{
	return_value_cast_to_float(val);
	val->f = cos(val->f);
}

inline void
trigo_sin(return_value_t* val)
{
	return_value_cast_to_float(val);
	val->f = sin(val->f);
}

inline void
trigo_tan(return_value_t* val)
{
	return_value_cast_to_float(val);
	val->f = tan(val->f);
}

inline void
trigo_arccos(return_value_t* val)
{
	return_value_cast_to_float(val);
	val->f = acos(val->f);
}

inline void
trigo_arcsin(return_value_t* val)
{
	return_value_cast_to_float(val);
	val->f = asin(val->f);
}

inline void
trigo_arctan(return_value_t* val)
{
	return_value_cast_to_float(val);
	val->f = atan(val->f);
}
