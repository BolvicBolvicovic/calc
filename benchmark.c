#include <lexer.h>
#include <evaluator.h>
#include <swissmap.h>
#include <builtins.h>

static volatile void *sink;
	
int
main()
{
	volatile return_value_t*	res	= 0;

	arena_t*	arena_loop	= ARENA_ALLOC();
	arena_t*	arena_vmap	= ARENA_ALLOC();
	variables_map*	vmap		= variables_map_new(arena_vmap, 100);
	ast_node_t*	tree		= 0;
	lexer_t		lexer;

	u64		i = 0x1000000;
	u64		ix= 0;
	const char*	s[2] =
	{
		"my_current :: current(milli(volt(52.6 + 87)), micro(ampere(0.5554 / 27)))",
		"my_current(0) + 75 / 100"
	};

	while (--i)
	{
		lexer_init(&lexer, s[ix], sizeof(s[ix]));
		tree= parser_parse_expression(arena_loop, &lexer, 0);
		res = evaluate(arena_loop, tree, arena_vmap, vmap);
		sink= res;
		arena_clear(arena_loop);
		ix = !ix;
	}

	return 0;
}
