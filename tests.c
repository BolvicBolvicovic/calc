#include <pthread.h>
#include <signal.h>
#include <swissmap.h>
#include <interpreter/lexer.h>
#include <interpreter/evaluator.h>
#include <interpreter/builtins.h>
#include <interpreter/plot.h>
#include <vm/chunk.h>
#include <vm/value.h>
	
#define TEST_COUNT	11
#define THREAD_FUNC_CAST(func) (void * (*)(void *))(func)
#define TEST_RUN(test, thread, threads) \
	pthread_create(&threads[thread], NULL, THREAD_FUNC_CAST(test), NULL)
typedef struct sigaction	sigaction_t;

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
	sigaction_t	sa = {0};
	pthread_t	threads[TEST_COUNT];

	sa.sa_handler	= on_ctrl_c;
	sa.sa_flags	= SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);

	// Note: Tests for interpreter
	TEST_RUN(test_lexer_consume_token,	0, threads);
	TEST_RUN(test_parser_parse_expression,	1, threads);
	TEST_RUN(test_parser_save_tree,		2, threads);
	TEST_RUN(test_evaluate,			3, threads);
	TEST_RUN(test_polynomials,		4, threads);
	TEST_RUN(test_swissmap,			5, threads);
	TEST_RUN(test_plot_get_dots,		6, threads);
	TEST_RUN(test_plot_get_dots_fract,	7, threads);

	// Note: Tests for VM
	TEST_RUN(test_chunk_write,		8, threads);
	TEST_RUN(test_value_array,		9, threads);
	TEST_RUN(test_chunk_write_const,	10, threads);

	for(u32 i = 0; i < TEST_COUNT; i++)
		pthread_join(threads[i], 0);

	return 0;
}
