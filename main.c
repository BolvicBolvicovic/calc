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
	arena_t*	arena_tmp	= ARENA_ALLOC();
	arena_t*	arena_glb	= ARENA_ALLOC();
	arena_t*	arena_smap	= ARENA_ALLOC();
	variables_map*	vmap_tmp	= variables_map_new(arena_tmp, 100);
	variables_map*	vmap_glb	= variables_map_new(arena_glb, 1000);
	string_map*	smap		= string_map_new(arena_smap, 100);
	char*		line		= 0;
	return_value_t* res		= 0;
	u32		l_size		= 0;
	lexer_t		lexer;

	string_map_put(smap, &(token_t){.start = "operations", .length = 10},
			"> Operations Helper\n");
	string_map_put(smap, &(token_t){.start = "lists", .length = 5},
			"> Lists Helper\n");
	string_map_put(smap, &(token_t){.start = "variables", .length = 9},
			"> Variables Helper\n");
	string_map_put(smap, &(token_t){.start = "functions", .length = 9},
			"> Functions Helper\n");
	string_map_put(smap, &(token_t){.start = "builtins", .length = 8},
			"> Built-ins Helper\n");
	string_map_put(smap, &(token_t){.start = "orders_of_magnitude", .length = 19},
			"> Orders of Magnetude Helper\n");
	string_map_put(smap, &(token_t){.start = "physics_units", .length = 13},
			"> Physics Unit Helper\n");

	while ((line = readline(PROMPT)))
	{
		if ((l_size = strlen(line)))
			add_history(line);
		else
			continue;

		lexer_init(&lexer, line, l_size);
		
		tree	= parser_parse_expression(arena_tmp, &lexer, 0);
		res	= evaluate(tree, &(evaluate_param_t){
					.arena_tmp	= arena_tmp,
					.arena_glb	= arena_glb,
					.vmap_tmp	= vmap_tmp,
					.vmap_glb	= vmap_glb,
					.smap		= smap});
		
		if (res)
		{
			printf("> \e[3m");
			evaluator_print_res(arena_tmp, res);
			printf("\e[0m");
		}

		free(line);
		arena_clear(arena_tmp);
		vmap_tmp = variables_map_new(arena_tmp, 100);
	}

	return 0;
}
