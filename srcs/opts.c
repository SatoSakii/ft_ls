#include "ft_ls.h"

static struct option const	g_long_options[] =
{
	{"all",				no_argument,		NULL, 'a'},
	{"almost-all",		no_argument,		NULL, 'A'},
	{"directory",		no_argument,		NULL, 'd'},
	{"classify",		optional_argument,	NULL, 'F'},
	{"human-readable",	no_argument,		NULL, 'h'},
	{"inode",			no_argument,		NULL, 'i'},
	{"numeric-uid-gid",	no_argument,		NULL, 'n'},
	{"no-group",		no_argument,		NULL, 'G'},
	{"reverse",			no_argument,		NULL, 'r'},
	{"recursive",		no_argument,		NULL, 'R'},
	{"size",			no_argument,		NULL, 's'},
	{"width",			required_argument,	NULL, 'w'},
	{"color",			optional_argument,	NULL, COLOR_OPTION},
	{"sort",			required_argument,	NULL, SORT_OPTION},
	{"time",			required_argument,	NULL, TIME_OPTION},
	{"indicator-style",	required_argument,	NULL, INDICATOR_STYLE_OPTION},
	{"format",			required_argument,	NULL, FORMAT_OPTION},
	{"help",			no_argument,		NULL, HELP_OPTION},
	{NULL, 0, NULL, 0}
};

static void	set_defaults(void)
{
	g_format = FMT_MANY_PER_LINE;
	g_sort_type = SORT_NAME;
	g_time_type = TIME_MTIME;
	g_indicator_style = IND_NONE;
	g_ignore_mode = IGNORE_DEFAULT;
	g_color_when = COLOR_NEVER;
	g_print_owner = 1;
	g_print_group = 1;
	g_exit_status = 0;
}

static int	apply_sort_and_format(int c)
{
	if (c == 'l')			{ g_format = FMT_LONG; g_format_set = 1; }
	else if (c == '1')		{ if (g_format != FMT_LONG)
								g_format = FMT_ONE_PER_LINE;
							  g_format_set = 1; }
	else if (c == 'x')		{ g_format = FMT_HORIZONTAL; g_format_set = 1; }
	else if (c == 'C')		{ g_format = FMT_MANY_PER_LINE; g_format_set = 1; }
	else if (c == 't')		{ g_sort_type = SORT_TIME; g_sort_set = 1; }
	else if (c == 'S')		{ g_sort_type = SORT_SIZE; g_sort_set = 1; }
	else if (c == 'X')		{ g_sort_type = SORT_EXTENSION; g_sort_set = 1; }
	else if (c == 'U')		{ g_sort_type = SORT_NONE; g_sort_set = 1; }
	else if (c == 'f')		{ g_sort_type = SORT_NONE; g_sort_set = 1;
							  g_ignore_mode = IGNORE_MINIMAL; }
	else if (c == 'u')		{ g_time_type = TIME_ATIME; g_explicit_time = 1; }
	else if (c == 'c')		{ g_time_type = TIME_CTIME; g_explicit_time = 1; }
	else if (c == 'r')		{ g_sort_reverse = 1; }
	else					return (0);
	return (1);
}

static void	arg_error(const char *opt, const char *value, const char *valid)
{
	fprintf(stderr, "ft_ls: invalid argument '%s' for '%s'\n", value, opt);
	fprintf(stderr, "Valid arguments are:\n%s", valid);
	fputs("Try 'ft_ls --help' for more information.\n", stderr);
	exit(1);
}

// -F alone means always, but --classify takes an optional WHEN like --color
static t_indicator_style	classify_when(void)
{
	if (!optarg || !strcmp(optarg, "always") || !strcmp(optarg, "yes")
		|| !strcmp(optarg, "force"))
		return (IND_CLASSIFY);
	if (!strcmp(optarg, "never") || !strcmp(optarg, "no")
		|| !strcmp(optarg, "none"))
		return (IND_NONE);
	if (!strcmp(optarg, "auto") || !strcmp(optarg, "tty")
		|| !strcmp(optarg, "if-tty"))
	{
		if (isatty(STDOUT_FILENO))
			return (IND_CLASSIFY);
		return (IND_NONE);
	}
	arg_error("--classify", optarg,
		"  - 'always', 'yes', 'force'\n"
		"  - 'never', 'no', 'none'\n"
		"  - 'auto', 'tty', 'if-tty'\n");
	return (IND_NONE);
}

