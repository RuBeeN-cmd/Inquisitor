#include <inquisitor.h>

int	parse_args(int argc, char *argv[], t_args *args) {
	dbg_show_raw_args(argc, argv);

	if (argc < 5) {
		ERR("%s: too few arguments.\n", argv[0]);
		return (1);
	}
	if (argc > 5) {
		ERR("%s: too many arguments.\n", argv[0]);
		return (1);
	}

	if (!inet_aton(argv[1], &args->src_addr)) {
		ERR("%s: invalid source address: `%s`\n", argv[0], argv[1]);
		return (1);
	}
	if (ether_aton_r(argv[2], &args->src_mac) == NULL) {
		ERR("%s: invalid source MAC: `%s`\n", argv[0], argv[2]);
		return (1);
	}
	if (!inet_aton(argv[3], &args->dst_addr)) {
		ERR("%s: invalid destination address: `%s`\n", argv[0], argv[3]);
		return (1);
	}
	if (ether_aton_r(argv[4], &args->dst_mac) == NULL) {
		ERR("%s: invalid destination MAC: `%s`\n", argv[0], argv[4]);
		return (1);
	}

	dbg_show_args(args);
	return (0);
}