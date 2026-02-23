#ifndef SCANNER_H
#define SCANNER_H

#include <c_types.h>

typedef enum token_type_t token_type_t;
enum token_type_t
{
	TK_ID, TK_NB, TK_NAN, TK_INF, TK_TRUE, TK_FALSE,
	TK_FUNC, TK_FOR, TK_IF, TK_ELSE, TK_RET, TK_LET, TK_AND, TK_OR,
	
	TK_ADD, TK_SUB, TK_MUL, TK_DIV,
	TK_EQ, TK_LESS, TK_MORE, TK_NOT, TK_LESS_EQ, TK_MORE_EQ, TK_NOT_EQ,
	TK_ANDL, TK_ORL, TK_XOR, TK_INC, TK_DEC,

	TK_LP, TK_RP, TK_LB, TK_RB, TK_STR,

	TK_COMMA, TK_DOT, TK_SC, TK_BIND,

	TK_PRINT, TK_ERR, TK_EOF,
};

typedef struct token_t token_t;
struct token_t
{
	token_type_t	type;
	char*		start;
	u32		line;
	u32		size;
};

typedef struct scanner_t scanner_t;
struct scanner_t
{
	char*	start;
	char*	current;
	s32	line;
};

token_t	scanner_consume(scanner_t*);

#endif
