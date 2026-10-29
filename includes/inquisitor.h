#ifndef INQUISITOR_H
#define INQUISITOR_H

#include <netinet/ether.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libnet.h>

#include <utils/log.h>

#define CAPTURE_DEVICE	"eth0"
#define IFACE_NAME		"eth0"

typedef struct	s_args {
	struct in_addr		src_addr;
	struct ether_addr	src_mac;
	struct in_addr		dst_addr;
	struct ether_addr	dst_mac;
	const char			*write_dir;		// -w <dir>: dump transferred files, NULL if off
}				t_args;

// debug.c
void	dbg_show_raw_args(int argc, char *argv[]);
void	dbg_show_args(t_args *args);

// parsing.c
int	parse_args(int argc, char *argv[], t_args *args);

// capture.c
int	capture(t_args *args, libnet_t *l_client, libnet_t *l_server);

// mac.c
int	get_mac(const char *ifname, struct ether_addr *mac);

// send.c
libnet_ptag_t	build_arp_reply(struct ether_addr *src_mac, struct in_addr *src_addr, struct ether_addr *dst_mac, struct in_addr *dst_addr, libnet_t *l);
void			destroy_arp_reply(libnet_t *l);
int				send_arp(libnet_t *l, char target[]);

#endif