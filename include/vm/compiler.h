#ifndef COMPILER_H
#define COMPILER_H

#include <vm/chunk.h>
#include <vm/scanner.h>

typedef struct parser_t parser_t;
struct parser_t
{
	token_t		prev;
	token_t		current;
	scanner_t	scanner;
	bool		error;
	bool		panic;
	chunk_t*	chunk;
	arena_t*	arena_const;
	arena_t*	arena_tmp;
};

typedef void(*parser_fn_t)(parser_t*);

typedef enum precedence_t precedence_t;
enum precedence_t
{
	PREC_NONE,
	PREC_BIND,	// ::
	PREC_OR,	// |
	PREC_AND,	// &
	PREC_EQUALITY,	// = !=
	PREC_COMPARISON,// < > <= >=
	PREC_TERM,	// + -
	PREC_FACTOR,	// * /
	PREC_UNARY,	// ! -
	PREC_CALL,	// . ()
	PREC_PRIMARY
};

typedef struct parser_rules_t parser_rules_t;
struct parser_rules_t
{
	parser_fn_t	prefix;
	parser_fn_t	infix;
	precedence_t	precedence;
};

typedef struct compiler_t compiler_t;
struct compiler_t
{
	arena_t*	arena_const;
	arena_t*	arena_tmp;
	string_set*	strings;
	value_array_t*	constants;
	char*		src;
};

chunk_t*	compiler_run(compiler_t*);

#endif
