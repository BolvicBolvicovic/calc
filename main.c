#include <c_types.h>
#include <lexer.h>
#include <parser.h>
#include <evaluator.h>
#include <arena.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <macros.h>

s32
main()
{
	ast_node_t*	tree		= 0;
	arena_t*	arena_loop	= ARENA_ALLOC();
	arena_t*	arena_vmap	= ARENA_ALLOC();
	variables_map*	vmap		= variables_map_new(arena_vmap, 1000);
	variables_map*	vmap_tmp	= variables_map_new(arena_loop, 100);
	char*		line		= 0;
	return_value_t* res		= 0;
	u32		l_size		= 0;
	lexer_t		lexer;

	while ((line = readline(PROMPT)))
	{
		if ((l_size = strlen(line)))
			add_history(line);
		else
			continue;

		lexer_init(&lexer, line, l_size);
		
		tree	= parser_parse_expression(arena_loop, &lexer, 0);
		res	= evaluate(arena_loop, vmap_tmp, tree, arena_vmap, vmap);
		
		if (res)
		{
			printf("> \e[3m");
			evaluator_print_res(arena_loop, res);
			printf("\e[0m");
		}

		free(line);
		arena_clear(arena_loop);
		vmap_tmp = variables_map_new(arena_loop, 100);
	}

	return 0;
}
