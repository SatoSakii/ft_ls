#include "ft_ls.h"

t_format			g_format = FMT_MANY_PER_LINE;
t_sort_type			g_sort_type = SORT_NAME;
t_time_type			g_time_type = TIME_MTIME;
t_indicator_style	g_indicator_style = IND_NONE;
t_ignore_mode		g_ignore_mode = IGNORE_DEFAULT;
t_color_when		g_color_when = COLOR_NEVER;

int					g_format_set = 0;
int					g_sort_set = 0;
int					g_explicit_time = 0;
int					g_sort_reverse = 0;
int					g_recursive = 0;
int					g_immediate_dirs = 0;
int					g_print_inode = 0;
int					g_print_block_size = 0;
int					g_human = 0;
int					g_numeric_ids = 0;
int					g_print_owner = 1;
int					g_print_group = 1;
int					g_print_with_color = 0;
int					g_is_tty = 0;
int					g_width_set = 0;
int					g_exit_status = 0;
size_t				g_line_length = 80;

t_pending			*g_pending_dirs = NULL;
