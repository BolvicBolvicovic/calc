#ifndef ERRORS_H
#define ERRORS_H

#include <lexer.h>

typedef enum error_code_t error_code_t;
enum error_code_t
{
	ERR_UNKNOWN_FUNC,
	ERR_DIV_BY_ZERO,
	ERR_WRONG_ARG,
	ERR_OUT_OF_BOUND,
	ERR_BINARY_OP_MISSING_LEFT,
	ERR_BINDING_ALREADY_DEFINED,
	ERR_TOKEN_IS_NOT_BIN_OP,
	ERR_OPERATION_UNBOUND_VAR,
	ERR_MISSING_CLOSING_PARENTHESIS,
	ERR_MISSING_OPERATOR_ARGUMENT,
};

void	error_print(token_t* err, error_code_t code);

#endif
