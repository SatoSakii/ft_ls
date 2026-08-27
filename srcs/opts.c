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
	else if (c == 't')		{ g_sort_type = SORT_TIME; g_sort_set = 1; }
	else if (c == 'S')		{ g_sort_type = SORT_SIZE; g_sort_set = 1; }
	else if (c == 'U')		{ g_sort_type = SORT_NONE; g_sort_set = 1; }
	else if (c == 'f')		{ g_sort_type = SORT_NONE; g_sort_set = 1;
							  g_ignore_mode = IGNORE_MINIMAL; }
	else if (c == 'u')		{ g_time_type = TIME_ATIME; g_explicit_time = 1; }
	else if (c == 'c')		{ g_time_type = TIME_CTIME; g_explicit_time = 1; }
	else if (c == 'r')		{ g_sort_reverse = 1; }
	else					return (0);
	return (1);
}

static int	apply_display(int c)
{
	if (c == 'a')			{ g_ignore_mode = IGNORE_MINIMAL; }
	else if (c == 'A')		{ g_ignore_mode = IGNORE_DOT_AND_DOTDOT; }
	else if (c == 'R')		{ g_recursive = 1; }
	else if (c == 'd')		{ g_immediate_dirs = 1; }
	else if (c == 'F')		{ g_indicator_style = IND_CLASSIFY; }
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
	else if (c == 'w')		{ g_line_length = (size_t)atoi(optarg);
							  g_width_set = 1; }
	else					return (0);
	return (1);
}

static void	arg_error(const char *opt, const char *value, const char *valid)
{
	fprintf(stderr, "ft_ls: invalid argument '%s' for '%s'\n", value, opt);
	fprintf(stderr, "Valid arguments are:\n%s", valid);
	fprintf(stderr, "Try 'ft_ls --help' for more information.\n");
	exit(2);
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
	else if (!strcmp(optarg, "time"))
		g_sort_type = SORT_TIME;
	else if (!strcmp(optarg, "size"))
		g_sort_type = SORT_SIZE;
	else if (!strcmp(optarg, "extension"))
		g_sort_type = SORT_EXTENSION;
	else if (!strcmp(optarg, "width"))
		g_sort_type = SORT_WIDTH;
	else if (!strcmp(optarg, "version"))
		g_sort_type = SORT_VERSION;
	else
		arg_error("--sort", optarg,
			"  - 'none'\n  - 'time'\n  - 'size'\n"
			"  - 'extension'\n  - 'width'\n  - 'version'\n");
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
	else if (!strcmp(optarg, "single-column"))
		g_format = FMT_ONE_PER_LINE;
	else if (!strcmp(optarg, "vertical"))
		g_format = FMT_MANY_PER_LINE;
	else if (!strcmp(optarg, "across") || !strcmp(optarg, "horizontal"))
		g_format = FMT_HORIZONTAL;
	else
		arg_error("--format", optarg,
			"  - 'verbose', 'long'\n  - 'single-column'\n"
			"  - 'vertical'\n  - 'across', 'horizontal'\n");
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
	printf("Usage: ft_ls [OPTION]... [FILE]...\n");
	printf("List information about the FILEs "
		"(the current directory by default).\n");
	printf("Sort entries alphabetically unless "
		"-t, -S, -U or --sort is given.\n\n");
	printf("  -a, --all                  do not ignore entries starting with .\n");
	printf("  -A, --almost-all           do not list implied . and ..\n");
	printf("  -c                         sort by, and show, ctime\n");
	printf("  -d, --directory            list directories themselves, "
		"not their contents\n");
	printf("  -f                         do not sort, enable -a\n");
	printf("  -F, --classify             append indicator (one of */=>@|) "
		"to entries\n");
	printf("  -g                         like -l, but do not list owner\n");
	printf("  -G, --no-group             in a long listing, "
		"don't print group names\n");
	printf("  -h, --human-readable       print sizes like 1K 234M 2G etc.\n");
	printf("  -i, --inode                print the index number of each file\n");
	printf("  -l                         use a long listing format\n");
	printf("  -n, --numeric-uid-gid      like -l, but list numeric user "
		"and group IDs\n");
	printf("  -o                         like -l, but do not list "
		"group information\n");
	printf("  -p                         append / indicator to directories\n");
	printf("  -r, --reverse              reverse order while sorting\n");
	printf("  -R, --recursive            list subdirectories recursively\n");
	printf("  -s, --size                 print the allocated size of each "
		"file, in blocks\n");
	printf("  -S                         sort by file size, largest first\n");
	printf("  -t                         sort by time, newest first\n");
	printf("  -u                         sort by, and show, atime\n");
	printf("  -U                         do not sort; list entries in "
		"directory order\n");
	printf("  -w, --width=COLS           set output width to COLS, 0 means "
		"no limit\n");
	printf("  -x                         list entries by lines "
		"instead of by columns\n");
	printf("  -1                         list one file per line\n\n");
	printf("      --color[=WHEN]         color the output WHEN; more info below\n");
	printf("      --format=WORD          across, horizontal, long, "
		"single-column, verbose, vertical\n");
	printf("      --indicator-style=WORD append indicator with style WORD to "
		"entry names:\n");
	printf("                               none, slash (-p), file-type, "
		"classify (-F)\n");
	printf("      --sort=WORD            sort by WORD instead of name: none (-U), "
		"size (-S),\n");
	printf("                               time (-t), version, extension, width\n");
	printf("      --time=WORD            select which timestamp is used to "
		"display or sort:\n");
	printf("                               atime (-u), ctime (-c), mtime\n");
	printf("      --help                 display this help and exit\n\n");
	printf("The WHEN argument defaults to 'always' and can also be "
		"'auto' or 'never'.\n");
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

static void	usage_error(char **argv)
{
	char	*bad;

	bad = argv[optind - 1];
	if (bad[0] == '-' && bad[1] == '-')
		fprintf(stderr, "ft_ls: unrecognized option '%s'\n", bad);
	else
		fprintf(stderr, "ft_ls: invalid option -- '%c'\n", optopt);
	fprintf(stderr, "Try 'ft_ls --help' for more information.\n");
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
	if (!g_width_set && (g_format == FMT_MANY_PER_LINE
			|| g_format == FMT_HORIZONTAL))
		g_line_length = term_width();
	if (g_immediate_dirs)
		g_recursive = 0;
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
		c = getopt_long(argc, argv, "aAcdfFgGhilnopRrsStuUw:x1",
				g_long_options, &optindex);
		if (c == -1)
			break ;
		if (!apply_switch(c))
			usage_error(argv);
		optindex = -1;
	}
	resolve_conflicts();
	return (optind);
}
