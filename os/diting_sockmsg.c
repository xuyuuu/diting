#include <linux/module.h>
#include <linux/security.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kallsyms.h>
#include <linux/list.h>
#include <linux/version.h>
#include <net/sock.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/pid.h>

#include <net/net_namespace.h>
#include <linux/netlink.h>

#include "diting_sockmsg.h"
#include "diting_util.h"

static pid_t diting_userspace_pid;
static struct sock * diting_kernelspace_sk;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
static void diting_sockmsg_module_inside_msgrecv(struct sock *sk, int len)
#else
static void diting_sockmsg_module_inside_msgrecv(struct sk_buff *skb)
#endif
{
        struct nlmsghdr *nlh;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
        int rc = 0;
        struct sk_buff *skb;
again:
        skb = skb_recv_datagram(sk, 0, 0, &rc);
        if (rc == -EINTR)
                goto again;
        else if(rc < 0)
                return;
#endif

        nlh = (struct nlmsghdr *)skb->data;
        if (!NLMSG_OK(nlh, skb->len))
                goto out;

	if(DITING_SOCKMSG_SYN == nlh->nlmsg_type){
		diting_userspace_pid = NETLINK_CREDS(skb)->pid;
	}
out:

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
        if(skb && !IS_ERR(skb))
                skb_free_datagram(sk, skb);
#endif

        return;
}

static int
diting_sockmsg_module_init(void)
{
	diting_userspace_pid = 0;

        /*netlink socket channel*/
#define DITING_SOCKMSG_PROTOCOL                30
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
        diting_kernelspace_sk = netlink_kernel_create(DITING_SOCKMSG_PROTOCOL, 0, diting_sockmsg_module_inside_msgrecv, THIS_MODULE);
#elif (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 18) && LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 32))
        diting_kernelspace_sk = netlink_kernel_create(&init_net, DITING_SOCKMSG_PROTOCOL, 0, diting_sockmsg_module_inside_msgrecv, NULL, THIS_MODULE);
#elif (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 32))
        struct netlink_kernel_cfg nelkcfg;
        memset(&nelkcfg, 0x00, sizeof(nelkcfg));
        nelkcfg.input = diting_sockmsg_module_inside_msgrecv;
        diting_kernelspace_sk = netlink_kernel_create(&init_net, DITING_SOCKMSG_PROTOCOL, &nelkcfg);
#endif

	diting_kernelspace_sk->sk_sndtimeo = 10;

	return 0;
}

static int
diting_sockmsg_module_sendlog(void *data, int datalen, int type)
{
	struct sk_buff *skb;
        struct nlmsghdr *nlh;
        char *pmsg;
        size_t len;
        int rc = 0;

        if (!diting_kernelspace_sk || IS_ERR(diting_kernelspace_sk) || !diting_userspace_pid)
                return -1; 

        len = ((data && datalen) ? datalen : 0); 
        skb = alloc_skb(NLMSG_SPACE(len), GFP_KERNEL);
        if (!skb){
                rc = -ENOMEM;
                goto out;
        }

        nlh = NLMSG_PUT(skb, diting_userspace_pid, 0, type, len);
        if(len){
                pmsg = (char *)NLMSG_DATA(nlh);
                memcpy(pmsg, data, datalen);
        }
        rc = netlink_unicast(diting_kernelspace_sk, skb, diting_userspace_pid, 0); 
        if (rc < 0)
                goto out;

        rc = 0;
	goto out;
nlmsg_failure:
        kfree_skb(skb);
out:
        return rc;
}

static int
diting_sockmsg_module_destroy(void)
{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
        if(diting_kernelspace_sk && diting_kernelspace_sk->sk_socket)
                sock_release(diting_kernelspace_sk->sk_socket);
#else
        if(diting_kernelspace_sk)
                netlink_kernel_release(diting_kernelspace_sk);
#endif		
	return 0;
}


struct diting_sockmsg_module diting_sockmsg_module = {
	.init		= diting_sockmsg_module_init,
	.sendlog	= diting_sockmsg_module_sendlog,
	.destroy	= diting_sockmsg_module_destroy
};
