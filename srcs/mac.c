#include <inquisitor.h>
#include <libft.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>

int get_mac(const char *ifname, struct ether_addr *mac) {
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) return -1;

	struct ifreq ifr = {0};
	ft_strlcpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

	if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
		close(fd);
		return -1;
	}
	close(fd);

	ft_memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
	return 0;
}