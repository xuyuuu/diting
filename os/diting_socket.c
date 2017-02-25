#include <linux/types.h>
#include <linux/fs.h>
#include <linux/security.h> 
#include <linux/mount.h> 
#include <linux/list.h> 
#include <linux/fs_struct.h> 
#include <linux/sched.h>
#include <linux/net.h>
#include <asm/current.h> 
#include <asm/signal.h>
#include <linux/dcache.h> 
#include <linux/fs_struct.h>
#include <linux/stat.h>
#include <linux/version.h>
#include <net/sock.h>

#include "diting_util.h"
#include "diting_nolockqueue.h"

#define DITING_SOCKET_FAMILY_MAX		37
static char *diting_socket_family[] = {
	"AF_UNSPEC", "AF_LOCAL", "AF_INET", "AF_AX25", "AF_IPX",
	"AF_APPLETALK", "AF_NETROM", "AF_BRIDGE", "AF_ATMPVC", "AF_X25",
	"AF_INET6", "AF_ROSE", "AF_DECnet", "AF_NETBEUI", "AF_SECURITY",
	"AF_KEY", "AF_NETLINK", "AF_PACKET", "AF_ASH", "AF_ECONET"
	"AF_ATMSVC", "AF_RDS", "AF_SNA", "AF_IRDA", "AF_PPPOX",
	"AF_WANPIPE", "AF_LLC", "", "", "AF_CAN", 
	"AF_TIPC", "AF_BLUETOOTH", "AF_IUCV", "AF_RXRPC", "AF_ISDN",
	"AF_PHONET", "AF_IEEE802154"

};

#define DITING_SOCKET_TYPE_MAX		11
static char *diting_socket_type[] = {
	"", "SOCK_STREAM", "SOCK_DGRAM", "SOCK_RAW", "SOCK_RDM",
	"SOCK_SEQPACKET", "SOCK_DCCP", "", "", "", "SOCK_PACKET"
};

int diting_module_inside_socket_create(int family, int type, int protocol, int kern)
{
	char username[64] = {0};
	struct diting_socket_msgnode *item;
	item = (struct diting_socket_msgnode *)kmalloc(\
			sizeof(struct diting_socket_msgnode), GFP_KERNEL);
	memset(item, 0x0, sizeof(struct diting_socket_msgnode));

	if(current->cred && !IS_ERR(current->cred))
		item->uid  = current->cred->uid;
	else
		item->uid = -1;
	if(diting_common_getuser(current, username))
		strncpy(username, "SYSTEM", sizeof("SYSTEM") - 1);
	strncpy(item->username, username, strlen(username));
	item->type = DITING_SOCKET;
	item->actype = DITING_SOCKET_CREATE;
	if((family > DITING_SOCKET_FAMILY_MAX) || (type > DITING_SOCKET_TYPE_MAX))
		goto out;
	strncpy(item->sockfamily, diting_socket_family[family], strlen(diting_socket_family[family]));
	strncpy(item->socktype, diting_socket_type[type], strlen(diting_socket_type[type]));

	//diting_nolockqueue_module.enqueue(diting_nolockqueue_module.getque(), item);		
	kfree(item);
out:
	return 0;
}


int diting_module_inside_socket_listen(struct socket *sock, int backlog)
{
	int family, type;
	char username[64] = {0};
	struct diting_socket_msgnode *item;
	item = (struct diting_socket_msgnode *)kmalloc(\
			sizeof(struct diting_socket_msgnode), GFP_KERNEL);
	memset(item, 0x0, sizeof(struct diting_socket_msgnode));

	if(current->cred && !IS_ERR(current->cred))
		item->uid  = current->cred->uid;
	else
		item->uid = -1;
	if(diting_common_getuser(current, username))
		strncpy(username, "SYSTEM", sizeof("SYSTEM") - 1);
	strncpy(item->username, username, strlen(username));
	item->type = DITING_SOCKET;
	item->actype = DITING_SOCKET_LISTEN;
	if(!sock->ops || IS_ERR(sock->ops) || (family > DITING_SOCKET_FAMILY_MAX) || (type > DITING_SOCKET_TYPE_MAX))
		goto out;
	family = sock->ops->family;
	type = sock->type;
	strncpy(item->sockfamily, diting_socket_family[family], strlen(diting_socket_family[family]));
	strncpy(item->socktype, diting_socket_type[type], strlen(diting_socket_type[type]));

	//diting_nolockqueue_module.enqueue(diting_nolockqueue_module.getque(), item);
	kfree(item);
out:
	return 0;
}


