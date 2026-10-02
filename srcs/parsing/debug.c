#include <inquisitor.h>

void dbg_show_raw_args(int argc, char *argv[]) {
	DBG("---------- Raw Args ----------\n");
	DBG("argc = %d\n", argc);
	for (int i = 0; i < argc; i++) {
		DBG("argv[%d] = %s\n", i, argv[i]);
	}
}

void	dbg_show_args(t_args args) {
	DBG("------------ Args ------------\n");
	DBG("Source address:\t%s\n", inet_ntoa(args.src_addr));
	DBG("Dest address:\t%s\n", inet_ntoa(args.dst_addr));
}