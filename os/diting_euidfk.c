#include <linux/kernel.h>  
#include <linux/version.h>
#include <linux/list.h>
#include <linux/module.h>  
#include <linux/err.h>  
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/fs_struct.h>
#include <linux/types.h>
#include <asm/current.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
#include <linux/namespace.h>
#include <linux/mm.h>
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 18) && LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 32)
#include <linux/nsproxy.h>
#include <linux/mnt_namespace.h>
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 32)
#include <linux/nsproxy.h>
#endif

#include "diting_euidfk.h"
#include "diting_util.h"
#include "diting_nolockqueue.h"

static unsigned long diting_euidfk_do_fork;
static unsigned long diting_euidfk_do_exit;
static unsigned long diting_euidfk_copy_process;
static unsigned long diting_euidfk_profile_task_exit;

static struct task_struct *(* old_copy_process)(unsigned long clone_flags,
					unsigned long stack_start,
					struct pt_regs *regs,
					unsigned long stack_size,
					int __user *child_tidptr,
					struct pid *pid,
					int trace);
static struct task_struct * diting_euidfk_inside_copy_process(unsigned long clone_flags,
					unsigned long stack_start,
					struct pt_regs *regs,
					unsigned long stack_size,
					int __user *child_tidptr,
					struct pid *pid,
					int trace)
{
	struct file *dt_file = NULL;
	struct mm_struct *dt_mm = NULL;
	struct dentry *dt_dentry = NULL;
	struct inode *dt_inode = NULL;
	struct task_struct *p;
	char username[64] = {0}, *path = NULL, *name = NULL;

	struct diting_chroot_msgnode *item = NULL;

	dt_mm = current->mm;
	if(!dt_mm || IS_ERR(dt_mm))
		goto out;

	dt_file = dt_mm->exe_file;
	if(!dt_file || IS_ERR(dt_file))
		goto out;

	dt_dentry = dt_file->f_dentry;
	if(!dt_dentry || IS_ERR(dt_dentry))
		goto out;

	dt_inode = dt_dentry->d_inode;
	if(!dt_inode || IS_ERR(dt_inode))
		goto out;

	if(((dt_inode->i_mode & S_ISUID) == S_ISUID) && (0 == current_uid())){
		item = (struct diting_chroot_msgnode *)kmalloc(
				sizeof(struct diting_chroot_msgnode), GFP_KERNEL);
		memset(item, 0x0, sizeof(struct diting_chroot_msgnode));
		item->type = DITING_CHROOT;
		if(current->cred && !IS_ERR(current->cred))
			item->uid  = current->cred->uid;
		else
			item->uid = -1;
		if(diting_common_getuser(current, username))
			strncpy(username, "SYSTEM", sizeof("SYSTEM") - 1);
		strncpy(item->username, username, strlen(username));
		
		path = diting_common_get_name(current, &name, NULL, 
				DITING_FULLFILE_TASK_TYPE);
		if(path && !IS_ERR(path)){
			strncpy(item->proc, path, strlen(path));
			kfree(name);
		}else{
			strncpy(item->proc, current->comm, strlen(current->comm));	
		}
		diting_nolockqueue_module.enqueue(diting_nolockqueue_module.getque(), item);
	}

out:
	p = (*old_copy_process)(clone_flags, stack_start, regs, stack_size, child_tidptr, pid, trace);

	return p;
}

static int diting_euidfk_module_inside_init(void)
{
	int ret;
	unsigned long res;

	res = diting_find_symbol_addr("do_fork");	
	if (!res)
	{
		printk("can't find do_fork symbol.\n");
		ret = -1;
		goto out;
	}
	diting_euidfk_do_fork = res;

	res = diting_find_symbol_addr("do_exit");	
	if (!res)
	{
		printk("can't find do_exit symbol.\n");
		ret = -1;
		goto out;
	}
	diting_euidfk_do_exit = res;

	res = diting_find_symbol_addr("copy_process");	
	if (!res)
	{
		printk("can't find copy_process symbol.\n");
		ret = -1;
		goto out;
	}
	diting_euidfk_copy_process = res;

	res = diting_find_symbol_addr("profile_task_exit");	
	if (!res)
	{
		printk("can't find profile_task_exit symbol.\n");
		ret = -1;
		goto out;
	}
	diting_euidfk_profile_task_exit = res;

	ret = 0;
out:
	return ret;
}

static int diting_euidfk_module_inside_inside_replace(unsigned long base, unsigned long old_function, unsigned long new_function)
{
	int i = 0;
	unsigned char *p = (unsigned char *)base;

	while(1)
	{
		if(i++ > 256)
			return 0;

		if(*p == 0xe8)
		{
			if((*(int *)(p+1) + (unsigned long)p + 5) == old_function)
			{
				*(int *)(p+1) = new_function - (unsigned long)p - 5;
				return 1;
			}
		}
		p++;
	}

	return 0;
}

static int diting_euidfk_module_inside_replace(void)
{
	old_copy_process = (void *)diting_euidfk_copy_process;
	diting_euidfk_module_inside_inside_replace(diting_euidfk_do_fork,
		diting_euidfk_copy_process, (unsigned long)diting_euidfk_inside_copy_process);

	return 0;
}

static int diting_euidfk_module_inside_reset(void)
{
	diting_euidfk_module_inside_inside_replace(diting_euidfk_do_fork,
		(unsigned long)diting_euidfk_inside_copy_process, (unsigned long)old_copy_process);

	return 0;
}

struct diting_euidfk_module diting_euidfk_module = 
{
	.init 		= diting_euidfk_module_inside_init,
	.replace	= diting_euidfk_module_inside_replace,
	.reset		= diting_euidfk_module_inside_reset
};
