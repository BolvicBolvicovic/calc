#include <vm/scanner.h>
#include <string.h>
#include <bool.h>

static __always_inline bool
is_eof(scanner_t* scanner) { return *scanner->current == 0; }

static __always_inline token_t
token_new(scanner_t* scanner, token_type_t type)
{
	return (token_t)
	{
		.type = type,
		.start= scanner->start,
		.size = (u32)(scanner->current - scanner->start),
		.line = scanner->line,
	};
}

static __always_inline token_t
token_error(scanner_t* scanner, char* msg)
{
	return (token_t)
	{
		.type = TK_ERR,
		.start= msg,
		.size = strlen(msg),
		.line = scanner->line,
	};
}

static __always_inline bool
scanner_match(scanner_t* scanner, char c)
{
	if (*scanner->current != c)
		return 0;

	scanner->current++;
	return 1;
}

static __always_inline void
scanner_skip_whitespace(scanner_t* scanner)
{
	// TODO: maybe do a switch instead
	while (is_space(*scanner->current) || *scanner->current == '#')
	{
		if (*scanner->current == '#')
		while (!is_eof(scanner) && *scanner->current != '\n')
			scanner->current++;

		if (*scanner->current == '\n')
			scanner->line++;

		scanner->current++;
	}
}

static __always_inline token_type_t
scanner_check_keyword(scanner_t* scanner, char* str, token_type_t type, u32 start, u32 size)
{
	char*	s = scanner->start + start;
	u32	t = scanner->current - scanner->start;
	
	if (t == start + size && memcmp(s, str, size) == 0)
		return type;

	return TK_ID;
}

static token_type_t
scanner_get_id_type(scanner_t* scanner)
{
	char	c = *scanner->start;

	switch (c)
	{
	case 'e': return scanner_check_keyword(scanner, "lse", TK_ELSE, 1, 3);
	case 'f': return *(scanner->start + 1) == 'u'
			? scanner_check_keyword(scanner, "nc", TK_FUNC, 2, 2)
			: *(scanner->start + 1) == 'a'
			? scanner_check_keyword(scanner, "lse", TK_FALSE, 2, 3)
			: scanner_check_keyword(scanner, "or", TK_FOR, 1, 2);
	case 'i': return *(scanner->start + 1) == 'f'
			? TK_IF
			: scanner_check_keyword(scanner, "nf", TK_INF, 1, 2);
	case 'l': return scanner_check_keyword(scanner, "et", TK_LET, 1, 2);
	case 'n': return scanner_check_keyword(scanner, "an", TK_NAN, 1, 2);
	case 'p': return scanner_check_keyword(scanner, "rint", TK_PRINT, 1, 4);
	case 'r': return scanner_check_keyword(scanner, "eturn", TK_RET, 1, 5);
	case 't': return scanner_check_keyword(scanner, "rue", TK_TRUE, 1, 3);
	}

	return TK_ID;
}

token_t
scanner_consume(scanner_t* scanner)
{
	scanner_skip_whitespace(scanner);
	scanner->start = scanner->current;

	if (is_eof(scanner))
		return token_new(scanner, TK_EOF);

	scanner->current++;

	char	c = *scanner->start;

	switch (c)
	{
	case '(': return token_new(scanner, TK_LP);
	case ')': return token_new(scanner, TK_RP);
	case '{': return token_new(scanner, TK_LB);
	case '}': return token_new(scanner, TK_RB);
	case ';': return token_new(scanner, TK_SC);
	case ',': return token_new(scanner, TK_COMMA);
	case '.': return token_new(scanner, TK_DOT);
	case '+': return token_new(scanner, TK_ADD);
	case '-': return token_new(scanner, TK_SUB);
	case '*': return token_new(scanner, TK_MUL);
	case '/': return token_new(scanner, TK_DIV);
	case '=': return token_new(scanner, TK_EQ);
	case '&': return token_new(scanner, TK_AND);
	case '|': return token_new(scanner, TK_OR);
	case '^': return token_new(scanner, TK_XOR);
	case '!': return scanner_match(scanner, '=')
			? token_new(scanner, TK_NOT_EQ)
			: token_new(scanner, TK_NOT);
	case '<': return scanner_match(scanner, '=')
			? token_new(scanner, TK_LESS_EQ)
			: token_new(scanner, TK_LESS);
	case '>': return scanner_match(scanner, '=')
			? token_new(scanner, TK_MORE_EQ)
			: token_new(scanner, TK_MORE);
	case ':': return scanner_match(scanner, ':')
		  	? token_new(scanner, TK_BIND)
			: token_error(scanner, "semi-colon missing for binding");
	case '"':
	{
		while (*scanner->current && *scanner->current != '"')
		{
			if (*scanner->current == '\n')
				scanner->line++;

			scanner->current++;
		}

		if (is_eof(scanner))
			return token_error(scanner, "Unterminated string");
		
		scanner->current++;

		return token_new(scanner, TK_STR);
	} break;
	}

	if (is_alpha(c))
	{
		while (is_digit(*scanner->current) || is_alpha(*scanner->current))
			scanner->current++;

		return token_new(scanner, scanner_get_id_type(scanner));
	}

	if (is_digit(c))
	{
		while (is_digit(*scanner->current))
			scanner->current++;

		if (*scanner->current ==  '.')
			scanner->current++;

		while (is_digit(*scanner->current))
			scanner->current++;

		return token_new(scanner, TK_NB);
	}

	return token_error(scanner, "Unknown token");
}