// atoi would silently turn a bad -w into 0, which means "no limit"
static void	set_line_width(void)
{
	unsigned long	v;
	char			*end;
	char			*s;

	s = optarg;
	while (isspace((unsigned char)*s))
		s++;
	errno = 0;
	v = strtoul(s, &end, 0);
	if (*s == '-' || s == end || *end != '\0')
	{
		fflush(stdout);
		fprintf(stderr, "ft_ls: invalid line width: '%s'\n", optarg);
		exit(2);
	}
	if (errno == ERANGE)
		v = ULONG_MAX;
	g_line_length = (size_t)v;
	g_width_set = 1;
}

static int	apply_display(int c)
{
	if (c == 'a')			{ g_ignore_mode = IGNORE_MINIMAL; }
	else if (c == 'A')		{ g_ignore_mode = IGNORE_DOT_AND_DOTDOT; }
	else if (c == 'R')		{ g_recursive = 1; }
	else if (c == 'd')		{ g_immediate_dirs = 1; }
	else if (c == 'F')		{ g_indicator_style = classify_when(); }
	else if (c == 'p')		{ g_indicator_style = IND_SLASH; }
	else if (c == 'i')		{ g_print_inode = 1; }
	else if (c == 's')		{ g_print_block_size = 1; }
	else if (c == 'h')		{ g_human = 1; }
	else if (c == 'n')		{ g_numeric_ids = 1; g_format = FMT_LONG;
							  g_format_set = 1; }
	else if (c == 'g')		{ g_print_owner = 0; g_format = FMT_LONG;
							  g_format_set = 1; }
	else if (c == 'o' || c == 'G')
							{ g_print_group = 0;
							  if (c == 'o')
							  {
								  g_format = FMT_LONG;
								  g_format_set = 1;
							  } }
	else if (c == 'w')		{ set_line_width(); }
	else					return (0);
	return (1);
}

static void	apply_color_option(void)
{
	if (!optarg || !strcmp(optarg, "always") || !strcmp(optarg, "yes")
		|| !strcmp(optarg, "force"))
		g_color_when = COLOR_ALWAYS;
	else if (!strcmp(optarg, "never") || !strcmp(optarg, "no")
		|| !strcmp(optarg, "none"))
		g_color_when = COLOR_NEVER;
	else if (!strcmp(optarg, "auto") || !strcmp(optarg, "tty")
		|| !strcmp(optarg, "if-tty"))
		g_color_when = COLOR_AUTO;
	else
		arg_error("--color", optarg,
			"  - 'always', 'yes', 'force'\n"
			"  - 'never', 'no', 'none'\n"
			"  - 'auto', 'tty', 'if-tty'\n");
}

static void	apply_sort_option(void)
{
	if (!strcmp(optarg, "none"))
		g_sort_type = SORT_NONE;
	else if (!strcmp(optarg, "size"))
		g_sort_type = SORT_SIZE;
	else if (!strcmp(optarg, "time"))
		g_sort_type = SORT_TIME;
	else if (!strcmp(optarg, "extension"))
		g_sort_type = SORT_EXTENSION;
	else if (!strcmp(optarg, "name"))
		g_sort_type = SORT_NAME;
	else if (!strcmp(optarg, "width"))
		g_sort_type = SORT_WIDTH;
	else
		arg_error("--sort", optarg,
			"  - 'none'\n  - 'size'\n  - 'time'\n"
			"  - 'extension'\n  - 'name'\n  - 'width'\n");
	g_sort_set = 1;
}

