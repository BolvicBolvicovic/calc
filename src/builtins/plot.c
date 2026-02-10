#include <plot.h>
#include <stdio.h>
#include <parser.h>
#include <math.h>

extern s32	interrupted;

static dots_t*
plot_get_dots_fract
(
	evaluate_param_t* param,
	token_t*	x_name,
	ast_node_t*	y,
	f64		bound,
	f64		center_re,
	f64		center_im,
	u64		range,
	f64		threshold,
	u64		opts
)
{
	dots_t*	dots = ARENA_PUSH_STRUCT(param->arena_tmp, dots_t);
	u64	bufsz= (PLOT_ROWS - PLOT_PADDING_Y) * (PLOT_COLS - PLOT_PADDING_X);

	dots->err= 0;
	dots->x	= ARENA_PUSH_ARRAY(param->arena_tmp, f64, bufsz);
	dots->y = ARENA_PUSH_ARRAY(param->arena_tmp, f64, bufsz);
	dots->z = ARENA_PUSH_ARRAY(param->arena_tmp, f64, bufsz);

	variables_map_put(param->vmap_tmp, x_name, &(return_value_t)
		{.c	= 0.0 + 0.0*I,
		.type	= RET_COMPLEX,
		.oom	= OOM_NONE});

	return_value_t*	param_var	= *variables_map_get(param->vmap_tmp, x_name);
	return_value_t*	param_c		= *variables_map_get(param->vmap_const,
						&(token_t){.start="C", .length=1});

	f64	aspect_ratio	= (f64)(PLOT_COLS - PLOT_PADDING_X) / (f64)(PLOT_ROWS - PLOT_PADDING_Y);
	f64	re_min		= center_re - bound;
	f64	re_max		= center_re + bound;
	f64	im_min		= center_im - bound / aspect_ratio;
	f64	im_max		= center_im + bound / aspect_ratio;
	f64	re_range	= re_max - re_min;
	f64	im_range	= im_max - im_min;

	dots->x_min = re_min;
	dots->x_max = re_max;
	dots->y_min = im_min;
	dots->y_max = im_max;

	f64	z_min = INFINITY;
	f64	z_max = -INFINITY;

	u64	idx = 0;

	for (u64 row = 0; row < PLOT_ROWS - PLOT_PADDING_Y; row++)
	{
		f64	im = im_max - (im_range * row) / (PLOT_ROWS - PLOT_PADDING_Y - 1);

		for (u64 col = 0; col < PLOT_COLS - PLOT_PADDING_X; col++)
		{
			f64		re = re_min + (re_range * col) / (PLOT_COLS - PLOT_PADDING_X - 1);

			param_c->c	= re + im * I;
			param_var->c	= opts & PLOT_MANDELBROT ? 0
					: opts & PLOT_JULIA ? re + im * I
					: fabs(re) + fabs(im) * I;

			return_value_t*	ret;
			u32		cnt = 0;
			// TODO: thread this.
			do
			{
				ret = evaluate(y, param);

				if (ret->type == RET_ERR)
				{
					dots->err	= 1;
					dots->err_code	= ret->err_code;
					return dots;
				}

				param_var->c	= opts & PLOT_BURNING_SHIP
						? fabs(creal(ret->c)) + fabs(cimag(ret->c)) * I
						: ret->c;

				cnt++;
			} while (cabs(ret->c) < threshold && cnt < range);

			dots->x[idx] = re;
			dots->y[idx] = im;
			dots->z[idx] = cnt;
		
			if (dots->z[idx] < z_min) z_min = dots->z[idx];
			if (dots->z[idx] > z_max) z_max = dots->z[idx];

			idx++;
		}
	}

	dots->z_min	= z_min;
	dots->z_max	= z_max;
	param_c->c	= 0;

	return dots;
}

