#include <lexer.h>
#include <evaluator.h>
#include <swissmap.h>
	
int
main()
{
	test_lexer_consume_token();
	test_parser_parse_expression();
	test_evaluator_atoi();
	test_evaluator_atof();
	test_evaluate();
	test_swissmap();

	return 0;
}