static void	apply_time_option(void)
{
	if (!strcmp(optarg, "atime") || !strcmp(optarg, "access")
		|| !strcmp(optarg, "use"))
		g_time_type = TIME_ATIME;
	else if (!strcmp(optarg, "ctime") || !strcmp(optarg, "status"))
		g_time_type = TIME_CTIME;
	else if (!strcmp(optarg, "mtime") || !strcmp(optarg, "modification"))
		g_time_type = TIME_MTIME;
	else
		arg_error("--time", optarg,
			"  - 'atime', 'access', 'use'\n"
			"  - 'ctime', 'status'\n"
			"  - 'mtime', 'modification'\n");
	g_explicit_time = 1;
}

static void	apply_format_option(void)
{
	if (!strcmp(optarg, "verbose") || !strcmp(optarg, "long"))
		g_format = FMT_LONG;
	else if (!strcmp(optarg, "horizontal") || !strcmp(optarg, "across"))
		g_format = FMT_HORIZONTAL;
	else if (!strcmp(optarg, "vertical"))
		g_format = FMT_MANY_PER_LINE;
	else if (!strcmp(optarg, "single-column"))
		g_format = FMT_ONE_PER_LINE;
	else
		arg_error("--format", optarg,
			"  - 'verbose', 'long'\n  - 'horizontal', 'across'\n"
			"  - 'vertical'\n  - 'single-column'\n");
	g_format_set = 1;
}

static void	apply_indicator_option(void)
{
	if (!strcmp(optarg, "none"))
		g_indicator_style = IND_NONE;
	else if (!strcmp(optarg, "slash"))
		g_indicator_style = IND_SLASH;
	else if (!strcmp(optarg, "file-type"))
		g_indicator_style = IND_FILE_TYPE;
	else if (!strcmp(optarg, "classify"))
		g_indicator_style = IND_CLASSIFY;
	else
		arg_error("--indicator-style", optarg,
			"  - 'none'\n  - 'slash'\n  - 'file-type'\n  - 'classify'\n");
}

static void	print_help(void)
{
	fputs("Usage: ft_ls [OPTION]... [FILE]...\n", stdout);
	fputs("List information about the FILEs "
		"(the current directory by default).\n", stdout);
	fputs("Sort entries alphabetically unless "
		"-t, -S, -X, -U or --sort is given.\n\n", stdout);
	fputs("  -a, --all                  do not ignore entries starting with .\n", stdout);
	fputs("  -A, --almost-all           do not list implied . and ..\n", stdout);
	fputs("  -c                         sort by, and show, ctime\n", stdout);
	fputs("  -C                         list entries by columns\n", stdout);
	fputs("  -d, --directory            list directories themselves, "
		"not their contents\n", stdout);
	fputs("  -f                         do not sort, enable -a\n", stdout);
	fputs("  -F, --classify[=WHEN]      append indicator (one of */=>@|) "
		"to entries WHEN\n", stdout);
	fputs("  -g                         like -l, but do not list owner\n", stdout);
	fputs("  -G, --no-group             in a long listing, "
		"don't print group names\n", stdout);
	fputs("  -h, --human-readable       print sizes like 1K 234M 2G etc.\n", stdout);
	fputs("  -i, --inode                print the index number of each file\n", stdout);
	fputs("  -l                         use a long listing format\n", stdout);
	fputs("  -n, --numeric-uid-gid      like -l, but list numeric user "
		"and group IDs\n", stdout);
	fputs("  -o                         like -l, but do not list "
		"group information\n", stdout);
	fputs("  -p                         append / indicator to directories\n", stdout);
	fputs("  -r, --reverse              reverse order while sorting\n", stdout);
	fputs("  -R, --recursive            list subdirectories recursively\n", stdout);
	fputs("  -s, --size                 print the allocated size of each "
		"file, in blocks\n", stdout);
	fputs("  -S                         sort by file size, largest first\n", stdout);
	fputs("  -t                         sort by time, newest first\n", stdout);
	fputs("  -u                         sort by, and show, atime\n", stdout);
	fputs("  -U                         do not sort; list entries in "
		"directory order\n", stdout);
	fputs("  -w, --width=COLS           set output width to COLS, 0 means "
		"no limit\n", stdout);
	fputs("  -x                         list entries by lines "
		"instead of by columns\n", stdout);
	fputs("  -X                         sort alphabetically by entry "
		"extension\n", stdout);
	fputs("  -1                         list one file per line\n\n", stdout);
	fputs("      --color[=WHEN]         color the output WHEN; more info below\n", stdout);
	fputs("      --format=WORD          across, horizontal, long, "
		"single-column, verbose, vertical\n", stdout);
	fputs("      --indicator-style=WORD append indicator with style WORD to "
		"entry names:\n", stdout);
	fputs("                               none, slash (-p), file-type, "
		"classify (-F)\n", stdout);
	fputs("      --sort=WORD            sort by WORD instead of name: none (-U), "
		"size (-S),\n", stdout);
	fputs("                               time (-t), extension (-X), name, "
		"width\n", stdout);
	fputs("      --time=WORD            select which timestamp is used to "
		"display or sort:\n", stdout);
	fputs("                               atime (-u), ctime (-c), mtime\n", stdout);
	fputs("      --help                 display this help and exit\n\n", stdout);
	fputs("The WHEN argument defaults to 'always' and can also be "
		"'auto' or 'never'.\n", stdout);
	exit(0);
}

