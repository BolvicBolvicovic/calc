#ifndef PLOT_H
#define PLOT_H

#include <c_types.h>
#include <bits.h>
#include <interpreter/evaluator.h>

#define PLOT_PADDING_X		10
#define PLOT_PADDING_Y		4
#define PLOT_ROWS		40
#define PLOT_COLS		100
#define PLOT_FRAC_BUF_SIZE	(PLOT_COLS + 1) * (PLOT_ROWS + 2) + 1 + PLOT_ROWS * PLOT_COLS * 30

typedef enum plot_opt_t	plot_opt_t;
enum plot_opt_t
{
	PLOT_COMPLEX		= BIT0,
	PLOT_SURF		= BIT1,
	PLOT_MANDELBROT		= BIT2,
	PLOT_JULIA		= BIT3,
	PLOT_BURNING_SHIP	= BIT4,
	PLOT_ZOOM_IN		= BIT5,
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

void	test_plot_get_dots_fract(void);
void	test_plot_get_dots(void);


#endif // TESTER

#endif // PLOT_H
