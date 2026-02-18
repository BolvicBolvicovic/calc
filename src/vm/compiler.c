#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <macros.h>
#include <vm/compiler.h>

#define PARSER_EMIT_BYTE(parser, byte)	\
	(parser)->chunk =		\
		chunk_write((parser)->context->arena_line, (parser)->chunk, byte, (parser)->prev.line)
#define PARSER_EMIT_BYTES(parser, ...)			\
do							\
{							\
	u8	_bytes[] = {__VA_ARGS__};		\
	for (u32 i = 0; i < sizeof(_bytes); i++)	\
		PARSER_EMIT_BYTE(parser, _bytes[i]);	\
} while (0)

static void	parser_parse_precedence(parser_t*, precedence_t);
static void	parser_expression(parser_t*);
static void	parser_number(parser_t*);
static void	parser_parenthesis(parser_t*);
static void	parser_unary(parser_t*);
static void	parser_binary(parser_t*);
static void	parser_literal(parser_t*);
static void	parser_array(parser_t*);
static void	parser_string(parser_t* parser);
static void	parser_identifier(parser_t* parser);

static parser_rules_t rules[]	=
{
	[TK_ID]		= {parser_identifier,	NULL,		PREC_NONE},
	[TK_NB]		= {parser_number,	NULL,		PREC_NONE},
	[TK_NAN]	= {parser_literal,	NULL,		PREC_NONE},
	[TK_INF]	= {parser_literal,	NULL,		PREC_NONE},
	[TK_TRUE]	= {parser_literal,	NULL,		PREC_NONE},
	[TK_FALSE]	= {parser_literal,	NULL,		PREC_NONE},

	[TK_FUNC]	= {NULL,		NULL,		PREC_NONE},
	[TK_FOR]	= {NULL,		NULL,		PREC_NONE},
	[TK_IF]		= {NULL,		NULL,		PREC_NONE},
	[TK_ELSE]	= {NULL,		NULL,		PREC_NONE},
	[TK_RET]	= {NULL,		NULL,		PREC_NONE},
	[TK_LET]	= {NULL,		NULL,		PREC_NONE},

	[TK_ADD]	= {parser_unary,	parser_binary,	PREC_TERM},
	[TK_SUB]	= {parser_unary,	parser_binary,	PREC_TERM},
	[TK_MUL]	= {NULL,		parser_binary,	PREC_FACTOR},
	[TK_DIV]	= {NULL,		parser_binary,	PREC_FACTOR},

	[TK_EQ]		= {NULL,		parser_binary,	PREC_EQUALITY},
	[TK_LESS]	= {NULL,		parser_binary,	PREC_COMPARISON},
	[TK_MORE]	= {NULL,		parser_binary,	PREC_COMPARISON},
	[TK_NOT]	= {parser_unary,	NULL,		PREC_UNARY},
	[TK_MORE_EQ]	= {NULL,		parser_binary,	PREC_COMPARISON},
	[TK_LESS_EQ]	= {NULL,		parser_binary,	PREC_COMPARISON},
	[TK_NOT_EQ]	= {NULL,		parser_binary,	PREC_EQUALITY},

	[TK_AND]	= {NULL,		NULL,		PREC_AND},
	[TK_OR]		= {NULL,		NULL,		PREC_OR},
	[TK_XOR]	= {NULL,		parser_binary,	PREC_OR},

	[TK_LP]		= {parser_parenthesis,	NULL,		PREC_NONE},
	[TK_RP]		= {parser_array,	NULL,		PREC_NONE},
	[TK_LB]		= {NULL,		NULL,		PREC_NONE}, 
	[TK_RB]		= {NULL,		NULL,		PREC_NONE},
	[TK_STR]	= {parser_string,	NULL,		PREC_NONE},

	[TK_COMMA]	= {NULL,		NULL,		PREC_NONE},
	[TK_DOT]	= {NULL,		NULL,		PREC_NONE},
	[TK_SC]		= {NULL,		NULL,		PREC_NONE},
	[TK_BIND]	= {NULL,		NULL,		PREC_BIND},

	[TK_ERR]	= {NULL,		NULL,		PREC_NONE},
	[TK_EOF]	= {NULL,		NULL,		PREC_NONE},
};

