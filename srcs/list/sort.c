#include "ft_ls.h"

static int	cmp_name(const void *a, const void *b)
{
	const t_file	*fa;
	const t_file	*fb;
	int				diff;

	fa = *(t_file *const *)a;
	fb = *(t_file *const *)b;
	diff = strcoll(fa->name, fb->name);
	if (diff == 0)
		diff = strcmp(fa->name, fb->name);
	return (diff);
}

// re-build g_sorted
// never use malloc per folder, only realloc if it needs to grow
static int	init_sorted(void)
{
	t_file	**tmp;
	size_t	i;

	if (g_cwd_n_used > g_sorted_alloc)
	{
		tmp = realloc(g_sorted, g_cwd_n_used * sizeof(t_file *));
		if (!tmp)
			return (0);
		g_sorted = tmp;
		g_sorted_alloc = g_cwd_n_used;
	}
	i = 0;
	while (i < g_cwd_n_used)
	{
		g_sorted[i] = &g_cwd_file[i];
		i++;
	}
	g_sorted_n_used = g_cwd_n_used;
	return (1);
}

// reverse for for -r flag
static void	reverse_sorted(void)
{
	size_t	i;
	size_t	j;
	t_file	*tmp;

	if (g_sorted_n_used == 0)
		return ;
	i = 0;
	j = g_sorted_n_used - 1;
	while (i < j)
	{
		tmp = g_sorted[i];
		g_sorted[i] = g_sorted[j];
		g_sorted[j] = tmp;
		i++;
		j--;
	}
}

void	sort_files(void)
{
	if (!init_sorted())
		return ;
	if (g_sort_type != SORT_NONE)
		qsort(g_sorted, g_sorted_n_used, sizeof(t_file *), cmp_name);
	if (g_sort_reverse)
		reverse_sorted();
}
