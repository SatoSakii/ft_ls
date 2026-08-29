#include "ft_ls.h"

// lstat may have failed, in that case fall back on the readdir type
char	entry_indicator(const t_file *f)
{
	if (f->stat_ok)
		return (file_indicator(f->st.st_mode));
	return (file_indicator(mode_from_type(f->filetype)));
}

char	file_indicator(mode_t m)
{
	if (g_indicator_style == IND_NONE)
		return (0);
	if (S_ISDIR(m))
		return ('/');
	if (g_indicator_style == IND_SLASH)
		return (0);
	if (S_ISLNK(m))
		return ('@');
	if (S_ISFIFO(m))
		return ('|');
	if (S_ISSOCK(m))
		return ('=');
	if (g_indicator_style == IND_CLASSIFY && S_ISREG(m) && (m & (S_IXUSR | S_IXGRP | S_IXOTH)))
		return ('*');
	return (0);
}