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

int diting_dentry_has_permission(struct task_struct*task,struct dentry *new_dentry, struct dentry *old_dentry, int mode, int type)
{
	char *fullpath = NULL, *comm, *name, *dstpath = NULL;
	int result = 0, found = 0;

	comm = task->comm;

	fullpath = diting_common_get_name(task, &name, new_dentry, DITING_FULLFILE_ACCESS_TYPE);
	if(!fullpath || IS_ERR(fullpath))
		goto out;

out:
	if(fullpath && !IS_ERR(fullpath))
		kfree(name);

	return 0;
}
