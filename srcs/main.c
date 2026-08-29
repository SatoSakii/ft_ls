#include "ft_ls.h"

int	main(int argc, char **argv)
{
	int	i;

	i = decode_switches(argc, argv);
	color_init();
	list_operands(argc, argv, i);
	free_table();
	free_columns();
	free_pending();
	free_colors();
	return (g_exit_status);
}