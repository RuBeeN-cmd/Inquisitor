#include <inquisitor.h>

void dbg_show_raw_args(int argc, char *argv[]) {
	DBG("---------- Raw Args ----------\n");
	DBG("argc = %d\n", argc);
	for (int i = 0; i < argc; i++) {
		DBG("argv[%d] = %s\n", i, argv[i]);
	}
}

void	dbg_show_args(t_args *args) {
	DBG("------------ Args ------------\n");
	DBG("Source address:\t%s\n", inet_ntoa(args->src_addr));
	DBG("Source MAC:\t%02x:%02x:%02x:%02x:%02x:%02x\n",
		args->src_mac.ether_addr_octet[0],
		args->src_mac.ether_addr_octet[1],
		args->src_mac.ether_addr_octet[2],
		args->src_mac.ether_addr_octet[3],
		args->src_mac.ether_addr_octet[4],
		args->src_mac.ether_addr_octet[5]);
	DBG("Dest address:\t%s\n", inet_ntoa(args->dst_addr));
	DBG("Dest MAC:\t%02x:%02x:%02x:%02x:%02x:%02x\n",
		args->dst_mac.ether_addr_octet[0],
		args->dst_mac.ether_addr_octet[1],
		args->dst_mac.ether_addr_octet[2],
		args->dst_mac.ether_addr_octet[3],
		args->dst_mac.ether_addr_octet[4],
		args->dst_mac.ether_addr_octet[5]);
}