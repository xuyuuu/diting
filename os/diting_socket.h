#ifndef __diting_socket_h__
#define __diting_socket_h__

typedef uint32_t diting_socket_in_addr_t;
struct diting_socket_in_addr{
	diting_socket_in_addr_t s_addr;
};

struct diting_socket_sockaddr_in
{
	uint16_t sin_family;
	uint16_t sin_port;
	struct diting_socket_in_addr sin_addr;
	uint8_t sin_zero[sizeof(struct sockaddr)-
		(sizeof(uint16_t))-
		sizeof(uint16_t) -
		sizeof(struct diting_socket_in_addr)];
};

extern int diting_module_inside_socket_create(int family, int type,
		int protocol, int kern);
extern int diting_module_inside_socket_listen(struct socket *sock, int backlog);




#endif
