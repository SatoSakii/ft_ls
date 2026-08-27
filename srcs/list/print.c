#include "ft_ls.h"

void	print_current_files(void)
{
	size_t	i;

	if (g_format == FMT_LONG)
	{
		print_long_files();
		return ;
	}
	if (g_print_inode || g_print_block_size)
		compute_widths();
	i = 0;
	while (i < g_sorted_n_used)
	{
		print_prefix(g_sorted[i]);
		fprintf(stdout, "%s\n", g_sorted[i]->name);
		i++;
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
	fprintf(stdout, "%s:\n", name);
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