static dots_t*
plot_get_dots
(
	evaluate_param_t* param,
	token_t*	x_name,
	f64 		x_start, f64 x_inc,
	ast_node_t*	y,
	u64		range,
	plot_opt_t	opts
)
{
	dots_t*	dots = ARENA_PUSH_STRUCT(param->arena_tmp, dots_t);

	dots->err= 0;
	dots->x	= ARENA_PUSH_ARRAY(param->arena_tmp, f64, range);
	dots->y = ARENA_PUSH_ARRAY(param->arena_tmp, f64, range);
	
	variables_map_put(param->vmap_tmp, x_name, &(return_value_t)
		{.c	= x_start,
		.type	= RET_COMPLEX,
		.oom	= OOM_NONE});

	return_value_t*	ret = evaluate(y, param);

	if (ret->type == RET_ERR)
	{
		dots->err	= 1;
		dots->err_code	= ret->err_code;
		return dots;
	}
	
	return_value_t*	x		= *variables_map_get(param->vmap_tmp, x_name);
	f64		x_current	= x_start + x_inc;
	

	if (opts & PLOT_COMPLEX)
	{
		dots->x[0] = creal(ret->c);
		dots->y[0] = cimag(ret->c);
		
		f64	x_min = dots->x[0];
		f64	x_max = dots->x[0];
		f64	y_min = dots->y[0];
		f64	y_max = dots->y[0];
		
		for (u64 i = 1; i < range; i++, x_current += x_inc)
		{
			x->f		= x_current;
			ret		= evaluate(y, param);
			dots->x[i]	= creal(ret->c);
			dots->y[i]	= cimag(ret->c);
			x_min		= x_min < dots->x[i] ? x_min : dots->x[i];
			x_max		= x_max > dots->x[i] ? x_max : dots->x[i];
			y_min		= y_min < dots->y[i] ? y_min : dots->y[i];
			y_max		= y_max > dots->y[i] ? y_max : dots->y[i];
		}
		
		dots->x_min = x_min;
		dots->x_max = x_max;
		dots->y_min = y_min;
		dots->y_max = y_max;
		
		return dots;
	}

	dots->x[0] = x_start;
	dots->y[0] = ret->f;
	
	f64	y_min = ret->f;
	f64	y_max = ret->f;
	
	for (u64 i = 1; i < range; i++, x_current += x_inc)
	{
		x->f		= x_current;
		ret		= evaluate(y, param);
		dots->x[i]	= x_current;
		dots->y[i]	= ret->f;
		y_min		= y_min < ret->f ? y_min : ret->f;
		y_max		= y_max > ret->f ? y_max : ret->f;
	}
	
	dots->y_min = y_min;
	dots->y_max = y_max;

	return dots;
}

static char*
plot_get_buffer
(
	arena_t*	arena,
	dots_t*		dots,
	u64		opts,
	u64		range,
	f64		x_start, f64 x_end
)
{
	(void)opts;
	f64	x_range =  x_end - x_start;
	f64	y_range	= dots->y_max - dots->y_min;
	u64	rbufsize= (PLOT_COLS + 1) * (PLOT_ROWS + 2) + 1;
	char*	rbuffer	= ARENA_PUSH_ARRAY(arena, char, rbufsize);
	char	brush	= '*';
	
	memset(rbuffer, ' ', rbufsize);
	rbuffer[rbufsize - 1] = '\0';
	
	char	label[64] = {0};
	u64	offset; 
	
	sprintf(label, ">%*s|", 8, "");

	u64	label_len = strlen(label);

	for (u64 row = 0; row < PLOT_ROWS - 1; row++)
	{
		offset = row * (PLOT_COLS + 1);
		memcpy(rbuffer + offset, label, label_len);
		rbuffer[offset + PLOT_COLS] = '\n';
	}

	rbuffer[(PLOT_COLS + 1) + offset + PLOT_COLS] = '\n';
	rbuffer[(PLOT_COLS + 1)*2 + offset + PLOT_COLS] = '\n';
	
	char	label1[64] = {0};
	char	label2[64] = {0};
	
	sprintf(label, "> %-7.3g|", dots->y_max);
	offset = 0;
	memcpy(rbuffer, label, label_len);

	sprintf(label1, "> %-7.3g|", y_range / 2);
	offset = PLOT_ROWS / 2 * (PLOT_COLS + 1);
	memcpy(rbuffer + offset, label1, label_len);

	sprintf(label2, "> %-7.3g|", dots->y_min);
	offset = (PLOT_ROWS - 1) * (PLOT_COLS + 1);
	memcpy(rbuffer + offset, label2, label_len);

	offset += PLOT_COLS + 1;

	for (u64 col = PLOT_PADDING_X; col < PLOT_COLS; col++)
		rbuffer[offset + col] = '-';
	
	offset += PLOT_COLS + 1;

	char	x_label_start[32];
	char	x_label_end[32];
	char	x_label_mid[32];
	u64	end_pos = PLOT_COLS - strlen(x_label_end);
	f64	x_mid = (x_start + x_end) / 2.0;
	u64	mid_pos = PLOT_PADDING_X + (PLOT_COLS - PLOT_PADDING_X) / 2 - strlen(x_label_mid) / 2;

	sprintf(x_label_start, "%.2g", x_start);
	memcpy(rbuffer + offset + label_len, x_label_start, strlen(x_label_start));

	sprintf(x_label_end, "%.2g", x_end);
	memcpy(rbuffer + offset + end_pos, x_label_end, strlen(x_label_end));
	
	sprintf(x_label_mid, "%.2g", x_mid);
	memcpy(rbuffer + offset + mid_pos, x_label_mid, strlen(x_label_mid));

	// Note: default if y_range == 0
	u64	row = PLOT_ROWS / 2;
	
	for (u64 i = 0; i < range; i++)
	{
		if (y_range > 0.0)
			row = (u64)((dots->y_max - dots->y[i]) / y_range * (PLOT_ROWS - 1));
		
		u64 col = PLOT_PADDING_X + (u64)((dots->x[i] - x_start)
				/ x_range * (PLOT_COLS - PLOT_PADDING_X - 1));
		
		if (row >= PLOT_ROWS) row = PLOT_ROWS - 1;
		if (col >= PLOT_COLS) col = PLOT_COLS - 1;
		
		u64	index = row * (PLOT_COLS + 1) + col;

		rbuffer[index] = brush;
	}
	
	return rbuffer;
}

