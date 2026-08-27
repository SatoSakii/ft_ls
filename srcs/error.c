#include "ft_ls.h"

// preverse output order by flushing stdout
void	file_failure(int cmdline, const char *msg, const char *path)
{
	fflush(stdout);
	fprintf(stderr, "ft_ls: %s '%s': %s\n", msg, path, strerror(errno));
	if (cmdline)
		g_exit_status = 2;
	else
		g_exit_status = 1;
}