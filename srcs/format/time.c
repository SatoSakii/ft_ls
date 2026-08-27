#include "ft_ls.h"

static time_t	now(void)
{
	static time_t	cached;

	if (cached == 0)
		cached = time(NULL);
	return (cached);
}

// return 1 if the date is within the last six months and not in the future
static int	is_recent(time_t t)
{
	time_t	n;

	n = now();
	return (t <= n && t >= n - SIX_MONTHS);
}

const char	*format_time(const struct timespec *ts)
{
	static char	buf[64];
	struct tm	*tm;

	tm = localtime(&ts->tv_sec);
	if (!tm)
		return ("?");
	if (is_recent(ts->tv_sec))
		strftime(buf, sizeof(buf), "%b %e %H:%M", tm);
	else
		strftime(buf, sizeof(buf), "%b %e  %Y", tm);
	return (buf);
}