static inline bool
parser_advance(parser_t* parser)
{
	parser->prev	= parser->current;
	parser->current	= scanner_consume(&parser->scanner);

	if (parser->current.type == TK_ERR)
	{
		parser->error = 1;
		parser->panic = 1;
		ERROR(parser->current.line, parser->current.start);
		return 0;
	}

	return 1;
}

static inline bool
parser_consume(parser_t* parser, token_type_t type, char* err)
{
	if (parser->current.type == type)
		return parser_advance(parser);

	parser->error = 1;
	parser->panic = 1;
	ERROR(parser->current.line, err);
	return 0;
}

static void
parser_parse_precedence(parser_t* parser, precedence_t precedence)
{
	if (parser->current.type == TK_SC)
		return;

	parser_advance(parser);

	parser_fn_t	prefix_rule = rules[parser->prev.type].prefix;

	if (!prefix_rule)
	{
		parser->error = 1;
		parser->panic = 1;
		ERROR(parser->prev.line, "Expect expression");
		return;
	}
	
	prefix_rule(parser);

	while (precedence <= rules[parser->current.type].precedence)
	{
		parser_advance(parser);

		parser_fn_t	infix_rule = rules[parser->prev.type].infix;
		
		if (!infix_rule)
		{
			parser->error = 1;
			parser->panic = 1;
			ERROR(parser->prev.line, "Expect expression");
			return;
		}

		infix_rule(parser);
	}
}

static __always_inline void
parser_expression(parser_t* parser) { parser_parse_precedence(parser, PREC_BIND); }

static void
parser_identifier(parser_t* parser)
{
	arena_t*	arena	= parser->context->arena_line;
	string_t*	str	= string_new(arena, parser->prev.start, parser->prev.size);

	parser->chunk = chunk_write_const(arena, parser->chunk, VAL_AS_STR(str), parser->prev.line);

	PARSER_EMIT_BYTE(parser, OP_GET_GLOBAL);
}

static void
parser_number(parser_t* parser)
{
	value_t	val = VAL_AS_NB(strtod(parser->prev.start, 0));
	parser->chunk = chunk_write_const(parser->context->arena_line, parser->chunk, val, parser->prev.line);
}

static void
parser_parenthesis(parser_t* parser)
{
	parser_expression(parser);
	parser_consume(parser, TK_RP, "Expect ')' after expression");
}

static void
parser_literal(parser_t* parser)
{
	switch (parser->prev.type)
	{
	case TK_NAN	: PARSER_EMIT_BYTE(parser, OP_NAN); break;
	case TK_INF	: PARSER_EMIT_BYTE(parser, OP_INF); break;
	case TK_TRUE	: PARSER_EMIT_BYTE(parser, OP_TRUE); break;
	case TK_FALSE	: PARSER_EMIT_BYTE(parser, OP_FALSE); break;
	default: return;
	}
}

static void
parser_array(parser_t* parser)
{
	// TODO: implement array handling
	parser_expression(parser);
	parser_consume(parser, TK_RB, "Expect '}' after expression");
}

static void
parser_string(parser_t* parser)
{
	u32		line	= parser->prev.line;
	u32		size	= parser->prev.size - 2;
	char*		start	= parser->prev.start + 1;
	string_set*	strings	= parser->context->strings;
	string_t	tmp_str	= {.size=size, .length=size, .buf=start};
	string_t**	has_str	= string_set_has_key(strings, &tmp_str);

	if (has_str)
	{
		parser->chunk	= chunk_write_const(
				parser->context->arena_line, parser->chunk, VAL_AS_STR(*has_str), line);
		return;
	}
	
	string_t*	str = string_new(parser->context->arena_glb, start, size);
	
	parser->chunk	= chunk_write_const(parser->context->arena_line, parser->chunk, VAL_AS_STR(str), line);
	string_set_put(strings, str, line);
}

static void
parser_unary(parser_t* parser)
{
	token_type_t	op_type = parser->prev.type;

	parser_parse_precedence(parser, PREC_UNARY);

	switch (op_type)
	{
	case TK_SUB: PARSER_EMIT_BYTE(parser, OP_NEG); break;
	case TK_NOT: PARSER_EMIT_BYTE(parser, OP_NOT); break;
	default: return;
	}
}

