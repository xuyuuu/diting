#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <linux/netlink.h>

#include "diting_sockmsg.h"
#include "diting_signal.h"
#include "diting_logdump.h"

#define __USERSPACE__
#include "../os/diting_util.h"

static int diting_sockmsg_fd;

static int
diting_sockmsg_module_init(void)
{
	int ret = 0, on, fd = -1;
        struct sockaddr_nl addr;

	/*init logdump*/
	diting_logdump_module.init();
	diting_logdump_module.run();

#define DITING_SOCKMSG_PROTOCOL		30
        fd = socket(AF_NETLINK, SOCK_RAW, DITING_SOCKMSG_PROTOCOL);
        if(fd <= 0){
                ret = -1;
                goto err0;
        }

        memset(&addr, 0x0, sizeof(addr));
        addr.nl_family = AF_NETLINK;
        addr.nl_pid = getpid();
        addr.nl_groups = 0; 
        on = 1; 
        ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if(ret == -1){
                ret = -1;
                goto err0;
        }
        ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
        if(ret == -1){
		ret = -1;
                goto err0;
        }

        diting_sockmsg_fd = fd;
        return ret;

err0:
	diting_sockmsg_fd = -1;
        if(fd)
                close(fd);

	return ret;
}

static int 
diting_sockmsg_module_syn(void)
{
	int len, ret;
        struct sockaddr_nl addr;
        struct nlmsghdr nlh;
	memset(&nlh, 0x0, sizeof(nlh));
	len = sizeof(nlh);

        memset(&addr, 0x0, sizeof(addr));
        addr.nl_family = AF_NETLINK;
        addr.nl_pid     = 0; 
        addr.nl_groups = 0; 

        nlh.nlmsg_len = sizeof(struct nlmsghdr);
        nlh.nlmsg_seq = 0;
        nlh.nlmsg_pid = getpid();
        nlh.nlmsg_type = DITING_SOCKMSG_SYN;
        nlh.nlmsg_flags = 0;

        ret = sendto(diting_sockmsg_fd, &nlh, len, 0, (struct sockaddr *)&addr, sizeof(addr));
        if(ret < 0){
		ret = -1;
                goto out; 
        }

	ret = 0;
out:
	return ret;
}

