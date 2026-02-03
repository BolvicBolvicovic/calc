#ifndef PLOT_H
#define PLOT_H

#include <c_types.h>
#include <evaluator.h>
#include <bits.h>

#define PLOT_PADDING_X	10
#define PLOT_PADDING_Y	4
#define PLOT_ROWS	40
#define PLOT_COLS	100

typedef enum plot_style_t	plot_style_t;
enum plot_style_t
{
	PLOT_LINE	= BIT0,
	PLOT_DOT	= BIT1,
	PLOT_TRAIT	= BIT2,
	PLOT_RED	= BIT3,
	PLOT_GREEN	= BIT4,
	PLOT_BLUE	= BIT5,
	PLOT_COMPLEX	= BIT6,
};

typedef struct dots_t	dots_t;
struct dots_t
{
	f64*	x;
	f64*	y;
};

s32	plot(arena_t* arena, token_t* x_name, f64 x_start, f64 x_end, f64 x_inc, ast_node_t* y, plot_style_t style);

#endif
