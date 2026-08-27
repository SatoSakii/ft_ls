#ifndef FT_LS_H
# define FT_LS_H

# include <getopt.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <limits.h>
# include <stddef.h>
# include <stdlib.h>
# include <dirent.h>
# include <string.h>
# include <unistd.h>
# include <stdio.h>
# include <errno.h>

// file types recognized by ls
// ARG_DIRECTORY is used only for dir passed as arguments, arg + directory, lol.
typedef enum e_filetype
{
	UNKNOWN,
	FIFO,
	CHARDEV,
	DIRECTORY,
	BLOCKDEV,
	NORMAL,
	SYMLINK,
	SOCK,
	ARG_DIRECTORY
}	t_filetype;

// sorting methods
typedef enum e_sort_type
{
	SORT_NAME = 0,
	SORT_EXTENSION,
	SORT_WIDTH,
	SORT_SIZE,
	SORT_VERSION,
	SORT_TIME,
	SORT_NONE
}	t_sort_type;

// date used for time sorting, also for -l opt
typedef enum e_time_type
{
	TIME_MTIME = 0,
	TIME_CTIME,
	TIME_ATIME
}	t_time_type;

// available formats
// long format has priority over other formats
typedef enum e_format
{
	FMT_LONG,
	FMT_ONE_PER_LINE,
	FMT_MANY_PER_LINE,
	FMT_HORIZONTAL
}	t_format;

// suffix after name
// eg: folder srcs = srcs/ (-p)
// eg: executable ft_ls = ft_ls* (-F)
typedef enum e_indicator_style
{
	IND_NONE = 0,
	IND_SLASH,
	IND_FILE_TYPE,
	IND_CLASSIFY
}	t_indicator_style;

// hidden file filter
// IGNORE_DEFAULT hides names starting with '.'
// IGNORE_DOT_AND_DOTDOT shows hidden files except '.' and '..'
// IGNORE_MINIMAL shows everything
typedef enum e_ignore_mode
{
	IGNORE_DEFAULT = 0,
	IGNORE_DOT_AND_DOTDOT,
	IGNORE_MINIMAL
}	t_ignore_mode;

// --color mode
typedef enum e_color_when
{
	COLOR_NEVER = 0,
	COLOR_ALWAYS,
	COLOR_AUTO
}	t_color_when;

// values used by getopt_long for opts without a short eq.
typedef enum e_no_short_opts
{
	COLOR_OPTION = CHAR_MAX + 1,
	SORT_OPTION,
	TIME_OPTION,
	INDICATOR_STYLE_OPTION,
	FORMAT_OPTION,
	HELP_OPTION
}	t_no_short_opts;

// st = file info collected by lstat
// filetype = taken from dtype
// stat ok = if lstat failed
// linkok = if symlike target doesnt exist
typedef struct s_file
{
	char		*name;
	char		*linkname;
	struct stat	st;
	t_filetype	filetype;
	mode_t		linkmode;
	int			stat_ok;
	int			linkok;
	char		acl_type;
	size_t		width;
}	t_file;

// directory using -R opt
typedef struct s_pending
{
	char				*name;
	char				*realname;
	int					cmdline_arg;
	struct s_pending	*next;
}	t_pending;

extern t_format				g_format;
extern t_sort_type			g_sort_type;
extern t_time_type			g_time_type;
extern t_indicator_style	g_indicator_style;
extern t_ignore_mode		g_ignore_mode;
extern t_color_when			g_color_when;

extern int					g_format_set;
extern int					g_sort_set;
extern int					g_explicit_time;

extern int					g_sort_reverse;
extern int					g_recursive;
extern int					g_immediate_dirs;
extern int					g_print_inode;
extern int					g_print_block_size;
extern int					g_human;
extern int					g_numeric_ids;
extern int					g_print_owner;
extern int					g_print_group;

extern int					g_print_with_color;
extern int					g_is_tty;
extern int					g_width_set;
extern size_t				g_line_length;

extern t_file				*g_cwd_file;
extern size_t				g_cwd_n_alloc;
extern size_t				g_cwd_n_used;
extern t_file				**g_sorted;
extern size_t				g_sorted_alloc;

// 0 = ok
// 1 = minor error
// 2 = error
extern int					g_exit_status;

extern t_pending			*g_pending_dirs;

int							decode_switches(int argc, char **argv);
size_t						term_width(void);
void						free_table(void);
void						print_dir(const char *name);
void						clear_files(void);
void						sort_files(void);
int							gobble_file(const char *name, unsigned char d_type,
								const char *dirname);

#endif
