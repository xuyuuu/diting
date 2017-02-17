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
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32)
#include <linux/binfmts.h>
#endif

#include "diting_bprm.h"
#include "diting_util.h"
#include "diting_nolockqueue.h"

#include "diting_config.h"
#include "diting_sysctl.h"

static char *diting_bprm_inside_get_name(struct linux_binprm *bprm, char *kbuf)
{
	struct file *file = bprm->file;
	char *rtnstr = NULL;

#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 18)
        struct dentry * tmp_dentry = file->f_path.f_dentry;
	struct vfsmount *vfsmnt = file->f_path.f_vfsmnt;

        if (IS_ERR(tmp_dentry) || !(tmp_dentry))
		return NULL;

        if (IS_ERR(vfsmnt) || !(vfsmnt))
		return NULL;

	rtnstr = d_path(tmp_dentry, vfsmnt, kbuf, PAGE_SIZE);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32) 
	path_get(&file->f_path);
	rtnstr = d_path(&file->f_path, kbuf, PAGE_SIZE);
	path_put(&file->f_path);
#endif

	return rtnstr;
}

static char * diting_bprm_get_name(struct linux_binprm *bprm, char **name)
{
	char *p = NULL;
        *name = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (NULL == (*name) || IS_ERR(*name))
		return NULL;

        p = diting_bprm_inside_get_name(bprm, *name);
	if (!p || IS_ERR(p)){
		kfree(*name);
		p = NULL;
	}

        return p;
}


int diting_inside_bprm_check_security(struct linux_binprm *bprm)
{
	uint32_t status = 0;
	struct diting_procrun_msgnode *item;
	char *realpath = NULL, *name = NULL, username[64] = {0};
	struct dentry *dentry = bprm->file->f_dentry;

	if(!dentry || IS_ERR(dentry))
		return 0;

	diting_sysctl_module.chkstatus(DITING_PROCBEHAVIOR_SWITCH, &status);
	if(!status)
		return 0;


	realpath = diting_bprm_get_name(bprm, &name);
	if(!realpath || IS_ERR(realpath))
		return 0;

	if(!diting_config_module.search(DITING_PROCRUN, realpath))
		goto out;

	if(diting_common_getuser(current, username))
		strncpy(username, "SYSTEM", sizeof("SYSTEM") - 1);

	item = (struct diting_procrun_msgnode *)kmalloc(\
			sizeof(struct diting_procrun_msgnode), GFP_KERNEL);
	memset(item, 0x0, sizeof(struct diting_procrun_msgnode));
	item->type = DITING_PROCRUN;
	if(current->cred && !IS_ERR(current->cred))
		item->uid  = current->cred->uid;
	else
		item->uid = -1;
	strncpy(item->username, username, strlen(username));
	strncpy(item->proc, realpath, sizeof(item->proc) - 1);
	diting_nolockqueue_module.enqueue(diting_nolockqueue_module.getque(), item);	

out:
	if(name && !IS_ERR(name))
		kfree(name);

	return 0;
}

