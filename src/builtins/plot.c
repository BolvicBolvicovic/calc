#include <plot.h>
#include <stdio.h>
#include <parser.h>
#include <math.h>

static dots_t*
plot_get_dots_surface
(
	evaluate_param_t* param,
	token_t*	x_name,
	ast_node_t*	y,
	f64		bound,
	u64		range,
	f64		threshold
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

	return_value_t*	param_var = *variables_map_get(param->vmap_tmp, x_name);

	f64	aspect_ratio = (f64)(PLOT_COLS - PLOT_PADDING_X) / (f64)(PLOT_ROWS - PLOT_PADDING_Y);

	f64	re_min = -bound;
	f64	re_max = bound;
	f64	im_min = -bound / aspect_ratio;
	f64	im_max = bound / aspect_ratio;
	f64	re_range = re_max - re_min;
	f64	im_range = im_max - im_min;

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
			param_var->c = re + im * I;

			return_value_t*	ret = evaluate(y, param);
			f64		cnt = 1.0;

			for (; cabs(ret->c) < threshold && cnt < range; cnt+=1)
			{
				if (ret->type == RET_ERR)
				{
					dots->err	= 1;
					dots->err_code	= ret->err_code;
					return dots;
				}

				param_var->c = ret->c;
				ret = evaluate(y, param);
			}

			dots->x[idx] = re;
			dots->y[idx] = im;
			dots->z[idx] = cnt;
		
			if (dots->z[idx] < z_min) z_min = dots->z[idx];
			if (dots->z[idx] > z_max) z_max = dots->z[idx];

			idx++;
		}
	}

	dots->z_min = z_min;
	dots->z_max = z_max;

	return dots;
}

static dots_t*
plot_get_dots_normal
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
		{.f	= x_start,
		.type	= RET_FLOAT,
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
	f64	x_range =  x_end - x_start;
	char	y_max_str[32];
	char	y_min_str[32];

	sprintf(y_max_str, "%.5g", dots->y_max);
	sprintf(y_min_str, "%.5g", dots->y_min);

	u64	y_label_width = strlen(y_max_str);

	if (strlen(y_min_str) > y_label_width)
		y_label_width = strlen(y_min_str);

	// Note: "> " prefix and "|" suffix
	y_label_width += 3;
	
	u64	y_label_spacing	= 2;
	u64	num_y_labels	= PLOT_ROWS / y_label_spacing;

	if (num_y_labels > 10) num_y_labels = 10;
	if (num_y_labels < 2) num_y_labels = 2;
	
	f64		y_range	= dots->y_max - dots->y_min;
	// Note: +2 for X-axis PLOT_ROWS
	u64		rbufsize= (PLOT_COLS + 1) * (PLOT_ROWS + 2) + 1;
	char*		rbuffer	= ARENA_PUSH_ARRAY(arena, char, rbufsize);
	char		brush	= opts & PLOT_LINE ? '_'
				: opts & PLOT_TRAIT? '|'
				: '*';
	
	memset(rbuffer, ' ', rbufsize);
	
	for (u64 row = 0; row < PLOT_ROWS + 2; row++)
		rbuffer[row * (PLOT_COLS + 1) + PLOT_COLS] = '\n';
	
	rbuffer[rbufsize - 1] = '\0';
	
	for (u64 label_idx = 0; label_idx < num_y_labels; label_idx++)
	{
		f64	y_value	= dots->y_max - (y_range * label_idx) / (num_y_labels - 1);
		u64	row	= (label_idx * (PLOT_ROWS - 1)) / (num_y_labels - 1);
		char	label[64];
	
		sprintf(label, "> %*.5g|", (s32)(y_label_width - 4), y_value);

		u64	offset		= row * (PLOT_COLS + 1);
		u64	label_len	= strlen(label);

		if (label_len > y_label_width)
			label_len = y_label_width;

		memcpy(rbuffer + offset, label, label_len);
	}
	
	for (u64 row = 0; row < PLOT_ROWS; row++)
	{
		u64	offset = row * (PLOT_COLS + 1);

		if (rbuffer[offset] != '>')
		{
			memcpy(rbuffer + offset, ">        |", 
			       y_label_width < 10 ? y_label_width : 10);
		}
	}
	
	for (u64 i = 0; i < range; i++)
	{
		u64	row;
	
		if (y_range > 0.0)
			row = (u64)((dots->y_max - dots->y[i]) / y_range * (PLOT_ROWS - 1));
		else
			// Center if all values are the same
			row = PLOT_ROWS / 2;
		
		u64 col = PLOT_PADDING_X + (u64)((dots->x[i] - x_start)
				/ x_range * (PLOT_COLS - PLOT_PADDING_X - 1));
		
		if (row >= PLOT_ROWS) row = PLOT_ROWS - 1;
		if (col >= PLOT_COLS) col = PLOT_COLS - 1;
		
		u64	index = row * (PLOT_COLS + 1) + col;

		rbuffer[index] = brush;
	}
	
	u64	x_axis_row	= PLOT_ROWS;
	u64	x_axis_offset	= x_axis_row * (PLOT_COLS + 1);
	for (u64 col = PLOT_PADDING_X; col < PLOT_COLS; col++)
		rbuffer[x_axis_offset + col] = '-';
	
	u64	x_label_row	= PLOT_ROWS + 1;
	u64	x_label_offset	= x_label_row * (PLOT_COLS + 1);
	char	x_label_start[32];
	char	x_label_end[32];

	sprintf(x_label_start, "%.2g", x_start);
	memcpy(rbuffer + x_label_offset + PLOT_PADDING_X, x_label_start, strlen(x_label_start));
	sprintf(x_label_end, "%.2g", x_end);

	u64	end_pos = PLOT_COLS - strlen(x_label_end);

	if (end_pos > PLOT_PADDING_X)
		memcpy(rbuffer + x_label_offset + end_pos, x_label_end, strlen(x_label_end));
	
	if (PLOT_COLS > PLOT_PADDING_X + 40)
	{
		f64	x_mid = (x_start + x_end) / 2.0;
		char	x_label_mid[32];

		sprintf(x_label_mid, "%.2g", x_mid);

		u64	mid_pos = PLOT_PADDING_X + (PLOT_COLS - PLOT_PADDING_X) / 2 - strlen(x_label_mid) / 2;

		memcpy(rbuffer + x_label_offset + mid_pos, x_label_mid, strlen(x_label_mid));
	}	

	return rbuffer;
}

