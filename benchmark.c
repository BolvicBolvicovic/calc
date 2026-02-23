#include <swissmap.h>
#include <signal.h>
#include <interpreter/lexer.h>
#include <interpreter/evaluator.h>
#include <interpreter/builtins.h>

volatile s32	interrupted = 0;

static void
on_ctrl_c(s32 sig)
{
	(void)sig;
	interrupted = 1;
}

int
main()
{
	return_value_t*	res		= 0;
	ast_node_t*	tree		= 0;
	arena_t*	arena_tmp	= ARENA_ALLOC(.reserve_size=MB(128), .commit_size=MB(64));
	arena_t*	arena_glb	= ARENA_ALLOC();
	arena_t*	arena_const	= ARENA_ALLOC();
	variables_map*	vmap_tmp	= variables_map_new(arena_tmp, 100);
	variables_map*	vmap_glb	= variables_map_new(arena_glb, 1000);
	variables_map*	vmap_const	= variables_map_new(arena_const, 100);
	string_map*	smap		= string_map_new(arena_const, 100);
	lexer_t		lexer;
	evaluate_param_t param		=
	{
		.arena_tmp	= arena_tmp,
		.arena_glb	= arena_glb,
		.vmap_tmp	= vmap_tmp,
		.vmap_glb	= vmap_glb,
		.vmap_const	= vmap_const,
		.smap		= smap
	};
	evaluator_init_const_map(vmap_const);

	struct sigaction	sa = {0};

	sa.sa_handler	= on_ctrl_c;
	sa.sa_flags	= SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
//	u64		i = 0x1000000;
	u64		ix= 2;//0;
	char*	s[4] =
	{
		"my_current :: current(milli(volt(52.6 + 87)), micro(ampere(0.5554 / 27)))",
		"my_current(0) + 75 / 100",
		"y::func(z*z+C)",
		"plot(z, 2, 1000, 2, y, PL_BURNING_SHIP, PL_ZOOM_IN)"
	};

	s32	l[4] =
	{
		73,
		24,
		14,
		51,
	};
	//while (--i)
	while (ix < 4)
	{
		lexer_init(&lexer, s[ix], l[ix]);
		tree= parser_parse_expression(arena_tmp, &lexer, 0);
		res = evaluate(tree, &param);
		if (res)
			evaluator_print_res(arena_tmp, res);
		arena_clear(arena_tmp);
		vmap_tmp = variables_map_new(arena_tmp, 100);
	//	ix = !ix;
		ix++;
	}

	return 0;
}