int diting_sockmsg_module_inside_recvfromnlk(int fd)
{
	struct nlmsghdr *nlh = NULL;
	int len , ret, flag, datalen;
	char buffer[2048] = {0};

	struct sockaddr_nl addr;
	memset(&addr, 0x0, sizeof(addr));
	socklen_t addrlen = sizeof(addr);

	len = sizeof(struct nlmsghdr);
	flag = MSG_PEEK;

receive:
	nlh = (struct nlmsghdr *)realloc(nlh, len);
	if(!nlh){
		ret = -1;
		goto err;
	}
	if(recvfrom(fd, nlh, len, flag, (struct sockaddr *)&addr, &addrlen) < 0){
		ret = -1;
		goto err;
	}

	len = nlh->nlmsg_len;
	if(MSG_PEEK == flag){
		flag &= ~MSG_PEEK;
		goto receive;
	}

	if(addrlen != sizeof(addr)){
		ret = -1;
		goto err;
	}
	if(addr.nl_pid){
		ret = -1;
		goto err;
	}

	datalen = NLMSG_PAYLOAD(nlh, 0);
	if(datalen <= 0){
		ret = -1;	
		goto err;
	}

	struct diting_procrun_msgnode *procrun_item = NULL;
	struct diting_procaccess_msgnode *procaccess_item = NULL;
	struct diting_killer_msgnode *killer_item = NULL;

	switch(nlh->nlmsg_type){
	case DITING_PROCRUN:
		procrun_item = NLMSG_DATA(nlh);
		diting_logdump_module.push("%d,[uid:%d],[user:%s],[action: Run Program %s]", procrun_item->type, procrun_item->uid, 
				procrun_item->username, procrun_item->proc);
		break;
	case DITING_PROCACCESS:
		procaccess_item = NLMSG_DATA(nlh);
		if(DITING_PROCACCESS_INODE_CREATE == procaccess_item->actype){
			sprintf(buffer, "[uid:%d],[user:%s],[proc:%s],[action:Create File %s]",procaccess_item->uid,
				procaccess_item->username, procaccess_item->proc, procaccess_item->new_path);
		}else if(DITING_PROCACCESS_INODE_UNLINK == procaccess_item->actype){
			sprintf(buffer, "[uid:%d],[user:%s],[proc:%s],[action:Delete LinkFile %s]",procaccess_item->uid,
				procaccess_item->username, procaccess_item->proc, procaccess_item->new_path);
		}else if(DITING_PROCACCESS_INODE_LINK == procaccess_item->actype){
			sprintf(buffer, "[uid:%d],[user:%s],[proc:%s],[action:Create Hard LinkFile %s Point %s]",procaccess_item->uid,
				procaccess_item->username, procaccess_item->proc, procaccess_item->new_path, procaccess_item->old_path);
		}else if(DITING_PROCACCESS_INODE_SYMLINK == procaccess_item->actype){
			sprintf(buffer, "[uid:%d],[user:%s],[proc:%s],[action:Create Symbol LinkFile %s Point %s]",procaccess_item->uid,
				procaccess_item->username, procaccess_item->proc, procaccess_item->new_path, procaccess_item->old_path);
		}else if(DITING_PROCACCESS_INODE_RENAME == procaccess_item->actype){
			sprintf(buffer, "[uid:%d],[user:%s],[proc:%s],[action:Rename File %s To %s]",procaccess_item->uid,
				procaccess_item->username, procaccess_item->proc, procaccess_item->old_path, procaccess_item->new_path);
		}else if(DITING_PROCACCESS_INODE_MKDIR == procaccess_item->actype){
			sprintf(buffer, "[uid:%d],[user:%s],[proc:%s],[action:Mkdir Path %s]",procaccess_item->uid,
				procaccess_item->username, procaccess_item->proc, procaccess_item->new_path);
		}else if(DITING_PROCACCESS_INODE_RMDIR == procaccess_item->actype){
			sprintf(buffer, "[uid:%d],[user:%s],[proc:%s],[action:Rmdir Path %s]",procaccess_item->uid,
				procaccess_item->username, procaccess_item->proc, procaccess_item->new_path);
		}else if(DITING_PROCACCESS_INODE_ACCESS == procaccess_item->actype){
			sprintf(buffer, "[uid:%d],[user:%s],[proc:%s],[action:Access Path %s],[mode:%d]",procaccess_item->uid,
				procaccess_item->username, procaccess_item->proc, procaccess_item->new_path, procaccess_item->mode);
		}else if(DITING_PROCACCESS_FILE_ACCESS == procaccess_item->actype){
			sprintf(buffer, "[uid:%d],[user:%s],[proc:%s],[action:Access Path %s],[mode:%d]",procaccess_item->uid,
				procaccess_item->username, procaccess_item->proc, procaccess_item->new_path, procaccess_item->mode);
		}
		diting_logdump_module.push("%d,%s", procaccess_item->type, buffer);
		break;
	case DITING_KILLER:
		killer_item = NLMSG_DATA(nlh);
		diting_logdump_module.push("%d,[uid:%d],[user:%s],[action: '%s' Send Signal '%s' To '%s']", killer_item->type, killer_item->uid,
				killer_item->username, killer_item->proc1, killer_item->signal, killer_item->proc2);
		break;
	}

	ret = 0;
err:
	if(nlh)
		free(nlh);

	return ret;
}

static int
diting_sockmsg_module_loop(void)
{
	int nf = 0;
	fd_set rset, tmpset;
	struct timeval tvalue;

	memset(&tvalue, 0x0, sizeof(tvalue));
	tvalue.tv_sec = 10;
	tvalue.tv_usec = 0;
	FD_ZERO(&rset);	
	FD_ZERO(&tmpset);	
	FD_SET(diting_sockmsg_fd, &tmpset);

	while(0 == diting_signal_module.getstatus()){
		rset = tmpset;
		nf = select(diting_sockmsg_fd + 1, &rset, NULL, NULL, &tvalue);
		if(!nf){
			usleep(1000);
			continue;
		}else if(nf > 0)
			diting_sockmsg_module_inside_recvfromnlk(diting_sockmsg_fd);
	}

	return 0;
}

static int
diting_sockmsg_module_destroy(void)
{
	if(diting_sockmsg_fd > 0)
		close(diting_sockmsg_fd);

	return 0;
}


struct diting_sockmsg_module diting_sockmsg_module = {
	.init		= diting_sockmsg_module_init,
	.syn		= diting_sockmsg_module_syn,
	.loop		= diting_sockmsg_module_loop,
	.destroy	= diting_sockmsg_module_destroy,
};