static void
get_ansi_color(f64 normalized, char* color_code, u64 max_len)
{
	u8	r, g, b;
	f64	fade_in = 0.1;   // 0.0-0.1: black to blue
	f64	fade_out = 0.9;  // 0.9-1.0: red to full red
	
	if (normalized < 0.0) normalized = 0.0;
	if (normalized > 1.0) normalized = 1.0;
	
	if (normalized < fade_in)
	{
		f64	t = normalized / fade_in;

		r = 0;
		g = 0;
		b = (u8)(t * 255.0 + 0.5);
	}
	else if (normalized > fade_out)
	{
		f64	t = (normalized - fade_out) / (1.0 - fade_out);

		r = 255;
		g = (u8)((1.0 - t) * 255.0 + 0.5);  // Fade out green
		b = 0;
	}
	else
	{
		f64	rainbow_pos = (normalized - fade_in) / (fade_out - fade_in);
		f64	hue = (1.0 - rainbow_pos) * 240.0;
		
		f64	h = hue / 60.0;
		s32	i = (s32)h;
		f64	f = h - i;
		f64	q = 1 - f;
		
		f64	rf, gf, bf;

		switch (i % 6)
		{
		case 0: rf = 1; gf = f; bf = 0; break;
		case 1: rf = q; gf = 1; bf = 0; break;
		case 2: rf = 0; gf = 1; bf = f; break;
		case 3: rf = 0; gf = q; bf = 1; break;
		case 4: rf = f; gf = 0; bf = 1; break;
		case 5: rf = 1; gf = 0; bf = q; break;
		default: rf = 0; gf = 0; bf = 0; break;
		}
		
		r = (u8)(rf * 255.0 + 0.5);
		g = (u8)(gf * 255.0 + 0.5);
		b = (u8)(bf * 255.0 + 0.5);
	}
	
	snprintf(color_code, max_len, "\033[38;2;%d;%d;%dm", r, g, b);
}

