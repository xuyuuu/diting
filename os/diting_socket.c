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
#include <linux/in.h>
#include <net/sock.h>
#include <linux/byteorder/generic.h>
#include <net/inet_sock.h>

#include "diting_util.h"
#include "diting_nolockqueue.h"
#include "diting_socket.h"

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
	char username[64] = {0}, *path = NULL, *name = NULL;
	struct diting_socket_msgnode *item;

	if((family > DITING_SOCKET_FAMILY_MAX) || (type > DITING_SOCKET_TYPE_MAX))
		goto out;

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
	item->pid = current->pid;
	item->type = DITING_SOCKET;
	item->actype = DITING_SOCKET_CREATE;
	path = diting_common_get_name(current, &name, NULL, DITING_FULLFILE_TASK_TYPE);
	if(path && !IS_ERR(path)){
		strncpy(item->proc, path, strlen(path));	
		kfree(name);
	}

	strncpy(item->sockfamily, diting_socket_family[family], strlen(diting_socket_family[family]));
	strncpy(item->socktype, diting_socket_type[type], strlen(diting_socket_type[type]));

	diting_nolockqueue_module.enqueue(diting_nolockqueue_module.getque(), item);
out:
	return 0;
}


int diting_module_inside_socket_listen(struct socket *sock, int backlog)
{
	struct sock *sk = NULL;
	struct inet_sock *inet = NULL;
	int family, type;
	char username[64] = {0}, *path = NULL, *name = NULL;
	struct diting_socket_msgnode *item;

	if(!sock || IS_ERR(sock))
		goto out;
	sk = sock->sk;
	if(!sk || IS_ERR(sk))
		goto out;
	inet = (struct inet_sock *)sk;
	if(!inet || IS_ERR(inet))
		goto out;

	if(!sock->ops || IS_ERR(sock->ops))
		goto out;

	family = sock->ops->family;
	type = sock->type;

	if((family > DITING_SOCKET_FAMILY_MAX) || (type > DITING_SOCKET_TYPE_MAX))
		goto out;

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
	item->pid = current->pid;
	item->type = DITING_SOCKET;
	item->actype = DITING_SOCKET_LISTEN;
	strncpy(item->sockfamily, diting_socket_family[family], strlen(diting_socket_family[family]));
	strncpy(item->socktype, diting_socket_type[type], strlen(diting_socket_type[type]));
	item->localport = inet->num;
	item->localaddr = inet->saddr;

	path = diting_common_get_name(current, &name, NULL, DITING_FULLFILE_TASK_TYPE);
	if(path && !IS_ERR(path)){
		strncpy(item->proc, path, strlen(path));	
		kfree(name);
	}

	diting_nolockqueue_module.enqueue(diting_nolockqueue_module.getque(), item);
out:
	return 0;
}


int diting_module_inside_socket_connect(struct socket *sock, struct sockaddr *address, int addrlen)
{
	struct sock *sk = NULL;
	int family, type;
	char username[64] = {0}, *path = NULL, *name = NULL;
	struct diting_socket_msgnode *item;
	struct sockaddr_in *sockaddr = NULL;

	if(!sock || IS_ERR(sock))
		goto out;
	sk = sock->sk;
	if(!sk || IS_ERR(sk))
		goto out;

	if(!sock->ops || IS_ERR(sock->ops))
		goto out;

	if(!address || IS_ERR(address))
		goto out;
	sockaddr = (struct sockaddr_in *)address;

	family = sock->ops->family;
	type = sock->type;

	if((family > DITING_SOCKET_FAMILY_MAX) || (type > DITING_SOCKET_TYPE_MAX))
		goto out;

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
	item->pid = current->pid;
	item->type = DITING_SOCKET;
	item->actype = DITING_SOCKET_CONNECT;
	strncpy(item->sockfamily, diting_socket_family[family], strlen(diting_socket_family[family]));
	strncpy(item->socktype, diting_socket_type[type], strlen(diting_socket_type[type]));
	item->remoteport = sockaddr->sin_port;
	item->remoteaddr = sockaddr->sin_addr.s_addr;

	path = diting_common_get_name(current, &name, NULL, DITING_FULLFILE_TASK_TYPE);
	if(path && !IS_ERR(path)){
		strncpy(item->proc, path, strlen(path));	
		kfree(name);
	}

	diting_nolockqueue_module.enqueue(diting_nolockqueue_module.getque(), item);
out:
	return 0;
}


