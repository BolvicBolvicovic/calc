#ifndef BOOL_H
#define BOOL_H

#include <c_types.h>

static __always_inline bool
is_space(char c)
{
	return c == ' ' || (u8)(c - '\b') <= ('\r' - '\b');
}

static __always_inline bool
is_digit(char c)
{
	return (u8)(c - '0') <= ('9' - '0');
}

static __always_inline bool
is_alpha(char c)
{
	return (u8)(c - 'a') <= ('z' - 'a') || (u8)(c - 'A') <= ('Z' - 'A') || c == '_';
}


#endif
