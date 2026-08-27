#include "ft_ls.h"

static t_file	*new_entry(void)
{
	t_file	*tmp;
	size_t	n_alloc;

	if (g_cwd_n_used == g_cwd_n_alloc)
	{
		if (g_cwd_n_alloc == 0)
			n_alloc = 100;
		else
			n_alloc = g_cwd_n_alloc * 2;
		tmp = realloc(g_cwd_file, n_alloc * sizeof(t_file));
		if (!tmp)
			return (NULL);
		g_cwd_file = tmp;
		g_cwd_n_alloc = n_alloc;
	}
	memset(&g_cwd_file[g_cwd_n_used], 0, sizeof(t_file));
	return (&g_cwd_file[g_cwd_n_used++]);
}

static int	file_ignored(const char *name)
{
	if (g_ignore_mode == IGNORE_MINIMAL)
		return (0);
	if (g_ignore_mode == IGNORE_DOT_AND_DOTDOT)
		return (!strcmp(name, ".") || !strcmp(name, ".."));
	return (name[0] == '.');
}

static void	free_ent(t_file *f)
{
	free(f->name);
	free(f->linkname);
	f->name = NULL;
	f->linkname = NULL;
}

void	clear_files(void)
{
	size_t	i;

	i = 0;
	while (i < g_cwd_n_used)
		free_ent(&g_cwd_file[i++]);
	g_cwd_n_used = 0;
}

void	free_table(void)
{
	clear_files();
	free(g_cwd_file);
	free(g_sorted);
	g_cwd_file = NULL;
	g_sorted = NULL;
	g_cwd_n_alloc = 0;
	g_sorted_alloc = 0;
}

// Avoid using a lstat by file
static int	need_stat(void)
{
	if (g_format == FMT_LONG)
		return (1);
	if (g_sort_type == SORT_TIME || g_sort_type == SORT_SIZE)
		return (1);
	if (g_print_inode || g_print_block_size)
		return (1);
	if (g_print_with_color)
		return (1);
	if (g_indicator_style != IND_NONE)
		return (1);
	return (0);
}

// d_type struct dirent -> t_filetype.
// d_type doesnt exist on POSIX, thats why gobble_file have an emergency lstat
static t_filetype	type_from_dtype(unsigned char d_type)
{
	if (d_type == DT_FIFO)
		return (FIFO);
	else if (d_type == DT_CHR)
		return (CHARDEV);
	else if (d_type == DT_DIR)
		return (DIRECTORY);
	else if (d_type == DT_BLK)
		return (BLOCKDEV);
	else if (d_type == DT_REG)
		return (NORMAL);
	else if (d_type == DT_LNK)
		return (SYMLINK);
	else if (d_type == DT_SOCK)
		return (SOCK);
	return (UNKNOWN);
}

static t_filetype	type_from_mode(mode_t m)
{
	if (S_ISDIR(m))
		return (DIRECTORY);
	else if (S_ISLNK(m))
		return (SYMLINK);
	else if (S_ISREG(m))
		return (NORMAL);
	else if (S_ISFIFO(m))
		return (FIFO);
	else if (S_ISCHR(m))
		return (CHARDEV);
	else if (S_ISBLK(m))
		return (BLOCKDEV);
	else if (S_ISSOCK(m))
		return (SOCK);
	return (UNKNOWN);
}

// concanecate "dir/name"
// avoid producing "//name"
static char	*make_path(const char *dir, const char *name)
{
	size_t	dlen;
	size_t	nlen;
	size_t	slash;
	char	*path;

	dlen = strlen(dir);
	nlen = strlen(name);
	slash = (dlen > 0 && dir[dlen - 1] != '/');
	path = malloc(dlen + slash + nlen + 1);
	if (!path)
		return (NULL);
	memcpy(path, dir, dlen);
	if (slash)
		path[dlen] = '/';
	memcpy(path + dlen + slash, name, nlen);
	path[dlen + slash + nlen] = '\0';
	return (path);
}

// lstat (not stat) get info on the symlink itself, not the symlink' target
static int	stat_entry(t_file *f, const char *name, const char *dirname, int cmdline)
{
	char	*path;

	path = make_path(dirname, name);
	if (!path)
		return (0);
	if (lstat(path, &f->st) == 0)
	{
		f->stat_ok = 1;
		f->filetype = type_from_mode(f->st.st_mode);
		if (cmdline && f->filetype == DIRECTORY)
			f->filetype = ARG_DIRECTORY;
		free(path);
		return (1);
	}
	fprintf(stderr, "ft_ls: cannot access '%s': %s\n", path, strerror(errno));
	if (cmdline)
		g_exit_status = 2;
	else
		g_exit_status = 1;
	free(path);
	return (0);
}

// adds an entry from a name read by readdir
// returns 1 if added, 0 if filtered or alloc failed :c
// filter THEN stat (only if needed)
int	gobble_file(const char *name, unsigned char d_type, const char *dirname, int cmdline)
{
	t_file	*f;

	if (!cmdline && file_ignored(name))
		return (0);
	f = new_entry();
	if (!f)
		return (0);
	f->name = strdup(name);
	if (!f->name)
	{
		g_cwd_n_used--;
		return (0);
	}
	f->filetype = type_from_dtype(d_type);
	if (cmdline || need_stat() || f->filetype == UNKNOWN)
	{
		if (!stat_entry(f, name, dirname, cmdline) && cmdline)
		{
			free_ent(f);
			g_cwd_n_used--;
			return (0);
		}
	}
	return (1);
}
