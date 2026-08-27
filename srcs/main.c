#include "ft_ls.h"

int	main(int argc, char **argv)
{
	int	i;

	i = decode_switches(argc, argv);
	list_operands(argc, argv, i);
	free_table();
	free_pending();
	return (g_exit_status);
}