static char*
plot_get_buffer_fract
(arena_t* arena, dots_t* dots)
{
	f64	z_range		= dots->z_max - dots->z_min;
	char*	rbuffer		= ARENA_PUSH_ARRAY(arena, char, PLOT_FRAC_BUF_SIZE);
	u64	write_pos	= 0;
	u64	idx		= 0;
	char	y_max_str[32];
	char	y_min_str[32];

	printf("> range :: %g\n", z_range);
	memset(rbuffer, ' ', PLOT_FRAC_BUF_SIZE);
	sprintf(y_max_str, "%.2g", dots->y_max);
	sprintf(y_min_str, "%.2g", dots->y_min);

	// Add Y-axis label
	for (u64 row = 0; row < PLOT_ROWS; row++)
	{
		if (row == 0)
			write_pos += sprintf(rbuffer + write_pos, "> %-8.5g|", dots->y_max);
		else if (row == PLOT_ROWS / 2)
		{
			f64 y_mid = (dots->y_max + dots->y_min) / 2.0;
			write_pos += sprintf(rbuffer + write_pos, "> %-8.5g|", y_mid);
		}
		else if (row == PLOT_ROWS - 1)
			write_pos += sprintf(rbuffer + write_pos, "> %-8.5g|", dots->y_min);
		else
			write_pos += sprintf(rbuffer + write_pos, ">%-9.9s|", "");

		if (row >= PLOT_ROWS - PLOT_PADDING_Y)
		{
			rbuffer[write_pos++] = '\n';
			continue;
		}

		for (u64 col = 0; col < PLOT_COLS - PLOT_PADDING_X; col++)
		{
			char	color_code[32];
			if (z_range > EPS)
			{
				f64	normalized = (dots->z[idx] - dots->z_min) / z_range;
				get_ansi_color(normalized, color_code, sizeof(color_code));
			}
			else
				memcpy(color_code, "\033[38;2;0;0;0m", 14);

			write_pos += sprintf(rbuffer + write_pos, "%s█\033[0m", color_code);

			idx++;
		}
		
		rbuffer[write_pos++] = '\n';
	}

	// Add X-axis
	write_pos += sprintf(rbuffer + write_pos, "%*s", 11, "");
	for (u64 col = 0; col < PLOT_COLS - PLOT_PADDING_X; col++)
		rbuffer[write_pos++] = '-';
	rbuffer[write_pos++] = '\n';

	// Add X-axis labels
	char	x_label_start[32];
	char	x_label_end[32];

	sprintf(x_label_start, "%.2g", dots->x_min);
	sprintf(x_label_end, "%.2g", dots->x_max);

	write_pos += sprintf(rbuffer + write_pos, "%*s", 11, "");
	write_pos += sprintf(rbuffer + write_pos, "%s", x_label_start);

	u64	available_space =
		(PLOT_COLS - PLOT_PADDING_X) - strlen(x_label_start) - strlen(x_label_end);

	if (available_space > 10)
	{
		f64	x_mid = (dots->x_min + dots->x_max) / 2.0;
		char	x_label_mid[32];
		sprintf(x_label_mid, "%.2g", x_mid);

		u64	spaces_before_mid = available_space / 2 - strlen(x_label_mid) / 2;
		write_pos += sprintf(rbuffer + write_pos, "%*s%s",
		                     (s32)spaces_before_mid, "", x_label_mid);

		u64	spaces_after_mid = available_space - spaces_before_mid - strlen(x_label_mid);
		write_pos += sprintf(rbuffer + write_pos, "%*s%s\n",
		                     (s32)spaces_after_mid, "", x_label_end);
	}
	else
	{
		write_pos += sprintf(rbuffer + write_pos, "%*s%s\n",
		                     (s32)available_space, "", x_label_end);
	}

	rbuffer[write_pos] = '\0';

	return rbuffer;
}


