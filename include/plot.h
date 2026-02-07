#ifndef PLOT_H
#define PLOT_H

#include <c_types.h>
#include <evaluator.h>
#include <bits.h>

#define PLOT_PADDING_X	10
#define PLOT_PADDING_Y	4
#define PLOT_ROWS	40
#define PLOT_COLS	100

typedef enum plot_opt_t	plot_opt_t;
enum plot_opt_t
{
	PLOT_LINE		= BIT0,
	PLOT_STAR		= BIT1,
	PLOT_TRAIT		= BIT2,
	PLOT_RED		= BIT3,
	PLOT_GREEN		= BIT4,
	PLOT_BLUE		= BIT5,
	PLOT_COMPLEX		= BIT6,
	PLOT_SURF		= BIT7,
	PLOT_MANDELBROT		= BIT8,
	PLOT_JULIA		= BIT9,
	PLOT_BURNING_SHIP	= BIT10,
	PLOT_ZOOM_IN		= BIT11,
};

typedef struct dots_t	dots_t;
struct dots_t
{
	f64*	x;
	f64*	y;
	f64	x_min;
	f64	x_max;
	f64	y_min;
	f64	y_max;
	f64	z_min;
	f64	z_max;
	
	bool	err;
	union
	{
		s32	err_code;
		f64*	z;
	};
};

s32	plot(evaluate_param_t* 	param,
	token_t*		x_name,
	f64			x_start, f64 x_end, f64 x_inc,
	ast_node_t*		y,
	u64			opt);

#ifdef TESTER



#endif // TESTER

#endif // PLOT_H
