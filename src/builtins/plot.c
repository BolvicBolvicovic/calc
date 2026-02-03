#include <plot.h>
#include <stdio.h>
#include <parser.h>

s32
plot
(
	arena_t*	arena,
	token_t*	x_name,
	f64		x_start, f64 x_end, f64 x_inc,
	ast_node_t*	y,
	plot_style_t	style
)
{
	f64	x_range	= x_end - x_start;
	u64	range	= (u64)(x_range / x_inc) + 1;
	dots_t	dots	=
	{
		.x = ARENA_PUSH_ARRAY(arena, f64, range),
		.y = ARENA_PUSH_ARRAY(arena, f64, range),
	};
	variables_map*		vmap	= variables_map_new(arena, 16);
	evaluate_param_t	param	=
	{
		.arena_tmp	= arena,
		.arena_glb	= arena,
		.vmap_tmp	= vmap,
		.vmap_glb	= vmap,
		.smap		= 0
	};
	variables_map_put(vmap, x_name, &(return_value_t){.f = x_start, .type = RET_FLOAT, .oom = OOM_NONE});
	
	return_value_t*	ret = evaluate(y, &param);
	if (ret->type == RET_ERR)
		return ret->err_code;
	dots.x[0] = x_start;
	dots.y[0] = ret->f;
	
	f64		y_min	= ret->f;
	f64		y_max	= ret->f;
	
	f64		x_current = x_start + x_inc;
	return_value_t*	x = *variables_map_get(vmap, x_name);
	
	for (u64 i = 1; i < range; i++, x_current += x_inc)
	{
		x->f		= x_current;
		ret		= evaluate(y, &param);
		dots.x[i]	= x_current;
		dots.y[i]	= ret->f;
		y_min		= y_min < ret->f ? y_min : ret->f;
		y_max		= y_max > ret->f ? y_max : ret->f;
	}
	
	char y_max_str[32], y_min_str[32];
	sprintf(y_max_str, "%.5f", y_max);
	sprintf(y_min_str, "%.5f", y_min);
	u64 y_label_width = strlen(y_max_str);
	if (strlen(y_min_str) > y_label_width)
		y_label_width = strlen(y_min_str);
	// Note: "> " prefix and "|" suffix
	y_label_width += 3;
	
	u64 y_label_spacing = 2;
	u64 num_y_labels = PLOT_ROWS / y_label_spacing;
	if (num_y_labels > 10) num_y_labels = 10;
	if (num_y_labels < 2) num_y_labels = 2;
	
	f64		y_range	= y_max - y_min;
	// Note: +2 for X-axis PLOT_ROWS
	u64		rbufsize= (PLOT_COLS + 1) * (PLOT_ROWS + 2) + 1;
	char*		rbuffer	= ARENA_PUSH_ARRAY(arena, char, rbufsize);
	char		brush	= style & PLOT_LINE ? '_'
				: style & PLOT_TRAIT? '|'
				: '*';
	
	memset(rbuffer, ' ', rbufsize);
	
	for (u64 row = 0; row < PLOT_ROWS + 2; row++)
		rbuffer[row * (PLOT_COLS + 1) + PLOT_COLS] = '\n';
	
	rbuffer[rbufsize - 1] = '\0';
	
	for (u64 label_idx = 0; label_idx < num_y_labels; label_idx++)
	{
		f64	y_value	= y_max - (y_range * label_idx) / (num_y_labels - 1);
		u64	row	= (label_idx * (PLOT_ROWS - 1)) / (num_y_labels - 1);
		char	label[64];
	
		sprintf(label, "> %*.5f|", (int)(y_label_width - 4), y_value);

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
			row = (u64)((y_max - dots.y[i]) / y_range * (PLOT_ROWS - 1));
		else
			// Center if all values are the same
			row = PLOT_ROWS / 2;
		
		u64 col = PLOT_PADDING_X + (u64)((dots.x[i] - x_start)
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

	char	expr[200];
	s32	idx = 0;

	parser_expr_to_str(y, expr, &idx);
	expr[idx] = 0;
	printf("> for y :: %s\n", expr);
	printf("%s\n", rbuffer);

	return -1;
}
