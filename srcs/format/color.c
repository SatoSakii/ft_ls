#include "ft_ls.h"

// every color for each file type
static const char	*g_color[C_SLOT_COUNT] = {
	[C_LEFT] = "\033[",
	[C_RIGHT] = "m",
	[C_END] = NULL,
	[C_RESET] = "0",
	[C_NORM] = NULL,
	[C_FILE] = NULL,
	[C_DIR] = "01;34",
	[C_LINK] = "01;36",
	[C_FIFO] = "33",
	[C_SOCK] = "01;35",
	[C_BLK] = "01;33",
	[C_CHR] = "01;33",
	[C_MISSING] = NULL,
	[C_ORPHAN] = NULL,
	[C_EXEC] = "01;32",
	[C_SETUID] = "37;41",
	[C_SETGID] = "30;43",
	[C_STICKY] = "37;44",
	[C_OW] = "34;42",
	[C_STICKY_OW] = "30;42"
};

static char			*g_ls_colors;
static t_color_ext	*g_ext_list;
static int			g_used_color;
static int			g_link_as_target;

// ls colors for each file type
static const char	*g_keys[C_SLOT_COUNT] = {
	[C_LEFT] = "lc",
	[C_RIGHT] = "rc",
	[C_END] = "ec",
	[C_RESET] = "rs",
	[C_NORM] = "no",
	[C_FILE] = "fi",
	[C_DIR] = "di",
	[C_LINK] = "ln",
	[C_FIFO] = "pi",
	[C_SOCK] = "so",
	[C_BLK] = "bd",
	[C_CHR] = "cd",
	[C_MISSING] = "mi",
	[C_ORPHAN] = "or",
	[C_EXEC] = "ex",
	[C_SETUID] = "su",
	[C_SETGID] = "sg",
	[C_STICKY] = "st",
	[C_OW] = "ow",
	[C_STICKY_OW] = "tw"
};

int	color_is_set(t_color_slot slot)
{
	return (g_color[slot] != NULL);
}

static void	add_ext(const char *suffix, const char *seq)
{
	t_color_ext	*e;

	e = malloc(sizeof(t_color_ext));
	if (!e)
		return ;
	e->suffix = suffix;
	e->seq = seq;
	e->next = g_ext_list;
	g_ext_list = e;
}

// is there key=value?
static void	set_entry(char *key, char *value)
{
	size_t	i;

	if (key[0] == '*')
	{
		add_ext(key + 1, value);
		return ;
	}
	if (!strcmp(key, "ln") && !strcmp(value, "target"))
	{
		g_link_as_target = 1;
		return ;
	}
	i = 0;
	while (i < C_SLOT_COUNT)
	{
		if (g_keys[i] && !strcmp(g_keys[i], key))
		{
			g_color[i] = value;
			return ;
		}
		i++;
	}
}

static void	parse_ls_colors(char *s)
{
	char	*eq;
	char	*next;

	while (s && *s)
	{
		next = strchr(s, ':');
		if (next)
			*next++ = '\0';
		eq = strchr(s, '=');
		if (eq)
		{
			*eq = '\0';
			set_entry(s, eq + 1);
		}
		s = next;
	}
}

void	color_init(void)
{
	char	*env;

	if (!g_print_with_color)
		return ;
	env = getenv("LS_COLORS");
	if (!env || !*env)
		return ;
	g_ls_colors = strdup(env);
	if (!g_ls_colors)
		return ;
	parse_ls_colors(g_ls_colors);
}

void	free_colors(void)
{
	t_color_ext	*e;

	while (g_ext_list)
	{
		e = g_ext_list->next;
		free(g_ext_list);
		g_ext_list = e;
	}
	free(g_ls_colors);
	g_ls_colors = NULL;
}

static t_color_slot	slot_regular(mode_t m)
{
	if ((m & S_ISUID) && g_color[C_SETUID])
		return (C_SETUID);
	if ((m & S_ISGID) && g_color[C_SETGID])
		return (C_SETGID);
	if ((m & (S_IXUSR | S_IXGRP | S_IXOTH)) && g_color[C_EXEC])
		return (C_EXEC);
	return (C_FILE);
}

static t_color_slot	slot_dir(mode_t m)
{
	if ((m & S_ISVTX) && (m & S_IWOTH) && g_color[C_STICKY_OW])
		return (C_STICKY_OW);
	if ((m & S_IWOTH) && g_color[C_OW])
		return (C_OW);
	if ((m & S_ISVTX) && g_color[C_STICKY])
		return (C_STICKY);
	return (C_DIR);
}

static t_color_slot	slot_of_mode(mode_t m)
{
	if (S_ISREG(m))
		return (slot_regular(m));
	if (S_ISDIR(m))
		return (slot_dir(m));
	if (S_ISLNK(m))
		return (C_LINK);
	if (S_ISFIFO(m))
		return (C_FIFO);
	if (S_ISSOCK(m))
		return (C_SOCK);
	if (S_ISBLK(m))
		return (C_BLK);
	if (S_ISCHR(m))
		return (C_CHR);
	return (C_ORPHAN);
}

// return the color for a matching file extension
static const char	*ext_seq(const char *name)
{
	t_color_ext	*e;
	size_t		nlen;
	size_t		elen;

	nlen = strlen(name);
	e = g_ext_list;
	while (e)
	{
		elen = strlen(e->suffix);
		if (elen <= nlen && !strcasecmp(name + nlen - elen, e->suffix))
			return (e->seq);
		e = e->next;
	}
	return (NULL);
}

// LS_COLORS can hold "ln=target", which asks for a symlink to be painted
// with the color of the file it points to
int	color_symlink_as_target(void)
{
	return (g_link_as_target);
}

static mode_t	entry_mode(const t_file *f)
{
	if (g_link_as_target && f->filetype == SYMLINK && f->linkok)
		return (f->linkmode);
	if (f->stat_ok)
		return (f->st.st_mode);
	return (mode_from_type(f->filetype));
}

static const char	*color_for(const t_file *f, const char *name, int target)
{
	mode_t			m;
	t_color_slot	slot;
	const char		*seq;

	if (target && !f->linkok && g_color[C_MISSING])
		return (g_color[C_MISSING]);
	if (target)
		m = f->linkmode;
	else
		m = entry_mode(f);
	slot = slot_of_mode(m);
	if (slot == C_FILE)
	{
		seq = ext_seq(name);
		if (seq)
			return (seq);
	}
	if (slot == C_LINK && !f->linkok
		&& (g_color[C_ORPHAN] || g_link_as_target))
		slot = C_ORPHAN;
	return (g_color[slot]);
}

static void	end_color(void)
{
	if (g_color[C_END])
	{
		fputs(g_color[C_END], stdout);
		return ;
	}
	fputs(g_color[C_LEFT], stdout);
	fputs(g_color[C_RESET], stdout);
	fputs(g_color[C_RIGHT], stdout);
}

void	print_colored(const t_file *f, const char *name, int target)
{
	const char	*seq;

	seq = NULL;
	if (g_print_with_color)
		seq = color_for(f, name, target);
	if (!seq)
	{
		fputs(name, stdout);
		return ;
	}
	if (!g_used_color)
	{
		g_used_color = 1;
		end_color();
	}
	fputs(g_color[C_LEFT], stdout);
	fputs(seq, stdout);
	fputs(g_color[C_RIGHT], stdout);
	fputs(name, stdout);
	end_color();
}
