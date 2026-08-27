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

void	mode_string(mode_t m, char *out)
{
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