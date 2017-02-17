#include <linux/version.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32)
#include "fs/mount.h"
#endif

#include <linux/list.h>
#include <linux/types.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/security.h>
#include <linux/mount.h>
#include <linux/fs_struct.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/net.h>
#include <asm/current.h>
#include <asm/signal.h>
#include <linux/dcache.h>
#include <linux/fs_struct.h>
#include <linux/stat.h>
#include <net/sock.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/pagemap.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
#include <linux/namespace.h>
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 18) && LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 32)
#include <linux/nsproxy.h>
#include <linux/mnt_namespace.h>
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 32)
#include <linux/nsproxy.h>
#endif

#include "diting_util.h"
#include "diting_access.h"
#include "diting_nolockqueue.h"

#include "diting_sysctl.h"
#include "diting_config.h"

int diting_dentry_has_permission(struct task_struct*task,struct dentry *new_dentry, 
		struct dentry *old_dentry, int mode, int type, const char *arg)
{
	uint32_t status = 0;
	struct diting_procaccess_msgnode *item;
	char username[64] = {0}, *old_fullpath = NULL, *old_name = NULL;
	char *new_fullpath = NULL, *new_name = NULL, *comm;

	comm = current->comm;

	diting_sysctl_module.chkstatus(DITING_ACCESSBEHAVIOR_SWITCH, &status);
	if(!status)
		return 0;

	new_fullpath = diting_common_get_name(task, &new_name, new_dentry, DITING_FULLFILE_ACCESS_TYPE);
	if(!new_fullpath || IS_ERR(new_fullpath))
		goto out;

	/*skip diting path*/
	if(!strncasecmp(new_fullpath, "/var/log/diting", sizeof("/var/log/diting") - 1))
		goto out;

	if(diting_config_module.search(DITING_PROCACCESS, new_fullpath))
		goto out;

	if(arg){
		old_fullpath = (char *)arg;
		goto skip;
	}
	if(old_dentry && !IS_ERR(old_dentry)){
		old_fullpath = diting_common_get_name(task, &old_name, old_dentry, DITING_FULLFILE_ACCESS_TYPE);
		if(!old_fullpath || IS_ERR(old_fullpath))
			goto out;
	}
skip:

	if(diting_common_getuser(current, username))
		strncpy(username, "SYSTEM", sizeof("SYSTEM") - 1);

	item = (struct diting_procaccess_msgnode *)kmalloc(\
			sizeof(struct diting_procaccess_msgnode), GFP_KERNEL);
	memset(item, 0x0, sizeof(struct diting_procaccess_msgnode));
	item->type = DITING_PROCACCESS;
	if(current->cred && !IS_ERR(current->cred))
		item->uid  = current->cred->uid;
	else
		item->uid = -1;
	strncpy(item->proc, comm, strlen(comm));
	strncpy(item->username, username, strlen(username));
	strncpy(item->new_path, new_fullpath, sizeof(item->new_path) - 1);
	if(old_fullpath)
		strncpy(item->old_path, old_fullpath, sizeof(item->old_path) - 1);
	item->actype = type;
	item->mode = mode;
	diting_nolockqueue_module.enqueue(diting_nolockqueue_module.getque(), item);

out:
	if(new_fullpath && !IS_ERR(new_fullpath))
		kfree(new_name);
	if(!arg && old_fullpath)
		kfree(old_name);

	return 0;
}
