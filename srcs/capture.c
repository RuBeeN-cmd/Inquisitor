#include <inquisitor.h>
#include <pcap.h>

#define	FILTER_BUFF_SIZE 128

static void build_capture_filter(t_args *args, char buff[]) {
	uint32_t ip = ntohl(args->src_addr.s_addr);

	snprintf(buff, FILTER_BUFF_SIZE,
		"arp[14:4] = 0x%08x and "
		"arp[8:4] = 0x%02x%02x%02x%02x and "
		"arp[12:2] = 0x%02x%02x",
		ip,
		args->src_mac.ether_addr_octet[0],
		args->src_mac.ether_addr_octet[1],
		args->src_mac.ether_addr_octet[2],
		args->src_mac.ether_addr_octet[3],
		args->src_mac.ether_addr_octet[4],
		args->src_mac.ether_addr_octet[5]);
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

static pcap_t	*create_pcap_handle(t_args *args) {
	char	errbuf[PCAP_ERRBUF_SIZE] = { 0 };
	pcap_t	*handle = pcap_open_live(CAPTURE_DEVICE, BUFSIZ, 1, 1000, errbuf);
	
	if (!handle) {
		ERR("pcap_open_live(%s): %s\n", CAPTURE_DEVICE, errbuf);
		return (NULL);
	}

	char	filter_buffer[FILTER_BUFF_SIZE] = { 0 };
	build_capture_filter(args, filter_buffer);
	DBG("Capture Filter: %s\n", filter_buffer);

	if (apply_filter(handle, filter_buffer)) {
		ERR("Failed to apply filter\n");
		pcap_close(handle);
		return (NULL);
	}
	return (handle);
}

int	capture(t_args *args) {
	pcap_t	*handle = create_pcap_handle(args);
	if (!handle) {
		return (1);
	}
	pcap_close(handle);
	return (0);
}