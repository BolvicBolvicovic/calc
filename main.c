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
#include <signal.h>

volatile s32	interrupted = 0;

static void
on_ctrl_c(s32 sig)
{
	(void)sig;
	interrupted = 1;
}

s32
main(void)
{
	ast_node_t*	tree		= 0;
	arena_t*	arena_tmp	= ARENA_ALLOC();
	arena_t*	arena_glb	= ARENA_ALLOC();
	arena_t*	arena_const	= ARENA_ALLOC();
	variables_map*	vmap_tmp	= variables_map_new(arena_tmp, 100);
	variables_map*	vmap_glb	= variables_map_new(arena_glb, 1000);
	variables_map*	vmap_const	= variables_map_new(arena_const, 100);
	string_map*	smap		= string_map_new(arena_const, 100);
	char*		line		= 0;
	return_value_t* res		= 0;
	u32		l_size		= 0;
	lexer_t		lexer;

	struct sigaction	sa = {0};

	sa.sa_handler	= on_ctrl_c;
	sa.sa_flags	= SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);

	evaluator_init_smap(smap);
	evaluator_init_const_map(vmap_const);

	while ((line = readline(PROMPT)))
	{
		if ((l_size = strlen(line)))
			add_history(line);
		else
			continue;
		
		interrupted = 0;

		lexer_init(&lexer, line, l_size);
		
		tree	= parser_parse_expression(arena_tmp, &lexer, 0);
		res	= evaluate(tree, &(evaluate_param_t){
					.arena_tmp	= arena_tmp,
					.arena_glb	= arena_glb,
					.vmap_tmp	= vmap_tmp,
					.vmap_glb	= vmap_glb,
					.vmap_const	= vmap_const,
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
