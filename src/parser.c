#include <parser.h>
#include <arena.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static inline void 
ast_node_init(ast_node_t* node, expr_t type, token_t* token)
{
	assert(node);
	assert(token);

	node->type = type;
	node->left = 0;
	node->right= 0;
	
	memcpy(&node->token, token, sizeof(token_t));
}

static inline s32
parser_get_precedence(symbol_t symbol)
{
	switch (symbol)
	{
	case TK_BIND:
		return 2;
	case TK_LIST:
		return 4;
	case TK_TMP_BIND:
		return 6;
	case TK_ADD:
	case TK_SUB:
		return 8;
	case TK_MUL:
	case TK_DIV:
	case TK_POW:
		return 10;
	case TK_LP:
		return 12;
	default:
		return 0;
	}
}

static ast_node_t*
parser_parse_prefix(arena_t* arena, lexer_t* lexer)
{
	ast_node_t*	node = 0;
	token_t*	token= &lexer->stream[lexer->stream_idx];

	switch (token->symbol)
	{
	case TK_ID:
		node = ARENA_PUSH_STRUCT(arena, ast_node_t);
		ast_node_init(node, EXPR_ID, lexer_consume_token(lexer));
		break;
	case TK_FLOAT:
		node = ARENA_PUSH_STRUCT(arena, ast_node_t);
		ast_node_init(node, EXPR_CONST, lexer_consume_token(lexer));
		break;
	case TK_LP:
		lexer_consume_token(lexer);
		node 		= parser_parse_expression(arena, lexer, 0);

		if (!(TK_RP == lexer_consume_token(lexer)->symbol))
		{
			node->type	= EXPR_ERR;
			node->err_code	= ERR_MISSING_CLOSING_PARENTHESIS;
		}
		break;
	case TK_SUB:
		node = ARENA_PUSH_STRUCT(arena, ast_node_t);
		ast_node_init(node, EXPR_UOP, lexer_consume_token(lexer));

		ast_node_t*	operand = parser_parse_expression(arena, lexer, 100);

		node->left = operand;
		break;
	default:
	}

	return node;
}

static ast_node_t*
parser_parse_infix(arena_t* arena, lexer_t* lexer, ast_node_t* left)
{
	ast_node_t*	node = 0;

	switch (lexer->stream[lexer->stream_idx].symbol)
	{
	case TK_ADD:
	case TK_SUB:
	case TK_MUL:
	case TK_DIV:
	case TK_POW:
	case TK_BIND:
	case TK_TMP_BIND:
	case TK_LIST:
		node = ARENA_PUSH_STRUCT(arena, ast_node_t);
		ast_node_init(node, EXPR_BOP, lexer_consume_token(lexer));
		node->left = left;
		node->right = parser_parse_expression(
			arena, lexer, parser_get_precedence(node->token.symbol) + 1);
		break;
	case TK_LP:
		lexer_consume_token(lexer);
		left->left = parser_parse_expression(arena, lexer, 0);
		node = left;
		if (!(TK_RP == lexer_consume_token(lexer)->symbol))
		{
			node->type	= EXPR_ERR;
			node->err_code	= ERR_MISSING_CLOSING_PARENTHESIS;
		}
		break;
	default:
		node = ARENA_PUSH_STRUCT(arena, ast_node_t);
		ast_node_init(node, EXPR_ERR, lexer_consume_token(lexer));
		node->left = left;
	}

	return node;
}

ast_node_t*
parser_parse_expression(arena_t* arena, lexer_t* lexer, s32 precedence)
{
	assert(arena);
	assert(lexer);

	ast_node_t*	left = parser_parse_prefix(arena, lexer);

	while (precedence < parser_get_precedence(lexer->stream[lexer->stream_idx].symbol))
		left = parser_parse_infix(arena, lexer, left);

	return left;
}

ast_node_t*
parser_save_tree(arena_t* arena, ast_node_t* tree)
{
	if (!tree)
		return 0;

	ast_node_t*	l = parser_save_tree(arena, tree->left);
	ast_node_t*	r = parser_save_tree(arena, tree->right);
	ast_node_t*	s = ARENA_PUSH_STRUCT(arena, ast_node_t);
	char*		ts= ARENA_PUSH_ARRAY(arena, char, tree->token.length);

	memcpy(s, tree, sizeof(ast_node_t));
	memcpy(ts, s->token.start, s->token.length);

	s->left = l;
	s->right= r;
	s->token.start = ts;

	return s;
}

#ifdef TESTER

void
parser_print_preorder(ast_node_t* expr, s32 d)
{
	if (!expr)
		return;
	
	printf("deepness: %d, token: %s\n", d, expr->token.start);
	//lexer_print_token(&expr->token);
	parser_print_preorder(expr->left, d + 1);
	parser_print_preorder(expr->right, d + 1);
}

