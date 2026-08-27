#include "ft_ls.h"

// add a directory to the front of the pending dir list
int	queue_directory(const char *name, int cmdline)
{
	t_pending	*p;

	p = malloc(sizeof(t_pending));
	if (!p)
		return (0);
	p->name = strdup(name);
	if (!p->name)
	{
		free(p);
		return (0);
	}
	p->realname = NULL;
	p->cmdline_arg = cmdline;
	p->next = g_pending_dirs;
	g_pending_dirs = p;
	return (1);
}

static t_pending	*pop_pending(void)
{
	t_pending	*p;

	p = g_pending_dirs;
	if (p)
		g_pending_dirs = p->next;
	return (p);
}

void	free_pending(void)
{
	t_pending	*p;

	p = pop_pending();
	while (p)
	{
		free(p->name);
		free(p->realname);
		free(p);
		p = pop_pending();
	}
}

static int	is_dir_entry(const t_file *f)
{
	return (f->filetype == DIRECTORY || f->filetype == ARG_DIRECTORY);
}

// move directories from g_sorted to the pending list
static size_t	extract_dirs_from_files(void)
{
	size_t	i;
	size_t	kept;

	i = g_sorted_n_used;
	while (i > 0)
	{
		i--;
		if (is_dir_entry(g_sorted[i]))
			queue_directory(g_sorted[i]->name, 1);
	}
	kept = 0;
	i = 0;
	while (i < g_sorted_n_used)
	{
		if (!is_dir_entry(g_sorted[i]))
			g_sorted[kept++] = g_sorted[i];
		i++;
	}
	g_sorted_n_used = kept;
	return (kept);
}

static void	gobble_operands(int argc, char **argv, int i)
{
	while (i < argc)
	{
		gobble_file(argv[i], DT_UNKNOWN, "", 1);
		i++;
	}
}

// decides if directory names should be printed as header
static void	set_print_dir_name(int n_files)
{
	g_print_dir_name = 1;
	if (n_files <= 1 && g_pending_dirs && !g_pending_dirs->next)
		g_print_dir_name = 0;
	if (g_recursive)
		g_print_dir_name = 1;
}

// process cmd-line operands
void	list_operands(int argc, char **argv, int i)
{
	int			n_files;
	size_t		n_nondirs;
	t_pending	*p;

	n_files = argc - i;
	if (n_files == 0)
		queue_directory(".", 1);
	else
		gobble_operands(argc, argv, i);
	n_nondirs = 0;
	if (g_cwd_n_used > 0)
	{
		sort_files();
		n_nondirs = g_sorted_n_used;
		if (!g_immediate_dirs)
			n_nondirs = extract_dirs_from_files();
	}
	set_print_dir_name(n_files);
	if (n_nondirs > 0)
	{
		print_current_files();
		if (g_pending_dirs)
			putchar('\n');
	}
	p = pop_pending();
	while (p)
	{
		print_dir(p->name, p->cmdline_arg);
		free(p->name);
		free(p->realname);
		free(p);
		p = pop_pending();
	}
}