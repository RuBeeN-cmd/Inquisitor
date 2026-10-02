#include "libft.h"

int ft_rand(void *dest, size_t size)
{
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0)
	{
		ft_printf_fd(2, "Failed to open /dev/urandom\n");
		return (1);
	}
	if (read(fd, dest, size) < 0)
	{
		ft_printf_fd(2, "Failed to read /dev/urandom\n");
		close(fd);
		return (1);
	}
	close(fd);
	return (0);
}