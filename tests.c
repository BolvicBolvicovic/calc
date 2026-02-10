#include <lexer.h>
#include <evaluator.h>
#include <swissmap.h>
#include <builtins.h>
#include <signal.h>
#include <plot.h>
	
volatile s32	interrupted = 0;

static void
on_ctrl_c(s32 sig)
{
	(void)sig;
	interrupted = 1;
}

int
main()
{
	struct sigaction	sa = {0};

	sa.sa_handler	= on_ctrl_c;
	sa.sa_flags	= SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	test_lexer_consume_token();
	test_parser_parse_expression();
	test_parser_save_tree();
	test_evaluate();
	test_polynomials();
	test_swissmap();
	test_plot_get_dots();
	test_plot_get_dots_fract();

	return 0;
}
