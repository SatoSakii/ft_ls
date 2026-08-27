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

const struct timespec	*entry_time(const t_file *f)
{
	if (g_time_type == TIME_ATIME)
		return (&ST_ATIM(f->st));
	if (g_time_type == TIME_CTIME)
		return (&ST_CTIM(f->st));
	return (&ST_MTIM(f->st));
}

// sort by date, recent first
static int	cmp_time(const void *a, const void *b)
{
	const struct timespec	*ta;
	const struct timespec	*tb;

	ta = entry_time(*(t_file *const *)a);
	tb = entry_time(*(t_file *const *)b);
	if (ta->tv_sec != tb->tv_sec)
	{
		if (tb->tv_sec < ta->tv_sec)
			return (-1);
		return (1);
	}
	if (ta->tv_nsec != tb->tv_nsec)
	{
		if (tb->tv_nsec < ta->tv_nsec)
			return (-1);
		return (1);
	}
	return (cmp_name(a, b));
}

// sort by size, biggest first
static int	cmp_size(const void *a, const void *b)
{
	const t_file	*fa;
	const t_file	*fb;

	fa = *(t_file *const *)a;
	fb = *(t_file *const *)b;
	if (fa->st.st_size != fb->st.st_size)
	{
		if (fb->st.st_size < fa->st.st_size)
			return (-1);
		return (1);
	}
	return (cmp_name(a, b));
}

// sort by extension (--sort=extension)
static int	cmp_extension(const void *a, const void *b)
{
	const char	*ea;
	const char	*eb;
	int			diff;

	ea = strrchr((*(t_file *const *)a)->name, '.');
	eb = strrchr((*(t_file *const *)b)->name, '.');
	if (!ea)
		ea = "";
	if (!eb)
		eb = "";
	diff = strcoll(ea, eb);
	if (diff == 0)
		return (cmp_name(a, b));
	return (diff);
}

// sort by display-width (--sort=width)
static int	cmp_width(const void *a, const void *b)
{
	size_t	wa;
	size_t	wb;

	wa = strlen((*(t_file *const *)a)->name);
	wb = strlen((*(t_file *const *)b)->name);
	if (wa != wb)
	{
		if (wa < wb)
			return (-1);
		return (1);
	}
	return (cmp_name(a, b));
}

static t_cmp	get_comparator(void)
{
	static const t_cmp	table[] = {
		[SORT_NAME] = cmp_name,
		[SORT_EXTENSION] = cmp_extension,
		[SORT_WIDTH] = cmp_width,
		[SORT_SIZE] = cmp_size,
		[SORT_VERSION] = cmp_name,
		[SORT_TIME] = cmp_time
	};

	if (g_sort_type == SORT_NONE)
		return (NULL);
	return (table[g_sort_type]);
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
	t_cmp	cmp;

	if (!init_sorted())
		return ;
	cmp = get_comparator();
	if (!cmp)
		return ;
	qsort(g_sorted, g_sorted_n_used, sizeof(t_file *), cmp);
	if (g_sort_reverse)
		reverse_sorted();
}
