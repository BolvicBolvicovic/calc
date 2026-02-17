#include <stdio.h>
#include <interpreter/errors.h>

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
	"Missing closing parenthesis",
	"Missing operator's argument",
};

void
error_print(token_t* err, error_code_t code)
{
	printf("Error at line %d here ->%s: %s",
		err->line, err->start, err_tab[code]);
}