s32
plot
(
	evaluate_param_t*	param,
	token_t*		x_name,
	f64			x_start, f64 x_end, f64 x_inc,
	ast_node_t*		y,
	u64			opts
)
{
	f64	x_range	= x_end - x_start;
	u64	range	= (u64)(x_range / x_inc) + 1;
	f64	zoom	= x_start;
	f64	zoom_in	= opts & PLOT_ZOOM_IN ? pow(10.0, floor(log10(zoom))-1) : zoom;
	f64	zoom_center_re = 0.0;
	f64	zoom_center_im = 0.0;
	dots_t*	dots;
	char*	rbuffer;

	// TODO: improve zoom
	if (opts & PLOT_MANDELBROT)
	{
		zoom_center_re = -0.7269;
		zoom_center_im = 0.1889;
	}
	else if (opts & PLOT_BURNING_SHIP)
	{
		zoom_center_re = -1.75;
		zoom_center_im = -0.035;
	}

	printf("\033[2J\033[H\033[?25l");

	do
	{
		printf("\033[H");
		arena_temp_t	atmp = arena_temp_begin(param->arena_tmp);

		if (opts & (PLOT_JULIA | PLOT_MANDELBROT | PLOT_BURNING_SHIP))
		{
			// Julia/Mandelbrot/Burning Ship plot mode (x_end = range and x_inc = threshold)
			// If Mandelbrot/Burning Ship opt, one should add the C variable to the equation.
			dots	= plot_get_dots_fract(param, x_name, y,
					zoom, zoom_center_re, zoom_center_im, x_end, x_inc, opts);

			if (dots->err)
			{
				printf("\033[?25h");
				return dots->err_code;
			}

			range	= (PLOT_ROWS - PLOT_PADDING_Y) * (PLOT_COLS - PLOT_PADDING_X);
			rbuffer	= plot_get_buffer_fract(param->arena_tmp, dots);
			printf("> zoom  :: %g\n", zoom);
		}
		else
		{
			// Normal plot mode
			dots	= plot_get_dots(param, x_name, x_start, x_inc, y, range, opts);

			if (dots->err)
			{
				printf("\033[?25h");
				return dots->err_code;
			}

			x_start	= opts & PLOT_COMPLEX ? dots->x_min : x_start;
			x_end	= opts & PLOT_COMPLEX ? dots->x_max : x_end;

			rbuffer = plot_get_buffer(param->arena_tmp, dots, opts, range, x_start, x_end);
		}

		char	expr[200];
		s32	idx = 0;

		parser_expr_to_str(y, expr, &idx);
		expr[idx] = 0;
		printf("> for y :: %s\n%s\n", expr, rbuffer);
		
		fflush(stdout);

		arena_temp_end(atmp);
		zoom -= zoom_in;
		zoom_in = pow(10.0, floor(log10(zoom))-1);
	} while (!interrupted && zoom > EPS);

	printf("\033[?25h");
	return -1;
}

#ifdef TESTER

#include <assert.h>

void
test_plot_get_dots(void)
{
	f64			x[]		= { 0., 1., 2., 3., 4., 5., 6., 7., 8., 9. };
	f64			y[]		= { 1., 2., 3., 4., 5., 6., 7., 8., 9., 10. };
	char*			expr		= "x+1";
	arena_t*		arena_tmp	= ARENA_ALLOC();
	arena_t*		arena_glb	= ARENA_ALLOC();
	variables_map*		vmap_tmp	= variables_map_new(arena_tmp, 100);
	variables_map*		vmap_glb	= variables_map_new(arena_glb, 1000);
	lexer_t			lexer;
	ast_node_t*		node;
	dots_t*			dots;
	evaluate_param_t	param	=
	{
		.arena_tmp	= arena_tmp,
		.arena_glb	= arena_glb,
		.vmap_tmp	= vmap_tmp,
		.vmap_glb	= vmap_glb,
		.vmap_const	= vmap_glb,
		.smap		= 0
	};

	
	lexer_init(&lexer, expr, 3);
	node	= parser_parse_expression(arena_tmp, &lexer, 0);
	dots	= plot_get_dots(&param, &(token_t){.start = "x", .length = 1}, 0, 1, node, 10, 0);

	for (u32 i = 0; i < 10; i++)
	{
		assert(dots->x[i] == x[i]);
		assert(dots->y[i] == y[i]);
	}
	
	arena_release(arena_tmp);
	arena_release(arena_glb);
}

