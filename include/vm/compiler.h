#ifndef COMPILER_H
#define COMPILER_H

#include <vm/chunk.h>
#include <vm/scanner.h>
#include <vm/context.h>

#define LOCAL_COUNT	(UINT8_MAX + 1)

typedef struct local_t	local_t;
struct local_t
{
	token_t	name;
	u32	depth;
};

typedef struct parser_t parser_t;
struct parser_t
{
	token_t		prev;
	token_t		current;
	scanner_t	scanner;
	u32		local_count;
	u32		scope_depth;
	local_t		locals[LOCAL_COUNT];
	bool		error;
	bool		panic;
	chunk_t*	chunk;
	context_t*	context;
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

chunk_t*	compiler_run(context_t*, char* src);

#endif
