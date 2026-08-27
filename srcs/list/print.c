#include "ft_ls.h"

static void	print_current_files(void)
{
	size_t	i;

	i = 0;
	while (i < g_cwd_n_used)
	{
		fprintf(stdout, "%s\n", g_sorted[i]->name);
		i++;
	}
}

void	print_dir(const char *name)
{
	DIR				*dir;
	struct dirent	*de;

	dir = opendir(name);
	if (!dir)
	{
		fprintf(stderr, "ft_ls: cannot open directory '%s': %s\n",
			name, strerror(errno));
		g_exit_status = 1;
		return ;
	}
	clear_files();
	errno = 0;
	de = readdir(dir);
	while (de)
	{
		gobble_file(de->d_name, de->d_type, name);
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
