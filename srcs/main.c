#include "ft_ls.h"

int	main(int argc, char **argv)
{
	int	i;
	int	n_files;

	i = decode_switches(argc, argv);
	n_files = argc - i;
	if (n_files == 0)
		print_dir(".");
	else
		print_dir(argv[i]);
	free_table();
	return (g_exit_status);
}