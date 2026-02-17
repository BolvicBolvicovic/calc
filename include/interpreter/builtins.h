#ifndef BUILTINS_H
#define BUILTINS_H

#include <interpreter/evaluator.h>

// MATH

// Polynomials

void	polynom_one(return_value_t*);
void	polynom_two(arena_t*, return_value_t*);
void	polynom_three(arena_t*, return_value_t*);
void	polynom_four(arena_t*, return_value_t*);

// Trigo

void	trigo_cos(return_value_t*);
void	trigo_sin(return_value_t*);
void	trigo_tan(return_value_t*);
void	trigo_arccos(return_value_t*);
void	trigo_arcsin(return_value_t*);
void	trigo_arctan(return_value_t*);

// Roots

void	roots_sqrt(return_value_t*);
void	roots_cbrt(return_value_t*);

// PHYSICS

// Electricity

void	current(return_value_t*);
void	res_parallel(return_value_t*);
void	current_divider(return_value_t*, unit_t); 

#ifdef TESTER

void	test_polynomials(void);

#endif // TESTER

#endif // BUILTINS_H
