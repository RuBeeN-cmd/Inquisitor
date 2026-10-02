#ifndef INQUISITOR_H
#define INQUISITOR_H

#include <netinet/ether.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <utils/log.h>

#define CAPTURE_DEVICE "any"

typedef struct	s_args {
	struct in_addr		src_addr;
	struct ether_addr	src_mac;
	struct in_addr		dst_addr;
	struct ether_addr	dst_mac;
}				t_args;

// debug.c
void	dbg_show_raw_args(int argc, char *argv[]);
void	dbg_show_args(t_args *args);

// parsing.c
int	parse_args(int argc, char *argv[], t_args *args);

// capture.c
int	capture(t_args *args);

#endif