void
test_parser_save_tree(void)
{
	ast_node_t*	tree = 0;
	arena_t*	a1   = ARENA_ALLOC();
	arena_t*	a2   = ARENA_ALLOC();
	char*		e    = "-5 - 10.0 * 5";
	char*		test = ARENA_PUSH_ARRAY(a1, char, 13);
	lexer_t		lexer;

	memcpy(test, e, 13);

	lexer_init(&lexer, test, 13);
	tree = parser_parse_expression(a1, &lexer, 0);
	//printf("First print:\n");
	//parser_print_preorder(tree, 0);

	tree = parser_save_tree(a2, tree);

	memset(a1, 0, a1->commit_size);
	arena_release(a1);
	//printf("Second print:\n");
	//parser_print_preorder(tree, 0);
	// tree should look like this:
	// 			[ TK_SUB ]
	// 	[ TK_SUB ]			[ TK_MUL ]
	// [ TK_FLOAT ]			[ TK_FLOAT ]	[ TK_FLOAT ]
	assert(tree);
	// 3. return - with left = -(5) and right = * (10.0, 5)
	assert(tree->token.symbol == TK_SUB);

	// 2. left = - (its left is -(5))
	//	2.3. left = * (its left is 10.0 and its right is 5)
	assert(tree->right->token.symbol == TK_MUL);
	//	2.2. left = 10.0
	assert(tree->right->left->token.symbol == TK_FLOAT);
	//	2.1. right = 5
	assert(tree->right->right->token.symbol == TK_FLOAT);

	// 1. left = -
	assert(tree->left->token.symbol == TK_SUB);
	// 	1.1. left = 5
	assert(tree->left->left->token.symbol == TK_FLOAT);
	
	arena_release(a2);
}

void
test_parser_parse_expression(void)
{
	char*		test = "-5 - 10.0 * 5";
	ast_node_t*	tree = 0;
	arena_t*	arena= ARENA_ALLOC();
	lexer_t		lexer;

	lexer_init(&lexer, test, 13);

	tree = parser_parse_expression(arena, &lexer, 0);

	// tree should look like this:
	// 			[ TK_SUB ]
	// 	[ TK_SUB ]			[ TK_MUL ]
	// [ TK_FLOAT ]			[ TK_FLOAT ]	[ TK_FLOAT ]
	assert(tree);
	// 3. return - with left = -(5) and right = * (10.0, 5)
	assert(tree->token.symbol == TK_SUB);

	// 2. left = - (its left is -(5))
	//	2.3. left = * (its left is 10.0 and its right is 5)
	assert(tree->right->token.symbol == TK_MUL);
	//	2.2. left = 10.0
	assert(tree->right->left->token.symbol == TK_FLOAT);
	//	2.1. right = 5
	assert(tree->right->right->token.symbol == TK_FLOAT);

	// 1. left = -
	assert(tree->left->token.symbol == TK_SUB);
	// 	1.1. left = 5
	assert(tree->left->left->token.symbol == TK_FLOAT);

	arena_clear(arena);

	char*		test2 = "(-5 - 10.0) * 5";

	lexer_init(&lexer, test2, 15);

	tree = parser_parse_expression(arena, &lexer, 0);

	// tree should look like this:
	// 				[ TK_MUL ]
	// 		[ TK_SUB ]			[ TK_FLOAT ]
	// 	[ TK_SUB ]	[ TK_FLOAT ]
	// [ TK_FLOAT ]
	
	assert(tree);
	assert(tree->token.symbol == TK_MUL);
	
	assert(tree->left->token.symbol == TK_SUB);
	assert(tree->left->right->token.symbol == TK_FLOAT);
	assert(tree->left->left->token.symbol == TK_SUB);
	assert(tree->left->left->left->token.symbol == TK_FLOAT);

	assert(tree->right->token.symbol == TK_FLOAT);
	
	arena_clear(arena);

	char*		test3 = "2 * ampere(5)";

	lexer_init(&lexer, test3, 13);

	tree = parser_parse_expression(arena, &lexer, 0);

	// tree should look like this:
	// 			[ TK_MUL ]
	// 	[ TK_FLOAT ]			[ TK_ID ]
	//				[ TK_FLOAT ]
	
	assert(tree);
	assert(tree->token.symbol == TK_MUL);
	
	assert(tree->left->token.symbol == TK_FLOAT);
	assert(tree->right->token.symbol == TK_ID);
	assert(tree->right->left->token.symbol == TK_FLOAT);

	arena_release(arena);
}

#endif // TESTER
