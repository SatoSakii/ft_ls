#include "ft_ls.h"

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