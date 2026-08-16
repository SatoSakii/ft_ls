#include "ft_ls.h"
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>

size_t	term_width(void)
{
	struct winsize	ws;
	char			*env;
	int				value;

	env = getenv("COLUMNS");
	if (env && *env)
	{
		value = atoi(env);
		if (value > 0)
			return ((size_t)value);
	}
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
		return ((size_t)ws.ws_col);
	return (80);
}