static int	apply_long_only(int c)
{
	if (c == COLOR_OPTION)
		apply_color_option();
	else if (c == SORT_OPTION)
		apply_sort_option();
	else if (c == TIME_OPTION)
		apply_time_option();
	else if (c == FORMAT_OPTION)
		apply_format_option();
	else if (c == INDICATOR_STYLE_OPTION)
		apply_indicator_option();
	else if (c == HELP_OPTION)
		print_help();
	else
		return (0);
	return (1);
}

static int	apply_switch(int c)
{
	if (apply_sort_and_format(c))
		return (1);
	if (apply_display(c))
		return (1);
	return (apply_long_only(c));
}

// getopt returns ':' for a missing argument when the optstring starts with ':'
static void	missing_arg_error(char **argv)
{
	char	*bad;

	bad = argv[optind - 1];
	fflush(stdout);
	if (bad[0] == '-' && bad[1] == '-')
		fprintf(stderr, "ft_ls: option '%s' requires an argument\n", bad);
	else
		fprintf(stderr, "ft_ls: option requires an argument -- '%c'\n", optopt);
	fputs("Try 'ft_ls --help' for more information.\n", stderr);
	exit(2);
}

static void	usage_error(char **argv)
{
	char	*bad;

	bad = argv[optind - 1];
	fflush(stdout);
	if (bad[0] == '-' && bad[1] == '-')
		fprintf(stderr, "ft_ls: unrecognized option '%s'\n", bad);
	else
		fprintf(stderr, "ft_ls: invalid option -- '%c'\n", optopt);
	fputs("Try 'ft_ls --help' for more information.\n", stderr);
	exit(2);
}

static void	resolve_conflicts(void)
{
	g_is_tty = isatty(STDOUT_FILENO);
	if (!g_format_set)
	{
		if (g_is_tty)
			g_format = FMT_MANY_PER_LINE;
		else
			g_format = FMT_ONE_PER_LINE;
	}
	if (!g_sort_set && g_explicit_time && g_format != FMT_LONG)
		g_sort_type = SORT_TIME;
	if (!g_width_set && (g_format == FMT_MANY_PER_LINE
			|| g_format == FMT_HORIZONTAL))
		g_line_length = term_width();
	if (g_immediate_dirs)
		g_recursive = 0;
	g_deref_cmdline = !(g_immediate_dirs || g_format == FMT_LONG || g_indicator_style == IND_CLASSIFY);
	if (g_color_when == COLOR_AUTO)
		g_print_with_color = g_is_tty;
	else
		g_print_with_color = (g_color_when == COLOR_ALWAYS);
}

int	decode_switches(int argc, char **argv)
{
	int	c;
	int	optindex;

	set_defaults();
	opterr = 0;
	optindex = -1;
	while (1)
	{
		c = getopt_long(argc, argv, ":aACcdfFgGhilnopRrsStuUw:xX1",
				g_long_options, &optindex);
		if (c == -1)
			break ;
		if (c == ':')
			missing_arg_error(argv);
		if (!apply_switch(c))
			usage_error(argv);
		optindex = -1;
	}
	resolve_conflicts();
	return (optind);
}