void
test_plot_get_dots_fract(void)
{
	char*			expr		= "z*z+C";
	arena_t*		arena_tmp	= ARENA_ALLOC();
	arena_t*		arena_glb	= ARENA_ALLOC();
	arena_t*		arena_const	= ARENA_ALLOC();
	variables_map*		vmap_tmp	= variables_map_new(arena_tmp, 100);
	variables_map*		vmap_glb	= variables_map_new(arena_glb, 1000);
	variables_map*		vmap_const	= variables_map_new(arena_const, 100);
	lexer_t			lexer;
	ast_node_t*		node;
	dots_t*			dots;
	u64			bufsz	= (PLOT_ROWS - PLOT_PADDING_Y) * (PLOT_COLS - PLOT_PADDING_X);
	evaluate_param_t	param	=
	{
		.arena_tmp	= arena_tmp,
		.arena_glb	= arena_glb,
		.vmap_tmp	= vmap_tmp,
		.vmap_glb	= vmap_glb,
		.vmap_const	= vmap_const,
		.smap		= 0
	};

	evaluator_init_const_map(vmap_const);
	lexer_init(&lexer, expr, 5);
	node	= parser_parse_expression(arena_tmp, &lexer, 0);
	dots	= plot_get_dots_fract(
			&param, &(token_t){.start = "z", .length = 1},
			node, 2, 0, 0, 100, 2, PLOT_MANDELBROT);

	f64	z[] = { 1., 1., 1., 1., 2., 2., 2., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 4., 4., 4., 4., 4., 5., 5., 6., 6., 6., 7., 9., 14., 100., 100., 100., 100., 58., 12., 7., 6., 5., 5., 5., 5., 4., 4., 4., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 1., 1., 1., 1., 1., 2., 2., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 4., 4., 4., 4., 5., 5., 6., 6., 6., 6., 7., 8., 9., 31., 100., 100., 100., 100., 31., 11., 8., 7., 6., 6., 5., 5., 5., 4., 4., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 1., 1., 1., 1., 2., 2., 2., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 4., 4., 4., 4., 5., 5., 7., 7., 8., 7., 7., 8., 9., 10., 15., 100., 100., 100., 100., 44., 11., 9., 8., 7., 6., 6., 6., 7., 6., 4., 4., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 1., 1., 1., 2., 2., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 4., 4., 5., 5., 5., 6., 7., 14., 26., 14., 9., 10., 29., 13., 44., 17., 23., 100., 100., 36., 18., 14., 30., 25., 20., 8., 7., 7., 9., 11., 5., 4., 4., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 1., 1., 1., 2., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 4., 4., 5., 5., 5., 5., 6., 6., 7., 23., 20., 25., 22., 13., 93., 34., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 20., 10., 12., 14., 12., 11., 6., 5., 4., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 1., 1., 2., 2., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 5., 5., 5., 5., 5., 5., 6., 6., 6., 8., 9., 25., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 16., 59., 100., 100., 18., 6., 5., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 1., 2., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 5., 5., 5., 5., 5., 5., 5., 6., 6., 6., 7., 8., 10., 15., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 57., 9., 6., 5., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 1., 2., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 5., 5., 5., 5., 5., 5., 5., 5., 6., 6., 6., 7., 17., 47., 14., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 24., 12., 8., 6., 5., 5., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 1., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 5., 10., 6., 6., 6., 6., 6., 6., 6., 6., 6., 7., 7., 8., 12., 48., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 36., 8., 7., 6., 5., 4., 4., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 2., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 4., 5., 5., 6., 8., 9., 7., 7., 7., 8., 8., 7., 7., 7., 7., 8., 8., 13., 26., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 34., 18., 10., 7., 5., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 2., 3., 3., 3., 3., 4., 4., 4., 4., 4., 4., 5., 5., 5., 6., 10., 12., 9., 11., 9., 11., 12., 9., 8., 8., 8., 9., 11., 48., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 45., 8., 5., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 3., 3., 3., 4., 4., 4., 4., 4., 4., 4., 5., 5., 5., 6., 7., 8., 11., 41., 19., 13., 13., 23., 13., 17., 10., 9., 10., 12., 77., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 18., 9., 6., 5., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 3., 3., 4., 4., 4., 4., 4., 4., 4., 5., 5., 5., 5., 6., 7., 8., 9., 18., 100., 25., 100., 71., 57., 100., 24., 12., 11., 15., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 11., 6., 5., 4., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 3., 4., 4., 4., 4., 4., 4., 4., 5., 5., 5., 5., 6., 7., 7., 10., 11., 17., 100., 100., 100., 100., 100., 100., 100., 73., 14., 38., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 30., 6., 5., 4., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 4., 4., 4., 4., 4., 4., 4., 5., 5., 5., 5., 6., 9., 8., 9., 11., 82., 100., 100., 100., 100., 100., 100., 100., 100., 100., 22., 32., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 22., 8., 6., 5., 4., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 4., 4., 4., 4., 4., 4., 5., 6., 6., 6., 7., 7., 10., 11., 30., 13., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 13., 7., 5., 5., 4., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 5., 5., 5., 5., 6., 7., 6., 6., 6., 7., 7., 9., 11., 23., 28., 25., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 55., 8., 6., 5., 5., 4., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 5., 7., 6., 7., 10., 9., 8., 9., 9., 9., 11., 13., 16., 35., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 9., 7., 6., 5., 5., 4., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 5., 7., 6., 7., 10., 9., 8., 9., 9., 9., 11., 13., 16., 35., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 9., 7., 6., 5., 5., 4., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 5., 5., 5., 5., 6., 7., 6., 6., 6., 7., 7., 9., 11., 23., 28., 25., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 55., 8., 6., 5., 5., 4., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 4., 4., 4., 4., 4., 4., 5., 6., 6., 6., 7., 7., 10., 11., 30., 13., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 13., 7., 5., 5., 4., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 4., 4., 4., 4., 4., 4., 4., 5., 5., 5., 5., 6., 9., 8., 9., 11., 82., 100., 100., 100., 100., 100., 100., 100., 100., 100., 22., 32., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 22., 8., 6., 5., 4., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 3., 4., 4., 4., 4., 4., 4., 4., 5., 5., 5., 5., 6., 7., 7., 10., 11., 17., 100., 100., 100., 100., 100., 100., 100., 73., 14., 38., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 30., 6., 5., 4., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 3., 3., 4., 4., 4., 4., 4., 4., 4., 5., 5., 5., 5., 6., 7., 8., 9., 18., 100., 25., 100., 71., 57., 100., 24., 12., 11., 15., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 11., 6., 5., 4., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 3., 3., 3., 4., 4., 4., 4., 4., 4., 4., 5., 5., 5., 6., 7., 8., 11., 41., 19., 13., 13., 23., 13., 17., 10., 9., 10., 12., 77., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 18., 9., 6., 5., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 2., 3., 3., 3., 3., 4., 4., 4., 4., 4., 4., 5., 5., 5., 6., 10., 12., 9., 11., 9., 11., 12., 9., 8., 8., 8., 9., 11., 48., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 45., 8., 5., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 2., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 4., 5., 5., 6., 8., 9., 7., 7., 7., 8., 8., 7., 7., 7., 7., 8., 8., 13., 26., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 34., 18., 10., 7., 5., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 5., 10., 6., 6., 6., 6., 6., 6., 6., 6., 6., 7., 7., 8., 12., 48., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 36., 8., 7., 6., 5., 4., 4., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 1., 2., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 5., 5., 5., 5., 5., 5., 5., 5., 6., 6., 6., 7., 17., 47., 14., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 24., 12., 8., 6., 5., 5., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 1., 2., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 5., 5., 5., 5., 5., 5., 5., 6., 6., 6., 7., 8., 10., 15., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 57., 9., 6., 5., 4., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 1., 2., 2., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 5., 5., 5., 5., 5., 5., 6., 6., 6., 8., 9., 25., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 16., 59., 100., 100., 18., 6., 5., 4., 3., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 1., 1., 2., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 4., 4., 5., 5., 5., 5., 6., 6., 7., 23., 20., 25., 22., 13., 93., 34., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 20., 10., 12., 14., 12., 11., 6., 5., 4., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 1., 1., 1., 2., 2., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 4., 4., 5., 5., 5., 6., 7., 14., 26., 14., 9., 10., 29., 13., 44., 17., 23., 100., 100., 36., 18., 14., 30., 25., 20., 8., 7., 7., 9., 11., 5., 4., 4., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 1., 1., 1., 2., 2., 2., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 4., 4., 4., 4., 5., 5., 7., 7., 8., 7., 7., 8., 9., 10., 15., 100., 100., 100., 100., 44., 11., 9., 8., 7., 6., 6., 6., 7., 6., 4., 4., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 1., 1., 1., 1., 2., 2., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 4., 4., 4., 4., 5., 5., 6., 6., 6., 6., 7., 8., 9., 31., 100., 100., 100., 100., 31., 11., 8., 7., 6., 6., 5., 5., 5., 4., 4., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 1., 1., 1., 1., 1., 2., 2., 2., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 4., 4., 4., 4., 4., 5., 5., 6., 6., 6., 7., 9., 14., 100., 100., 100., 100., 58., 12., 7., 6., 5., 5., 5., 5., 4., 4., 4., 3., 3., 3., 3., 3., 3., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 2., 1., 1., 1., 1.};

	for (u32 i = 0; i < bufsz; i++)
		assert(dots->z[i] == z[i]);

	arena_release(arena_tmp);
	arena_release(arena_glb);
}

#endif
