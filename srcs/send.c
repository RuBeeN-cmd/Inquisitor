#include <inquisitor.h>

libnet_ptag_t	build_arp_reply(struct ether_addr *src_mac, struct in_addr *src_addr, struct ether_addr *dst_mac, struct in_addr *dst_addr, libnet_t *l) {
	libnet_ptag_t arp = libnet_build_arp(
		ARPHRD_ETHER, ETHERTYPE_IP, 6, 4,
		ARPOP_REPLY,
		src_mac->ether_addr_octet, (uint8_t *)src_addr,
		dst_mac->ether_addr_octet, (uint8_t *)dst_addr,
		NULL, 0, l, 0);

	libnet_build_ethernet(
		dst_mac->ether_addr_octet,
		src_mac->ether_addr_octet,
		ETHERTYPE_ARP, NULL, 0, l, 0);
	return arp;
}

void	destroy_arp_reply(libnet_t *l) {
	libnet_destroy(l);
}

int	send_arp_reply(libnet_t *l) {
	if (libnet_write(l) == -1) {
		ERR("Failed to send ARP reply.\n");
		return (1);
	}
	return (0);
}

int	send_arp(libnet_t *l, char target[]) {
	DBG("Sending %sARP%s Reply to %s%s%s\n",
		get_color(COLOR_GREEN), get_color(COLOR_RESET),
		get_color(COLOR_BLUE), target, get_color(COLOR_RESET));
	
	if (send_arp_reply(l)) {
		WARN("Failed to send ARP Reply to %s\n", target);
		return (1);
	}
	return (0);
}
