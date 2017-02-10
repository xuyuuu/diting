#include <linux/time.h>
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
#include <net/sock.h>
#include <linux/hardirq.h>
#include <linux/version.h>
#include <linux/namei.h>

#include "diting_util.h"
#include "diting_door.h"
#include "diting_bprm.h"
#include "diting_access.h"

/* old hook function point */
static int (* old_bprm_check_security)(struct linux_binprm *bprm);
static int ( *old_file_permission)(struct file *file, int mask);
#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 18)
int (*old_inode_permission)(struct inode *inode, int mask, struct nameidata *nd);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32) 
static int (*old_inode_permission)(struct inode *inode, int mask);
#endif
static int (*old_inode_create) (struct inode *dir, struct dentry *dentry, int mode);
static int (* old_inode_unlink)(struct inode *dir, struct dentry *dentry);
static int (* old_inode_link)(struct dentry *old_dentry, struct inode *dir, struct dentry *new_dentry);
static int (* old_inode_symlink)(struct inode *dir, struct dentry *dentry, const char *old_name);
static int ( * old_inode_rename)(struct inode *old_inode,
		struct dentry *old_dentry, struct inode *new_inode, struct dentry *new_dentry);
static int (* old_inode_mkdir)(struct inode *inode, struct dentry *dentry, int mask);
static int (* old_inode_rmdir)(struct inode *dir, struct dentry *dentry);


static int diting_module_inside_inode_rmdir(struct inode *dir, struct dentry *dentry)
{
	diting_dentry_has_permission(current, dentry, NULL, 0, DITING_PROCACCESS_INODE_RMDIR);

	return 0;
}

static int diting_module_inside_inode_mkdir(struct inode *inode, struct dentry *dentry, int mask)
{
	diting_dentry_has_permission(current, dentry, NULL, mask, DITING_PROCACCESS_INODE_MKDIR);

	return 0;
}

static int diting_module_inside_inode_rename(struct inode *old_inode, struct dentry *old_dentry, 
		struct inode * new_inode, struct dentry *new_dentry)
{
	diting_dentry_has_permission(current, new_dentry, old_dentry, 0, DITING_PROCACCESS_INODE_RENAME);

	return 0;
}

static int diting_module_inside_inode_symlink(struct inode *dir, struct dentry *dentry, const char *old_name)
{
	diting_dentry_has_permission(current, dentry, NULL, 0, DITING_PROCACCESS_INODE_SYMLINK);

	return 0;
}

static int diting_module_inside_inode_link(struct dentry *old_dentry, struct inode *dir, struct dentry *new_dentry)
{
	diting_dentry_has_permission(current, new_dentry, old_dentry, 0, DITING_PROCACCESS_INODE_LINK);

	return 0;
}

static int diting_module_inside_inode_unlink(struct inode *dir, struct dentry *dentry)
{
	diting_dentry_has_permission(current, dentry, NULL, 0, DITING_PROCACCESS_INODE_UNLINK);

	return 0;
}

static int diting_module_inside_inode_create(struct inode *dir, struct dentry *dentry, int mode)
{
	diting_dentry_has_permission(current, dentry, NULL, mode, DITING_PROCACCESS_INODE_CREATE);

	return 0;
}

#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 18)
static int diting_module_inside_inode_permission(struct inode *inode, int mask, struct nameidata *nd)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
static int diting_module_inside_inode_permission(struct inode *inode, int mask)
#endif
{
	struct dentry *dentry = NULL;

	if(!inode || IS_ERR(inode) || !S_ISREG(inode->i_mode))
		goto out;

#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 18)
	if (!nd || IS_ERR(nd))
		goto out;
	dentry = nd->dentry;
	dget(dentry);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
	dentry = d_find_alias(inode);
#endif

	if(!dentry || IS_ERR(dentry))
		goto out;

	diting_dentry_has_permission(current, dentry, NULL, mask, DITING_PROCACCESS_INODE_ACCESS);

out:
	if(dentry)
		dput(dentry);

	return 0;
}

