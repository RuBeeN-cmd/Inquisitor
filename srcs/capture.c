#include <inquisitor.h>
#include <pcap.h>

#define	FILTER_BUFF_SIZE 1024

static int	build_capture_filter(struct in_addr src_addr) {
	char	filter[1024] = "arp.src.proto_ipv4 == ";
	int		i = ft_strlen(filter);
	
	char	*str_src_addr = inet_ntoa(src_addr);
	int		addr_len = ft_strlen(str_src_addr);
	ft_strlcpy(filter + i, str_src_addr, FILTER_BUFF_SIZE);
	i += addr_len;

	DBG("Capture filter: %s\n", filter);
	return (0);
}

static pcap_t	*create_pcap_handle(t_args *args) {
	char	errbuf[PCAP_ERRBUF_SIZE] = { 0 };
	pcap_t	*handle = pcap_open_live(CAPTURE_DEVICE, BUFSIZ, 1, 1000, errbuf);
	
	if (!handle) {
		ERR("pcap_open_live(%s): %s\n", CAPTURE_DEVICE, errbuf);
		return (NULL);
	}

	build_capture_filter(args->src_addr);

	return (handle);
}

int	capture(t_args *args) {
	pcap_t	*handle = create_pcap_handle(args);
	pcap_close(handle);
	return (0);
}