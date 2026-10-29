#include <inquisitor.h>

int	parse_args(int argc, char *argv[], t_args *args) {
	dbg_show_raw_args(argc, argv);

	args->write_dir = NULL;

	char	*pos[4];
	int		np = 0;
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-' && argv[i][1] == 'w' && argv[i][2] == '\0') {
			if (i + 1 >= argc) {
				ERR("%s: -w requires a directory argument.\n", argv[0]);
				return (1);
			}
			args->write_dir = argv[++i];
		} else if (np < 4) {
			pos[np++] = argv[i];
		} else {
			ERR("%s: too many arguments.\n", argv[0]);
			return (1);
		}
	}
	if (np < 4) {
		ERR("%s: too few arguments.\n", argv[0]);
		return (1);
	}

	if (!inet_aton(pos[0], &args->src_addr)) {
		ERR("%s: invalid source address: `%s`\n", argv[0], pos[0]);
		return (1);
	}
	if (ether_aton_r(pos[1], &args->src_mac) == NULL) {
		ERR("%s: invalid source MAC: `%s`\n", argv[0], pos[1]);
		return (1);
	}
	if (!inet_aton(pos[2], &args->dst_addr)) {
		ERR("%s: invalid destination address: `%s`\n", argv[0], pos[2]);
		return (1);
	}
	if (ether_aton_r(pos[3], &args->dst_mac) == NULL) {
		ERR("%s: invalid destination MAC: `%s`\n", argv[0], pos[3]);
		return (1);
	}

	dbg_show_args(args);
	return (0);
}