#include "ft_ls.h"

static char	type_letter(mode_t m)
{
	if (S_ISDIR(m))
		return ('d');
	else if (S_ISLNK(m))
		return ('l');
	else if (S_ISCHR(m))
		return ('c');
	else if (S_ISBLK(m))
		return ('b');
	else if (S_ISFIFO(m))
		return ('p');
	else if (S_ISSOCK(m))
		return ('s');
	else if (S_ISREG(m))
		return ('-');
	return ('?');
}

// return the character for the execute permission
// special bit (setuid, setgid or sticky) changes x to s/t
static char	exec_char(int has_exec, int has_special, char special)
{
	if (has_special && has_exec)
		return (special);
	if (has_special)
		return (special - 32);
	if (has_exec)
		return ('x');
	return ('-');
}

// when lstat failed, readdir still knows the type: ls keeps the type letter
// and fills the nine permission slots with '?' rather than inventing them
mode_t	mode_from_type(t_filetype t)
{
	if (t == DIRECTORY || t == ARG_DIRECTORY)
		return (S_IFDIR);
	if (t == SYMLINK)
		return (S_IFLNK);
	if (t == CHARDEV)
		return (S_IFCHR);
	if (t == BLOCKDEV)
		return (S_IFBLK);
	if (t == FIFO)
		return (S_IFIFO);
	if (t == SOCK)
		return (S_IFSOCK);
	if (t == NORMAL)
		return (S_IFREG);
	return (0);
}

static void	unknown_mode_string(const t_file *f, char *out)
{
	int	i;

	out[0] = type_letter(mode_from_type(f->filetype));
	i = 1;
	while (i < 10)
		out[i++] = '?';
	out[10] = '\0';
}

void	mode_string(const t_file *f, char *out)
{
	mode_t	m;

	if (!f->stat_ok)
		return (unknown_mode_string(f, out));
	m = f->st.st_mode;
	out[0] = type_letter(m);
	out[1] = (m & S_IRUSR) ? 'r' : '-';
	out[2] = (m & S_IWUSR) ? 'w' : '-';
	out[3] = exec_char(m & S_IXUSR, m & S_ISUID, 's');
	out[4] = (m & S_IRGRP) ? 'r' : '-';
	out[5] = (m & S_IWGRP) ? 'w' : '-';
	out[6] = exec_char(m & S_IXGRP, m & S_ISGID, 's');
	out[7] = (m & S_IROTH) ? 'r' : '-';
	out[8] = (m & S_IWOTH) ? 'w' : '-';
	out[9] = exec_char(m & S_IXOTH, m & S_ISVTX, 't');
	out[10] = '\0';
}