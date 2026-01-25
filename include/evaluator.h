#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <parser.h>
#include <swissmap.h>
#include <arena.h>
#include <complex.h>

#define EPS	1e-13

typedef enum return_t	return_t;
enum return_t
{
	RET_INT,
	RET_FLOAT,
	RET_COMPLEX,
	RET_BINDABLE,
	RET_ERR,
};

typedef enum unit_t	unit_t;
enum unit_t
{
	// Default
	U_NONE,

	// Actual units
	U_AMPERE,
	U_VOLT,
	U_WATT,
	U_OHM,

	// Errors
	U_ERR,
	U_UNSUPPORTED,
	U_UNKNOWN,
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
		f64	f;
		s64	i;
		complex	c;
	};
	return_t		type;
	unit_t			unit;
	order_of_magnetude_t	oom;
	token_t*		token;
	return_value_t*		next;
};

SWISSMAP_DECLARE(variables_map, token_t*, return_value_t*)
SWISSMAP_DECLARE_FUNCTIONS(variables_map, token_t*, return_value_t*)

f64		return_value_as_float(return_value_t* v);
void		return_value_convert_oom(return_value_t* v, order_of_magnetude_t oom);
void		return_value_cast_to_float(return_value_t* v);
return_value_t*	evaluate(arena_t*, ast_node_t* expr, arena_t* arena_vmap, variables_map*);
void		evaluator_print_res(return_value_t* res);

#ifdef TESTER

void	test_evaluator_atoi(void);
void	test_evaluator_atof(void);
void	test_evaluate(void);

#endif // TESTER

#endif // EVALUATOR_H