static void
parser_binary(parser_t* parser)
{
	token_type_t	op_type = parser->prev.type;
	parser_rules_t*	rule	= &rules[op_type];

	parser_parse_precedence(parser, rule->precedence + 1);

	switch (op_type)
	{
	case TK_ADD	: PARSER_EMIT_BYTE(parser, OP_ADD); break;
	case TK_SUB	: PARSER_EMIT_BYTE(parser, OP_SUB); break;
	case TK_MUL	: PARSER_EMIT_BYTE(parser, OP_MUL); break;
	case TK_DIV	: PARSER_EMIT_BYTE(parser, OP_DIV); break;
	case TK_EQ	: PARSER_EMIT_BYTE(parser, OP_EQ); break;
	case TK_NOT_EQ	: PARSER_EMIT_BYTE(parser, OP_NOT_EQ); break;
	case TK_MORE	: PARSER_EMIT_BYTE(parser, OP_MORE); break;
	case TK_MORE_EQ	: PARSER_EMIT_BYTE(parser, OP_MORE_EQ); break;
	case TK_LESS	: PARSER_EMIT_BYTE(parser, OP_LESS); break;
	case TK_LESS_EQ	: PARSER_EMIT_BYTE(parser, OP_LESS_EQ); break;
	default: return;
	}
}

static bool
parser_match(parser_t* parser, token_type_t type)
{
	if (parser->current.type != type)
		return 0;

	parser_advance(parser);
	return 1;
}

static void
parser_statement(parser_t* parser)
{
	if (parser_match(parser, TK_PRINT))
	{
		parser_expression(parser);
		parser_consume(parser, TK_SC, "Expect ';' at end of expression");
		PARSER_EMIT_BYTE(parser, OP_PRINT);
	}
	else
	{
		parser_expression(parser);
		parser_consume(parser, TK_SC, "Expect ';' at end of expression");
		PARSER_EMIT_BYTE(parser, OP_POP);
	}
}

static void
parser_var(parser_t* parser, char* err)
{
	parser_consume(parser, TK_ID, err);

	arena_t*	arena	= parser->context->arena_glb;
	string_t*	str	= string_new(arena, parser->prev.start, parser->prev.size);

	parser->chunk = chunk_write_const(arena, parser->chunk, VAL_AS_STR(str), parser->prev.line);
}

static void
parser_var_decl(parser_t* parser)
{
	parser_var(parser, "Expect variable name");

	if (parser_match(parser, TK_BIND))
		parser_expression(parser);
	else
		PARSER_EMIT_BYTE(parser, OP_ZERO);

	parser_consume(parser, TK_SC, "Expect ';' at end of expression");
	PARSER_EMIT_BYTE(parser, OP_DEFINE_GLOBAL);
}

static void
parser_declaration(parser_t* parser)
{
	if (parser_match(parser, TK_LET))
		parser_var_decl(parser);
	else
		parser_statement(parser);

	if (!parser->panic)
		return;

	parser->panic = 0;

	while (parser->current.type != TK_EOF)
	{
		if (parser->prev.type == TK_SC)
			return;

		switch (parser->current.type)
		{
		case TK_FUNC:
		case TK_LET:
		case TK_FOR:
		case TK_IF:
		case TK_PRINT:
		case TK_RET:
			return;
		default:
			parser_advance(parser);
		}
	}
}

chunk_t*
compiler_run(context_t* context, char* src)
{
	chunk_t*	expr	= chunk_new(context, 500);
	parser_t*	parser	= &(parser_t)
	{
		.chunk		= expr,
		.context	= context,
		.scanner	=
		{
			.start		= src,
			.current	= src,
			.line		= 1,
		}
	};

	if (!parser_advance(parser))
		return 0;

	while (!parser_match(parser, TK_EOF))
		parser_declaration(parser);

	if (parser->error || !parser_consume(parser, TK_EOF, "Expect end of expression"))
	{
		chunk_dissassemble(expr);
		return 0;
	}
	
	chunk_write(context->arena_line, expr, OP_RET, parser->prev.line);

	return expr;
}
