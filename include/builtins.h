#ifndef BUILTINS_H
#define BUILTINS_H

#include <evaluator.h>

// Polynomials

void	polynom_one(return_value_t* v);
void	polynom_two(arena_t*, return_value_t* v);
void	polynom_three(arena_t*, return_value_t* v);
void	polynom_four(arena_t*, return_value_t* v);

// Trigo

void	trigo_cos(return_value_t* val);
void	trigo_sin(return_value_t* val);
void	trigo_tan(return_value_t* val);
void	trigo_arccos(return_value_t* val);
void	trigo_arcsin(return_value_t* val);
void	trigo_arctan(return_value_t* val);

// Roots

void	roots_sqrt(return_value_t* val);
void	roots_cbrt(return_value_t* val);

#ifdef TESTER

void	test_polynomials(void);

#endif // TESTER

#endif // BUILTINS_H
