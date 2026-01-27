#include <lexer.h>
#include <evaluator.h>
#include <swissmap.h>
#include <builtins.h>
	
int
main()
{
	test_lexer_consume_token();
	test_parser_parse_expression();
	test_evaluate();
	test_polynomials();
	test_swissmap();

	return 0;
}
