#include <c_types.h>
#include <assert.h>
#include <signal.h>
#include <arena.h>
#include <stdio.h>
#include <stdlib.h>
#include <macros.h>
#include <readline/readline.h>
#include <readline/history.h>


#ifdef INTERPRETER_BUILD
// TODO: move it upward when ready
volatile s32	interrupted = 0;

static void
on_ctrl_c(s32 sig)
{
	(void)sig;
	interrupted = 1;
}

#include <interpreter/lexer.h>
#include <interpreter/parser.h>
#include <interpreter/evaluator.h>

s32
main(void)
{
	ast_node_t*	tree		= 0;
	arena_t*	arena_tmp	= ARENA_ALLOC(.reserve_size=MB(128), .commit_size=MB(64));
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

#else // INTERPRETER_BUILD

#include <read_file.h>
#include <vm/chunk.h>
#include <vm/value.h>
#include <vm/vm.h>
#include <vm/compiler.h>

s32
main(s32 argc, char** argv)
{
	arena_t*	arena_tmp	= ARENA_ALLOC();
	arena_t*	arena_const	= ARENA_ALLOC();
	vm_t*		vm		= vm_new(arena_const);
	compiler_t	compiler	=
	{
		.arena_const=arena_const,
		.arena_tmp=arena_tmp,
		.strings=vm->strings,
		.constants=value_array_new(arena_const, 1000),
	};

	//struct sigaction	sa = {0};

	//sa.sa_handler	= on_ctrl_c;
	//sa.sa_flags	= SA_RESTART;
	//sigemptyset(&sa.sa_mask);
	//sigaction(SIGINT, &sa, NULL);

	switch (argc)
	{
	case 1:
		u32	l_size	= 0;

		while ((compiler.src = readline(PROMPT)))
		{
			arena_temp_t	tmp = arena_temp_begin(arena_tmp);

			if ((l_size = strlen(compiler.src)))
				add_history(compiler.src);
			else
				continue;
			
			//interrupted = 0;

			chunk_t*	bytes	= compiler_run(&compiler);
			vm_result_t	result	= vm_run(vm, bytes);

			if (result == VM_RES_COMPILE_ERR) exit(65);
			if (result == VM_RES_RUNTIME_ERR)
			{
				chunk_dissassemble(bytes);
				exit(70);
			}

			arena_temp_end(tmp);
			free(compiler.src);
		}

		return 0;
	case 2:
		compiler.src = read_file(arena_tmp, argv[1]);

		chunk_t*	bytes	= compiler_run(&compiler);
		vm_result_t	result	= vm_run(vm, bytes);

		if (result == VM_RES_COMPILE_ERR) exit(65);
		if (result == VM_RES_RUNTIME_ERR) exit(70);

		return 0;
	default:
		fprintf(stderr, "Usage: calc-vm [path]\n");
		return 64;
	}

	return 0;
}


#endif // VM_BUILD 
