#include <vm/context.h>

context_t*
context_new(void)
{
	arena_t*	arena_glb	= ARENA_ALLOC();
	arena_t*	arena_line	= ARENA_ALLOC();
	context_t*	context		= ARENA_PUSH_STRUCT(arena_glb, context_t);

	context->arena_glb	= arena_glb;
	context->arena_line	= arena_line;
	context->strings	= string_set_new(arena_glb, 100);
	context->globals	= value_map_new(arena_glb, 100);
	context->constants	= value_array_new(arena_line, 100);

	return context;
}

inline void
context_reset_line(context_t* context)
{
	context->constants	= value_array_new(context->arena_line, 100);
}
