#include "ft_ls.h"

size_t	print_one(const t_file *f)
{
	char	c;

	print_prefix(f);
	print_colored(f, f->name, 0);
	c = file_indicator(f->st.st_mode);
	if (c)
		putchar(c);
	return (entry_display_width(f));
}

void	print_current_files(void)
{
	size_t	i;

	if (g_print_inode || g_print_block_size)
		compute_widths();
	if (g_format == FMT_LONG)
		return (print_long_files());
	if (g_format == FMT_MANY_PER_LINE)
		return (print_many_per_line());
	if (g_format == FMT_HORIZONTAL)
		return (print_horizontal());
	i = 0;
	while (i < g_sorted_n_used)
	{
		print_one(g_sorted[i++]);
		putchar('\n');
	}
}

static void	print_header(const char *name)
{
	static int	first = 1;

	if (!g_print_dir_name)
		return ;
	if (!first)
		putchar('\n');
	first = 0;
	printf("%s:\n", name);
}

void	print_dir(const char *name, int cmdline)
{
	DIR				*dir;
	struct dirent	*de;

	dir = opendir(name);
	if (!dir)
	{
		file_failure(cmdline, "cannot open directory", name);
		return ;
	}
	print_header(name);
	clear_files();
	errno = 0;
	de = readdir(dir);
	while (de)
	{
		gobble_file(de->d_name, de->d_type, name, 0);
		errno = 0;
		de = readdir(dir);
	}
	if (errno != 0)
		file_failure(0, "reading directory", name);
	closedir(dir);
	sort_files();
	if (g_format == FMT_LONG || g_print_block_size)
		print_total();
	print_current_files();
	if (g_recursive)
		queue_subdirs(name);
}
