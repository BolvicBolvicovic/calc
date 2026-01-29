#include <errors.h>
#include <stdio.h>

static const char*	err_tab[] =
{
	"Unknown function",
	"Division by zero",
	"Wrong argument to function",
	"Out of bound",
	"Binary operator missing left operand",
	"Binding operation on already bound variable",
	"Token is not a binary operator",
	"Operation with unbound variable not allowed",
};

void
error_print(return_value_t* err)
{
	printf("Error at line %d here ->%s: %s",
		err->token->line, err->token->start, err_tab[(error_code_t)err->f]);
}
