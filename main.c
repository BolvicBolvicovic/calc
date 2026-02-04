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
	arena_t*	arena_const	= ARENA_ALLOC();
	variables_map*	vmap_tmp	= variables_map_new(arena_tmp, 100);
	variables_map*	vmap_glb	= variables_map_new(arena_glb, 1000);
	variables_map*	vmap_const	= variables_map_new(arena_const, 100);
	string_map*	smap		= string_map_new(arena_const, 100);
	char*		line		= 0;
	return_value_t* res		= 0;
	u32		l_size		= 0;
	lexer_t		lexer;

	string_map_put(smap, &(token_t){.start = "operations", .length = 10},
		"> Operations Helper\n"
		"> Math operations available: +, -, /, *, ^\n"
		"> Example:\n"
		"calc> -5 + 5 * 8 / (5 ^ 6)\n"
		"> -5\n"
		"> More information can be found in ./mkdocs/docs/documentation.md\n");
	string_map_put(smap, &(token_t){.start = "lists", .length = 5},
		"> Lists Helper\n"
		"> A list is defined with the list operator ','.\n"
		"> An element of a list bound to a variable can be accessed with a 0-based index.\n"
		"> Example:\n"
		"calc> my_list :: (12, 5, 6)\n"
		"> (12, 5, 6)\n"
		"calc> my_list(0)\n"
		"> 12\n"
		"calc> my_list(10)\n"
		"> Error at line 1 here ->10): Out of bound\n"
		"> More information can be found in ./mkdocs/docs/documentation.md\n");
	string_map_put(smap, &(token_t){.start = "variables", .length = 9},
		"> Variables Helper\n"
		"> A value can be bound to a variable either globally with the operator '::'\n"
		"> or locally with the operator ':'.\n"
		"calc> x :: milli(watt(5))\n"
		"> 5 milli watt\n"
		"calc> (c : 5) + c\n"
		"> 10\n"
		"calc> c * c\n"
		"> Error at line 1 here ->* c: Operation with unbound variable not allowed\n"
		"> More information can be found in ./mkdocs/docs/documentation.md\n");
	string_map_put(smap, &(token_t){.start = "functions", .length = 9},
		"> Functions Helper\n"
		"> A function is declared with the built-in `func`.\n"
		"> The expression in its parenthesis is saved and can be used later.\n"
		"calc> circle :: func(2 * PI * _R)\n"
		"> ((2)*(PI))*(_R)\n"
		"calc> circle(_R:5)\n"
		"> 31.4159\n"
		"> More information can be found in ./mkdocs/docs/documentation.md\n");
	string_map_put(smap, &(token_t){.start = "builtins", .length = 8},
		"> Built-ins Helper\n"
		"> Math built-ins:\n"
		">	- Polyomials: polynom_one, polynom_two, polynom_three, polynom_four\n"
		">	- Trigonometry: cos, arccos, tan, arctan, sin, arcsin\n"
		">	- Roots: sqrt, cbrt\n"
		">	- Constants: PI, E, I\n"
		"> Physics built-ins:\n"
		">	- Electricity:\n"
		">		- current(a,b): 'a' != 'b' && 'ohm', 'ampere', 'watt' or 'volt'\n"
		">		- res_parallel(r1, ..., rn): 'rn' always considered as 'ohm'\n"
		">		- volt_divider(v, r1, ..., rn), 'v' as 'volt' and 'rn' as 'ohm'\n"
		">		- amp_divider(a, r1, ..., rn), 'a' as 'ampere' and 'rn' as 'ohm'\n"
		"> Other built-ins:\n"
		">	- exit: quits the calculator\n"
		">	- clear: clears the terminal\n"
		">	- new_session: clears the global variables hashmap\n"
		">	- unbind(var): clears 'var' from the global variables hashmap\n"
		"> More information can be found in ./mkdocs/docs/documentation.md\n");
	string_map_put(smap, &(token_t){.start = "orders_of_magnitude", .length = 19},
		"> Orders of Magnetude Helper\n"
		"> Orders of magnitude (OOMs) are subtypes that can affect a variable's value.\n"
		"> They are also metadata about a variable.\n"
		"> Available built-in magnitudes:\n"
		">	- base_magnitude\n"
		">	- deci\n"
		">	- centi\n"
		">	- milli\n"
		">	- micro\n"
		">	- nano\n"
		"calc> milli(ampere(5)) * milli(volt(5))\n"
		"> 0.025 milli watt\n"
		"> More information can be found in ./mkdocs/docs/documentation.md\n");
	string_map_put(smap, &(token_t){.start = "physics_units", .length = 13},
		"> Physics Unit Helper\n"
		"> Physics units are subtypes that cannot affect a variable's value.\n"
		"> They are metadata about a variable.\n"
		"> However, they can interact with each other, reducing their expression to the most consise form.\n"
		"> Example: ampere * volt = watt\n"
		"> Available built-in units:\n"
		">	- volt\n"
		">	- ampere\n"
		">	- ohm\n"
		">	- watt\n"
		">	- seconds\n"
		">	- joule\n"
		"calc> ampere(5) * volt(5)\n"
		"> 25 watt\n"
		"> More information can be found in ./mkdocs/docs/documentation.md\n");

	evaluator_init_const_map(vmap_const);

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
