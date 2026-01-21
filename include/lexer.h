#ifndef LEXER_H
#define LEXER_H

#include <c_types.h>

typedef enum symbol_t symbol_t;
enum __attribute__((mode(SI))) symbol_t
{
	// Identifiers
	TK_ID,

	// Types
	TK_FLOAT,
	TK_INT,

	// Operations
	TK_ADD,
	TK_SUB,
	TK_DIV,
	TK_MUL,
	TK_POW,
	TK_BIND,

	// Delimiters
	TK_LP,
	TK_RP,

	// End Of Input
	TK_EOI,
	TK_ERR
};

typedef struct token_t token_t;
struct token_t
{
	symbol_t	symbol;
	char*		start;
	u32		length;
	u32		line;
};

typedef struct lexer_t lexer_t;
struct lexer_t
{
	char*	buffer;
	u32	buffer_idx;
	u32	buffer_size;
	u32	buffer_line;
	token_t	stream[2];
	s8	stream_idx;
};

token_t*	lexer_consume_token(lexer_t*);
void		lexer_init(lexer_t*, char* buffer, u32 buffer_size);
void		lexer_print_token(token_t*);

#ifdef TESTER

void	test_lexer_consume_token(void);

#endif // TESTER

#endif // LEXER_H
