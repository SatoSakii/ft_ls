#include "ft_ls.h"

int	main(int argc, char **argv)
{
	int	i;
	int	n_files;

	i = decode_switches(argc, argv);
	n_files = argc - i;
	(void)n_files;
	return (0);
}