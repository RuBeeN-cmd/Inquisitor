#include <inquisitor.h>
#include <pcap.h>

int	capture() {
	char errbuf[PCAP_ERRBUF_SIZE] = { 0 };
	pcap_t *handle = pcap_open_live(CAPTURE_DEVICE, BUFSIZ, 1, 1000, errbuf);
	if (!handle) {
		ERR("pcap_open_live(%s): %s\n", CAPTURE_DEVICE, errbuf);
		return (1);
	}
	pcap_close(handle);
	return (0);
}