#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <swissmap.h>
#include <arena.h>
#include <complex.h>
#include <bits.h>
#include <interpreter/parser.h>
#include <interpreter/errors.h>

#define EPS	1e-13

typedef enum return_t	return_t;
enum return_t
{
	RET_FLOAT,
	RET_COMPLEX,
	RET_FUNC,
	RET_BINDABLE,
	RET_ERR,
};

typedef enum unit_t	unit_t;
enum unit_t
{
	// Default
	U_NONE,

	// Actual units
#define U_CURRENT 0b1111
	U_AMPERE= BIT0,
	U_VOLT	= BIT1,
	U_WATT	= BIT2,
	U_OHM	= BIT3,
	U_JOULE,
	U_SECOND,

	// Errors
	U_ERR,
	U_UNSUPPORTED,
};

typedef enum order_of_magnetude_t	order_of_magnetude_t;
enum order_of_magnetude_t
{
	// OOMs
	OOM_NANO,
	OOM_MICRO,
	OOM_MILLI,
	OOM_CENTI,
	OOM_DECI,
	OOM_BASE,

	// Default
	OOM_NONE,
};

typedef struct return_value_t	return_value_t;
struct return_value_t
{
	union
	{
		f64		f;
		complex		c;
		ast_node_t*	func;
		error_code_t	err_code;
	};
	return_t		type;
	unit_t			unit;
	order_of_magnetude_t	oom;
	token_t*		token;
	return_value_t*		next;
};

SWISSMAP_DECLARE(variables_map, token_t*, return_value_t*)
SWISSMAP_DECLARE_FUNCTIONS(variables_map, token_t*, return_value_t*)

SWISSMAP_DECLARE(string_map, token_t*, char*)
SWISSMAP_DECLARE_FUNCTIONS(string_map, token_t*, char*)

typedef struct evaluate_param_t	evaluate_param_t;
struct evaluate_param_t
{
	arena_t*	arena_tmp;
	arena_t*	arena_glb;
	variables_map*	vmap_tmp;
	variables_map*	vmap_glb;
	variables_map*	vmap_const;
	string_map*	smap;
};

f64		return_value_as_float(return_value_t* v);
void		return_value_convert_oom(return_value_t* v, order_of_magnetude_t oom);
return_value_t*	evaluate(ast_node_t* expr, evaluate_param_t*);
void		evaluator_print_res(arena_t*, return_value_t* res);
void		evaluator_init_const_map(variables_map* vmap);
void		evaluator_init_smap(string_map* smap);

#ifdef TESTER

void	test_evaluate(void);

#endif // TESTER

#endif // EVALUATOR_H
