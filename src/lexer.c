#include <lexer.h>
#include <arena.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

static inline __attribute__((always_inline)) void
lexer_skip_whitespaces(lexer_t* lexer)
{
	assert(lexer);

	// TODO: Update so that newlines are a special case (i.e. a token limit)
	while (lexer->buffer_idx < lexer->buffer_size
		&& isspace(lexer->buffer[lexer->buffer_idx]))
	{
		if (lexer->buffer[lexer->buffer_idx] == '\n')
			lexer->buffer_line++;

		lexer->buffer_idx++;
	}
}


static void
lexer_stream(lexer_t* lexer)
{
	assert(lexer);

	lexer_skip_whitespaces(lexer);
	
	lexer->stream[lexer->stream_idx].start	= lexer->buffer + lexer->buffer_idx;
	lexer->stream[lexer->stream_idx].line	= lexer->buffer_line;

	if (lexer->buffer_idx == lexer->buffer_size)
	{
		lexer->stream[lexer->stream_idx].symbol = TK_EOI;
		return;
	}

	switch (lexer->buffer[lexer->buffer_idx])
	{
	case '+':
		lexer->stream[lexer->stream_idx].symbol = TK_ADD;
		goto end_single_char;
	case '-':
		lexer->stream[lexer->stream_idx].symbol = TK_SUB;
		goto end_single_char;
	case '*':
		lexer->stream[lexer->stream_idx].symbol = TK_MUL;
		goto end_single_char;
	case '/':
		lexer->stream[lexer->stream_idx].symbol = TK_DIV;
		goto end_single_char;
	case '^':
		lexer->stream[lexer->stream_idx].symbol = TK_POW;
		goto end_single_char;
	case '(':
		lexer->stream[lexer->stream_idx].symbol = TK_LP;
		goto end_single_char;
	case ')':
		lexer->stream[lexer->stream_idx].symbol = TK_RP;
		goto end_single_char;
	case ',':
		lexer->stream[lexer->stream_idx].symbol = TK_LIST;
		goto end_single_char;
	case ':':
		if (lexer->buffer_idx == lexer->buffer_size + 1
			|| lexer->buffer[lexer->buffer_idx + 1] != ':')
			break;

		lexer->stream[lexer->stream_idx].symbol = TK_BIND;
		lexer->stream[lexer->stream_idx].length = 2;
		lexer->buffer_idx += 2;
		return;
	}

	//TODO: Make proper identifiers token
	if (isalpha(lexer->buffer[lexer->buffer_idx])
		|| lexer->buffer[lexer->buffer_idx] == '_')
	{
		lexer->stream[lexer->stream_idx].symbol = TK_ID;
		lexer->stream[lexer->stream_idx].length = 0;

		while (isalpha(lexer->buffer[lexer->buffer_idx])
			|| lexer->buffer[lexer->buffer_idx] == '_')
		{
			lexer->stream[lexer->stream_idx].length++;
			lexer->buffer_idx++;
		}

		return;
	}
	
	u32	dots = 0;
	lexer->buffer_idx++;

skip_numbers:
	while (lexer->buffer_idx < lexer->buffer_size && isdigit(lexer->buffer[lexer->buffer_idx]))
		lexer->buffer_idx++;

	if (lexer->buffer[lexer->buffer_idx] == '.')
	{
		lexer->buffer_idx++;
		dots++;
		goto skip_numbers;
	}
		
	lexer->stream[lexer->stream_idx].length =
		lexer->buffer + lexer->buffer_idx - lexer->stream[lexer->stream_idx].start;
	
	if (dots <= 1)
	{
		lexer->stream[lexer->stream_idx].symbol = TK_FLOAT;
		return;
	}

	// TODO: Check produced code to see if it does not do anything weird
	lexer->stream[lexer->stream_idx].symbol = TK_ERR;
	return;

end_single_char:
	lexer->stream[lexer->stream_idx].length = 1;
	lexer->buffer_idx++;
	return;
}

static inline char*
lexer_token_to_string(token_t* t)
{
	switch (t->symbol)
	{
	case TK_ID:
		return "TK_ID";

	case TK_FLOAT:
		return "TK_FLOAT";

	case TK_ADD:
		return "TK_ADD";
	case TK_SUB:
		return "TK_SUB";
	case TK_DIV:
		return "TK_DIV";
	case TK_MUL:
		return "TK_MUL";
	case TK_POW:
		return "TK_POW";
	case TK_BIND:
		return "TK_BIND";
	case TK_LIST:
		return "TK_LIST";

	case TK_LP:
		return "TK_LP";
	case TK_RP:
		return "TK_RP";

	case TK_EOI:
		return "TK_EOI";
	case TK_ERR:
		return "TK_ERR";
	}

	return "";
}

