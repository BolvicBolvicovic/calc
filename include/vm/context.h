#ifndef CONTEXT_H
#define CONTEXT_H

#include <arena.h>
#include <vm/value.h>

typedef struct context_t context_t;
struct context_t
{
	// Global lifetime
	arena_t*	arena_glb;
	string_set*	strings;	// plain strings
	value_map*	globals;	// global variables

	// Line lifetime
	arena_t*	arena_line;
	value_array_t*	constants;	// plain numbers

	// Expression lifetime
	//arena_t*	arena_expr;
};

context_t*	context_new(void);
void		context_reset_line(context_t* context);

#endif
