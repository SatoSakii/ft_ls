#include "ft_ls.h"

void	print_current_files(void)
{
	size_t	i;

	i = 0;
	while (i < g_cwd_n_used)
	{
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
		fprintf(stderr, "ft_ls: cannot open directory '%s': %s\n",
			name, strerror(errno));
		if (cmdline)
			g_exit_status = 2;
		else
			g_exit_status = 1;
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
	{
		fprintf(stderr, "ft_ls: reading directory '%s': %s\n",
			name, strerror(errno));
		g_exit_status = 1;
	}
	closedir(dir);
	sort_files();
	print_current_files();
}