int diting_module_inside_socket_sendmsg(struct socket *sock, struct msghdr *msg, int size)
{
	struct sock *sk = NULL;
	struct inet_sock *inet = NULL;
	int family, type;
	char username[64] = {0}, *path = NULL, *name = NULL;
	struct diting_socket_msgnode *item;

	if(!sock || IS_ERR(sock))
		goto out;
	sk = sock->sk;
	if(!sk || IS_ERR(sk))
		goto out;
	inet = (struct inet_sock *)sk;
	if(!inet || IS_ERR(inet))
		goto out;

	if(!sock->ops || IS_ERR(sock->ops))
		goto out;

	family = sock->ops->family;
	type = sock->type;

	if((family > DITING_SOCKET_FAMILY_MAX) || (type > DITING_SOCKET_TYPE_MAX))
		goto out;

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
	item->pid = current->pid;
	item->type = DITING_SOCKET;
	item->actype = DITING_SOCKET_SENDMSG;
	strncpy(item->sockfamily, diting_socket_family[family], strlen(diting_socket_family[family]));
	strncpy(item->socktype, diting_socket_type[type], strlen(diting_socket_type[type]));
	item->localport = inet->num;
	item->localaddr = inet->saddr;
	item->remoteport = ntohs(inet->dport);
	item->remoteaddr = inet->daddr;

	if(inet->num == 0 || inet->num == 0x16 || 
			inet->num == 0x3500 || inet->dport == 0x3500){
		kfree(name);
		goto out;	
	}

	path = diting_common_get_name(current, &name, NULL, DITING_FULLFILE_TASK_TYPE);
	if(path && !IS_ERR(path)){
		strncpy(item->proc, path, strlen(path));
		kfree(name);
	}

	diting_nolockqueue_module.enqueue(diting_nolockqueue_module.getque(), item);
out:
	return 0;
}


int diting_module_inside_socket_recvmsg(struct socket *sock, struct msghdr *msg, int size, int flags)
{
	struct sock *sk = NULL;
	struct inet_sock *inet = NULL;
	int family, type;
	char username[64] = {0}, *path = NULL, *name = NULL;
	struct diting_socket_msgnode *item;

	if(!sock || IS_ERR(sock))
		goto out;
	sk = sock->sk;
	if(!sk || IS_ERR(sk))
		goto out;
	inet = (struct inet_sock *)sk;
	if(!inet || IS_ERR(inet))
		goto out;

	if(!sock->ops || IS_ERR(sock->ops))
		goto out;

	family = sock->ops->family;
	type = sock->type;

	if((family > DITING_SOCKET_FAMILY_MAX) || (type > DITING_SOCKET_TYPE_MAX))
		goto out;

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
	item->pid = current->pid;
	item->type = DITING_SOCKET;
	item->actype = DITING_SOCKET_RECVMSG;
	strncpy(item->sockfamily, diting_socket_family[family], strlen(diting_socket_family[family]));
	strncpy(item->socktype, diting_socket_type[type], strlen(diting_socket_type[type]));
	item->localport = inet->num;
	item->localaddr = inet->saddr;
	item->remoteport = ntohs(inet->dport);
	item->remoteaddr = inet->daddr;

	if(inet->num == 0 || inet->num == 0x16 || 
			inet->num == 0x3500 || inet->dport == 0x3500){
		kfree(name);
		goto out;
	}

	path = diting_common_get_name(current, &name, NULL, DITING_FULLFILE_TASK_TYPE);
	if(path && !IS_ERR(path)){
		strncpy(item->proc, path, strlen(path));	
		kfree(name);
	}

	diting_nolockqueue_module.enqueue(diting_nolockqueue_module.getque(), item);
out:
	return 0;
}