static int diting_module_inside_file_permission(struct file *file, int mask)
{
	if(!file->f_dentry || IS_ERR(file->f_dentry))
		goto out;

	diting_dentry_has_permission(current, file->f_dentry, NULL, mask, DITING_PROCACCESS_FILE_ACCESS);

out:
	return 0;
}

static int diting_module_inside_bprm_check_security(struct linux_binprm *bprm)
{
	return diting_inside_bprm_check_security(bprm);
}

#ifndef __64bit__
static ulong diting_door_module_bitopen(void)
{
        ulong cr0 = 0;
        ulong ret;
        asm volatile ("movl %%cr0, %%eax"
                        : "=a"(cr0)
                     );  
        ret = cr0;
        /* clear the 20 bit of CR0, a.k.a WP bit */
        cr0 &= 0xfffeffff;
        asm volatile ("movl %%eax, %%cr0"
                        :   
                        : "a"(cr0)
                     );  
        return ret;
}
#else
static ulong diting_door_module_bitopen(void)
{
        ulong cr0 = 0;
        ulong ret;
        asm volatile ("movq %%cr0, %%rax"
                        : "=a"(cr0)
                     );  
        ret = cr0;
        /* clear the 20 bit of CR0, a.k.a WP bit */
        cr0 &= 0xfffeffff;
        asm volatile ("movq %%rax, %%cr0"
                        :   
                        : "a"(cr0)
                     );  
        return ret;
}
#endif

static void diting_door_module_bitclose(ulong val)
{
#ifdef __32bit__
	asm volatile ("movl %%eax, %%cr0"::"a"(val));	
#else
	asm volatile ("movq %%rax, %%cr0"::"a"(val));	
#endif
}


static int diting_door_module_interfaceset(struct security_operations *security_point)
{
	old_bprm_check_security = security_point->bprm_check_security;
	old_file_permission	= security_point->file_permission;
	old_inode_permission    = security_point->inode_permission;
	old_inode_create	= security_point->inode_create;
	old_inode_unlink	= security_point->inode_unlink;
	old_inode_link		= security_point->inode_link;
	old_inode_symlink	= security_point->inode_symlink;
	old_inode_rename	= security_point->inode_rename;
	old_inode_mkdir		= security_point->inode_mkdir;
	old_inode_rmdir		= security_point->inode_rmdir;

	security_point->bprm_check_security = diting_module_inside_bprm_check_security;
	security_point->file_permission	    = diting_module_inside_file_permission;
	security_point->inode_permission    = diting_module_inside_inode_permission;
	security_point->inode_create	    = diting_module_inside_inode_create;
	security_point->inode_unlink	    = diting_module_inside_inode_unlink;
	security_point->inode_link	    = diting_module_inside_inode_link;
	security_point->inode_symlink	    = diting_module_inside_inode_symlink;
	security_point->inode_rename	    = diting_module_inside_inode_rename;
	security_point->inode_mkdir	    = diting_module_inside_inode_mkdir;
	security_point->inode_rmdir	    = diting_module_inside_inode_rmdir;

	return 0;
}

static int diting_door_module_interfacereset(struct security_operations *security_point)
{
	security_point->bprm_check_security = old_bprm_check_security;
	security_point->file_permission	    = old_file_permission;
	security_point->inode_permission    = old_inode_permission;
	security_point->inode_create	    = old_inode_create;
	security_point->inode_unlink	    = old_inode_unlink;
	security_point->inode_link	    = old_inode_link;
	security_point->inode_symlink	    = old_inode_symlink;
	security_point->inode_rename	    = old_inode_rename;
	security_point->inode_mkdir	    = old_inode_mkdir;
	security_point->inode_rmdir	    = old_inode_rmdir;

	return 0;
}

struct diting_door_module diting_door_module =
{
	.bitopen	= diting_door_module_bitopen,
	.bitclose       = diting_door_module_bitclose,
	.interfaceset   = diting_door_module_interfaceset,
	.interfacereset = diting_door_module_interfacereset,
};
