#ifndef PARSER_H
#define PARSER_H

#include <c_types.h>
#include <lexer.h>
#include <arena.h>
#include <errors.h>

typedef enum expr_t expr_t;
enum expr_t
{
	EXPR_ID,
	EXPR_CONST,
	EXPR_UOP,
	EXPR_BOP,
	EXPR_ERR,
};

typedef struct ast_node_t ast_node_t;
struct ast_node_t
{
	expr_t		type;
	token_t		token;
	union
	{
		ast_node_t*	left;
		error_code_t	err_code;
	};
	ast_node_t*	right;
};

ast_node_t*	parser_parse_expression(arena_t*, lexer_t*, s32 precedence);

#ifdef TESTER

void	parser_print_preorder(ast_node_t* expr, s32 d);
void	test_parser_parse_expression(void);

#endif // TESTER

#endif // PARSER_H
