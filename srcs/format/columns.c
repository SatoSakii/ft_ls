#include "ft_ls.h"

static size_t	*g_colw;
static size_t	g_colw_alloc;

// ls from gnu disable tab when there is color
static size_t	tab_size(void)
{
	if (g_print_with_color)
		return (0);
	return (TAB_SIZE);
}

void	free_columns(void)
{
	free(g_colw);
	g_colw = NULL;
	g_colw_alloc = 0;
}

static int	ensure_colw(size_t n)
{
	size_t	*tmp;

	if (n <= g_colw_alloc)
		return (1);
	tmp = realloc(g_colw, n * sizeof(size_t));
	if (!tmp)
		return (0);
	g_colw = tmp;
	g_colw_alloc = n;
	return (1);
}

static void	fill_widths(void)
{
	size_t	i;

	i = 0;
	while (i < g_sorted_n_used)
	{
		g_sorted[i]->width = entry_display_width(g_sorted[i]);
		i++;
	}
}

// fill by columns or by rows
static size_t	col_of(size_t i, size_t cols, size_t rows, int by_columns)
{
	if (by_columns)
		return (i / rows);
	return (i % cols);
}

// compute the width of each output column
static size_t	compute_col_widths(size_t cols, int by_columns)
{
	size_t	rows;
	size_t	i;
	size_t	idx;
	size_t	w;

	rows = (g_sorted_n_used + cols - 1) / cols;
	i = 0;
	while (i < cols)
		g_colw[i++] = MIN_COL_WIDTH;
	i = 0;
	while (i < g_sorted_n_used)
	{
		idx = col_of(i, cols, rows, by_columns);
		w = g_sorted[i]->width + 2 * (idx + 1 != cols);
		if (w > g_colw[idx])
			g_colw[idx] = w;
		i++;
	}
	w = 0;
	i = 0;
	while (i < cols)
		w += g_colw[i++];
	return (w);
}

// theory
static size_t	max_columns(void)
{
	size_t	max_idx;

	max_idx = g_line_length / MIN_COL_WIDTH;
	if (g_line_length % MIN_COL_WIDTH)
		max_idx++;
	if (max_idx > 0 && max_idx < g_sorted_n_used)
		return (max_idx);
	return (g_sorted_n_used);
}

// find the largest number of cols that fits in the terminal
static size_t	fitting_cols(int by_columns)
{
	size_t	max_cols;
	size_t	c;

	max_cols = max_columns();
	if (max_cols < 1)
		max_cols = 1;
	if (!ensure_colw(max_cols))
		return (0);
	c = max_cols;
	while (c > 1)
	{
		if (compute_col_widths(c, by_columns) < g_line_length)
			return (c);
		c--;
	}
	compute_col_widths(1, by_columns);
	return (1);
}

static void	indent_to(size_t from, size_t to)
{
	size_t	tab;

	tab = tab_size();
	while (from < to)
	{
		if (tab != 0 && to / tab > (from + 1) / tab)
		{
			putchar('\t');
			from += tab - from % tab;
		}
		else
		{
			putchar(' ');
			from++;
		}
	}
}

static void	print_one_line(void)
{
	size_t	i;

	i = 0;
	while (i < g_sorted_n_used)
	{
		if (i)
			printf("  ");
		print_one(g_sorted[i]);
		i++;
	}
	putchar('\n');
}

void	print_many_per_line(void)
{
	size_t	cols;
	size_t	rows;
	size_t	row;
	size_t	i;
	size_t	col;
	size_t	pos;

	if (g_sorted_n_used == 0)
		return ;
	if (g_line_length == 0)
		return (print_one_line());
	fill_widths();
	cols = fitting_cols(1);
	if (cols == 0)
		return ;
	rows = (g_sorted_n_used + cols - 1) / cols;
	row = 0;
	while (row < rows)
	{
		i = row;
		col = 0;
		pos = 0;
		while (1)
		{
			print_one(g_sorted[i]);
			if (i + rows >= g_sorted_n_used)
				break ;
			indent_to(pos + g_sorted[i]->width, pos + g_colw[col]);
			pos += g_colw[col];
			i += rows;
			col++;
		}
		putchar('\n');
		row++;
	}
}

void	print_horizontal(void)
{
	size_t	cols;
	size_t	i;
	size_t	col;
	size_t	pos;

	if (g_sorted_n_used == 0)
		return ;
	if (g_line_length == 0)
		return (print_one_line());
	fill_widths();
	cols = fitting_cols(0);
	if (cols == 0)
		return ;
	print_one(g_sorted[0]);
	pos = 0;
	i = 1;
	while (i < g_sorted_n_used)
	{
		col = i % cols;
		if (col == 0)
		{
			putchar('\n');
			pos = 0;
		}
		else
		{
			indent_to(pos + g_sorted[i - 1]->width, pos + g_colw[col - 1]);
			pos += g_colw[col - 1];
		}
		print_one(g_sorted[i]);
		i++;
	}
	putchar('\n');
}
