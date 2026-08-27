#include "ft_ls.h"

static t_widths	g_w;

static void	update_max(size_t *max, size_t w)
{
	if (w >= *max)
		*max = w;
}

static int	is_device(const t_file *f)
{
	return (S_ISCHR(f->st.st_mode) || S_ISBLK(f->st.st_mode));
}

// update major/minor column widths for device files
static void	update_device_widths(const t_file *f)
{
	update_max(&g_w.major, num_width(major(f->st.st_rdev)));
	update_max(&g_w.minor, num_width(minor(f->st.st_rdev)));
}

// compute the maximum width of each column before printing
// device files need a second pass because their major/minor widths
// must be known before the size column can be calculated
void	compute_widths(void)
{
	size_t			i;
	const t_file	*f;

	memset(&g_w, 0, sizeof(g_w));
	i = 0;
	while (i < g_sorted_n_used)
	{
		f = g_sorted[i++];
		update_max(&g_w.inode, num_width(f->st.st_ino));
		update_max(&g_w.blocks, strlen(blocks_field(f)));
		update_max(&g_w.nlink, num_width(f->st.st_nlink));
		update_max(&g_w.owner, strlen(owner_field(f)));
		update_max(&g_w.group, strlen(group_field(f)));
		if (is_device(f))
			update_device_widths(f);
		else
			update_max(&g_w.size, strlen(size_field(f, 0, 0)));
	}
	if (g_w.major)
		update_max(&g_w.size, g_w.major + 2 + g_w.minor);
	i = 0;
	while (i < g_sorted_n_used)
		update_max(&g_w.size, strlen(size_field(g_sorted[i++], g_w.major, g_w.minor)));
}

void	print_prefix(const t_file *f)
{
	if (g_print_inode)
		printf("%*llu ", (int)g_w.inode, (unsigned long long)f->st.st_ino);
	if (g_print_block_size)
		printf("%*s ", (int)g_w.blocks, blocks_field(f));
}

// return the number of characters needed to display an entry
size_t	entry_display_width(const t_file *f)
{
	size_t	w;

	w = strlen(f->name);
	if (g_print_inode)
		w += g_w.inode + 1;
	if (g_print_block_size)
		w += g_w.blocks + 1;
	if (file_indicator(f->st.st_mode))
		w += 1;
	return (w);
}

static void	print_name(const t_file *f)
{
	char	c;

	printf("%s", f->name);
	if (f->filetype == SYMLINK && f->linkname)
	{
		printf(" -> %s", f->linkname);
		c = file_indicator(f->linkmode);
	}
	else
		c = file_indicator(f->st.st_mode);
	if (c)
		putchar(c);
	putchar('\n');
}

static void	print_long_line(const t_file *f)
{
	char	mode[11];

	print_prefix(f);
	mode_string(f->st.st_mode, mode);
	printf("%s ", mode);
	printf("%*llu ", (int)g_w.nlink, (unsigned long long)f->st.st_nlink);
	if (g_print_owner)
		printf("%-*s ", (int)g_w.owner, owner_field(f));
	if (g_print_group)
		printf("%-*s ", (int)g_w.group, group_field(f));
	printf("%*s ", (int)g_w.size, size_field(f, g_w.major, g_w.minor));
	printf("%s ", format_time(entry_time(f)));
	print_name(f);
}

void	print_long_files(void)
{
	size_t	i;

	compute_widths();
	i = 0;
	while (i < g_sorted_n_used)
		print_long_line(g_sorted[i++]);
}

void	print_total(void)
{
	size_t				i;
	unsigned long long	total;

	total = 0;
	i = 0;
	while (i < g_sorted_n_used)
		total += (unsigned long long)g_sorted[i++]->st.st_blocks;
	if (g_human)
		printf("total %s\n", human_size(total * 512));
	else
		printf("total %llu\n", blocks_to_kib(total));
}