#include "ft_ls.h"

size_t	num_width(unsigned long long n)
{
	size_t	w;

	w = 1;
	while (n >= 10)
	{
		n /= 10;
		w++;
	}
	return (w);
}

unsigned long long	blocks_to_kib(unsigned long long blocks512)
{
	return ((blocks512 + 1) / 2);
}

// Format a size like GNU ls -h.
const char	*human_size(unsigned long long size)
{
	static char			buf[32];
	static const char	*units = "BKMGTPE";
	unsigned long long	div;
	unsigned long long	tenths;
	int					i;

	div = 1;
	i = 0;
	while (size / div >= 1024 && units[i + 1])
	{
		div *= 1024;
		i++;
	}
	if (i == 0)
	{
		snprintf(buf, sizeof(buf), "%llu", size);
		return (buf);
	}
	tenths = (size / div) * 10 + ((size % div) * 10 + div - 1) / div;
	if (tenths >= 100)
		snprintf(buf, sizeof(buf), "%llu%c", (tenths + 9) / 10, units[i]);
	else
		snprintf(buf, sizeof(buf), "%llu.%llu%c", tenths / 10, tenths % 10,	units[i]);
	return (buf);
}

// block for -s
// ls -sh should display 4.0K, not 4
const char	*blocks_field(const t_file *f)
{
	static char			buf[32];
	unsigned long long	b;

	if (!f->stat_ok)
		return ("?");
	b = (unsigned long long)f->st.st_blocks;
	if (g_human)
		return (human_size(b * 512));
	snprintf(buf, sizeof(buf), "%llu", blocks_to_kib(b));
	return (buf);
}

// format the size field for -l
const char	*size_field(const t_file *f, size_t maj_w, size_t min_w)
{
	static char	buf[64];

	if (!f->stat_ok)
		return ("?");
	if (S_ISCHR(f->st.st_mode) || S_ISBLK(f->st.st_mode))
	{
		snprintf(buf, sizeof(buf), "%*u, %*u",
			(int)maj_w, (unsigned int)major(f->st.st_rdev),
			(int)min_w, (unsigned int)minor(f->st.st_rdev));
		return (buf);
	}
	if (g_human)
		return (human_size((unsigned long long)f->st.st_size));
	snprintf(buf, sizeof(buf), "%lld", (long long)f->st.st_size);
	return (buf);
}