#include <inquisitor.h>

int	main(int argc, char *argv[], char *env[]) {
	set_log_level(LEVEL_DEBUG);
	parse_env(env);

	t_args	args;
	if (parse_args(argc, argv, &args)) {
		ERR("Usage: %s <ip-src> <mac-src> <ip-dst> <mac-dst>\n", argv[0]);
		return (1);
	}
	capture(&args);
	return (0);
}