void
lexer_init(lexer_t* lexer, char* buffer, u32 buffer_size)
{
	assert(lexer);
	assert(buffer);

	lexer->buffer		= buffer;
	lexer->buffer_size	= buffer_size;
	lexer->buffer_line	= 1;
	lexer->buffer_idx	= 0;
	lexer->stream[0].symbol	= TK_EOI;
	lexer->stream[1].symbol	= TK_EOI;
	lexer->stream[0].start	= 0;
	lexer->stream[1].start	= 0;
	lexer->stream_idx	= 0;
	
	lexer_stream(lexer);

	return;
}

token_t*
lexer_consume_token(lexer_t* lexer)
{
	assert(lexer);

	token_t*	token = &lexer->stream[lexer->stream_idx];

	lexer->stream_idx = !lexer->stream_idx;

	// TODO: thread it
	lexer_stream(lexer);

	return token;
}

inline void
lexer_print_token(token_t* token)
{
	assert(token);
	char	str[100] = {0};

	memcpy(str, token->start, token->length);

	printf(
	"Token %s at %p:\n"
	"{\n"
	"	symbol	: %s,\n"
	"	start	: %p,\n"
	"	length	: %d,\n"
	"	line	: %d,\n"
	"}\n", str, token, lexer_token_to_string(token), token->start, token->length, token->line);
}

#ifdef TESTER

void
test_lexer_consume_token(void)
{
	char*	test = "x :: 5 * (10.0 + 7) + milli(5)";
	lexer_t	lexer;

	lexer_init(&lexer, test, 30);

	token_t*	token = lexer_consume_token(&lexer);

	assert(token->symbol == TK_ID);
	assert(token->start == test);
	assert(token->length == 1);
	assert(token->line == 1);

	token = lexer_consume_token(&lexer);
	assert(token->symbol == TK_BIND);
	assert(token->start == test + 2);
	assert(token->length == 2);
	assert(token->line == 1);
	
	token = lexer_consume_token(&lexer);
	assert(token->symbol == TK_FLOAT);
	assert(token->start == test + 5);
	assert(token->length == 1);
	assert(token->line == 1);

	token = lexer_consume_token(&lexer);
	assert(token->symbol == TK_MUL);
	assert(token->start == test + 7);
	assert(token->length == 1);
	assert(token->line == 1);
	
	token = lexer_consume_token(&lexer);
	assert(token->symbol == TK_LP);
	assert(token->start == test + 9);
	assert(token->length == 1);
	assert(token->line == 1);
	
	token = lexer_consume_token(&lexer);
	assert(token->symbol == TK_FLOAT);
	assert(token->start == test + 10);
	assert(token->length == 4);
	assert(token->line == 1);

	token = lexer_consume_token(&lexer);
	assert(token->symbol == TK_ADD);
	assert(token->start == test + 15);
	assert(token->length == 1);
	assert(token->line == 1);

	token = lexer_consume_token(&lexer);
	assert(token->symbol == TK_FLOAT);
	assert(token->start == test + 17);
	assert(token->length == 1);
	assert(token->line == 1);
	
	token = lexer_consume_token(&lexer);
	assert(token->symbol == TK_RP);
	assert(token->start == test + 18);
	assert(token->length == 1);
	assert(token->line == 1);
	
	token = lexer_consume_token(&lexer);
	assert(token->symbol == TK_ADD);
	assert(token->start == test + 20);
	assert(token->length == 1);
	assert(token->line == 1);
	
	token = lexer_consume_token(&lexer);
	assert(token->symbol == TK_ID);
	assert(token->start == test + 22);
	assert(token->length == 5);
	assert(token->line == 1);
	
	token = lexer_consume_token(&lexer);
	assert(token->symbol == TK_LP);
	assert(token->start == test + 27);
	assert(token->length == 1);
	assert(token->line == 1);
	
	token = lexer_consume_token(&lexer);
	assert(token->symbol == TK_FLOAT);
	assert(token->start == test + 28);
	assert(token->length == 1);
	assert(token->line == 1);

	token = lexer_consume_token(&lexer);
	assert(token->symbol == TK_RP);
	assert(token->start == test + 29);
	assert(token->length == 1);
	assert(token->line == 1);
	
	token = lexer_consume_token(&lexer);
	assert(token->symbol == TK_EOI);
	assert(token->line == 1);
}

#endif // TESTER