static void
get_ansi_color_rainbow(f64 normalized, char* color_code, u64 max_len)
{
	u8 r, g, b;
	f64 hue = normalized * 300.0;  // 0-300 degrees (red to purple)

	// Simple HSV to RGB conversion (S=1, V=1)
	f64 h = hue / 60.0;
	s32 i = (s32)h;
	f64 f = h - i;

	switch (i % 6)
	{
		case 0: r = 255; g = (u8)(f * 255); b = 0; break;
		case 1: r = (u8)((1-f) * 255); g = 255; b = 0; break;
		case 2: r = 0; g = 255; b = (u8)(f * 255); break;
		case 3: r = 0; g = (u8)((1-f) * 255); b = 255; break;
		case 4: r = (u8)(f * 255); g = 0; b = 255; break;
		case 5: r = 255; g = 0; b = (u8)((1-f) * 255); break;
		default: r = 0; g = 0; b = 0; break;
	}

	snprintf(color_code, max_len, "\033[38;2;%d;%d;%dm", r, g, b);
}

static char*
plot_get_buffer_surface
(arena_t* arena, dots_t* dots)
{
	f64	z_range = dots->z_max - dots->z_min;

	// Estimate buffer size with ANSI codes
	// Each colored cell needs: color code (~20 chars) + character + reset (~4 chars)
	// Note: This is a rough estimate, we'll use a dynamic approach with sprintf
	u64	base_size = (PLOT_COLS + 1) * (PLOT_ROWS + 2) + 1;
	u64	rbufsize = base_size + (PLOT_ROWS * PLOT_COLS * 30);  // Extra space for ANSI codes
	char*	rbuffer	= ARENA_PUSH_ARRAY(arena, char, rbufsize);

	memset(rbuffer, 0, rbufsize);

	u64	write_pos = 0;

	// Add Y-axis labels (imaginary axis)
	char	y_max_str[32];
	char	y_min_str[32];

	sprintf(y_max_str, "%.2g", dots->y_max);
	sprintf(y_min_str, "%.2g", dots->y_min);

	u64	y_label_width = strlen(y_max_str);
	if (strlen(y_min_str) > y_label_width)
		y_label_width = strlen(y_min_str);
	y_label_width += 3;  // "> " prefix and "|" suffix

	u64	idx = 0;

	for (u64 row = 0; row < PLOT_ROWS; row++)
	{
		// Add Y-axis label
		if (row == 0)
		{
			write_pos += sprintf(rbuffer + write_pos, "> %*.2g|",
			                     (s32)(y_label_width - 4), dots->y_max);
		}
		else if (row == PLOT_ROWS / 2)
		{
			f64 y_mid = (dots->y_max + dots->y_min) / 2.0;
			write_pos += sprintf(rbuffer + write_pos, "> %*.2g|",
			                     (s32)(y_label_width - 4), y_mid);
		}
		else if (row == PLOT_ROWS - 1)
		{
			write_pos += sprintf(rbuffer + write_pos, "> %*.2g|",
			                     (s32)(y_label_width - 4), dots->y_min);
		}
		else
		{
			write_pos += sprintf(rbuffer + write_pos, ">%*s|",
			                     (s32)(y_label_width - 3), "");
		}

		// Plot the row with colors
		if (row < PLOT_ROWS - PLOT_PADDING_Y)
		{
			for (u64 col = 0; col < PLOT_COLS - PLOT_PADDING_X; col++)
			{
				// Normalize z-value
				f64	normalized;

				if (z_range > 0.0)
					normalized = (dots->z[idx] - dots->z_min) / z_range;
				else
					normalized = 0.5;

				// Get ANSI color code
				char	color_code[32];
				get_ansi_color_rainbow(normalized, color_code, sizeof(color_code));

				// Write colored character (using a block character for better visibility)
				write_pos += sprintf(rbuffer + write_pos, "%s█\033[0m", color_code);

				idx++;
			}
		}
		else
		{
			// Padding rows at bottom
			for (u64 col = 0; col < PLOT_COLS - PLOT_PADDING_X; col++)
				rbuffer[write_pos++] = ' ';
		}

		rbuffer[write_pos++] = '\n';
	}

	// Add X-axis
	write_pos += sprintf(rbuffer + write_pos, "%*s", (s32)y_label_width, "");
	for (u64 col = 0; col < PLOT_COLS - PLOT_PADDING_X; col++)
		rbuffer[write_pos++] = '-';
	rbuffer[write_pos++] = '\n';

	// Add X-axis labels
	char	x_label_start[32];
	char	x_label_end[32];

	sprintf(x_label_start, "%.2g", dots->x_min);
	sprintf(x_label_end, "%.2g", dots->x_max);

	write_pos += sprintf(rbuffer + write_pos, "%*s", (s32)y_label_width, "");
	write_pos += sprintf(rbuffer + write_pos, "%s", x_label_start);

	u64	available_space = (PLOT_COLS - PLOT_PADDING_X) - strlen(x_label_start) - strlen(x_label_end);

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
	dots_t*	dots;
	char*	rbuffer;

	// TODO: rename SURF in JULIA/MANDELBROT/ETC...
	if (opts & PLOT_SURF)
	{
		// Surface plot mode (x_start = bound, x_end = range and x_inc = threshold)
		dots	= plot_get_dots_surface(param, x_name, y, x_start, x_end, x_inc);

		if (dots->err)
			return dots->err_code;

		range	= (PLOT_ROWS - PLOT_PADDING_Y) * (PLOT_COLS - PLOT_PADDING_X);
		rbuffer	= plot_get_buffer_surface(param->arena_tmp, dots);
	}
	else
	{
		// Normal plot mode
		dots	= plot_get_dots_normal(param, x_name, x_start, x_inc, y, range, opts);

		if (dots->err)
			return dots->err_code;

		x_start	= opts & PLOT_COMPLEX ? dots->x_min : x_start;
		x_end	= opts & PLOT_COMPLEX ? dots->x_max : x_end;

		rbuffer = plot_get_buffer(param->arena_tmp, dots, opts, range, x_start, x_end);
	}

	char	expr[200];
	s32	idx = 0;

	parser_expr_to_str(y, expr, &idx);
	expr[idx] = 0;
	printf("> for y :: %s\n", expr);
	printf("%s\n", rbuffer);

	return -1;
}

#ifdef TESTER



#endif
