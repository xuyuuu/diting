#include <linux/version.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32)
#include "fs/mount.h"
#endif

#include <net/sock.h>
#include <linux/security.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/fs_struct.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/net.h>
#include <linux/mount.h>
#include <linux/list.h>
#include <linux/dcache.h>
#include <linux/stat.h>
#include <linux/spinlock.h>

#include <asm/current.h>
#include <asm/signal.h>

#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/pagemap.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
#include <linux/namespace.h>
#include <linux/mm.h>
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 18) && LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 32)
#include <linux/nsproxy.h>
#include <linux/mnt_namespace.h>
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 32)
#include <linux/nsproxy.h>
#endif

#include "diting_util.h"

char *diting_common_filp_fgets(char *str, int size, struct file *filp)
{ 
	char *cp; 
	int len, readlen; 
	mm_segment_t oldfs; 

	if (filp && filp->f_op->read && ((filp->f_flags & O_ACCMODE) & O_WRONLY) == 0)
	{ 

		oldfs = get_fs(); 
		set_fs(KERNEL_DS); 
		for (cp = str, len = -1, readlen = 0; readlen < size - 1; ++cp, ++readlen)
		{ 
			if ((len = filp->f_op->read(filp, cp, 1, &filp->f_pos)) <= 0) 
				break; 
			if (*cp == '\n')
			{ 
				++cp; 
				++readlen; 
				break; 
			} 
		} 
		*cp = 0; 
		set_fs(oldfs); 

		return (len < 0 || readlen == 0) ? NULL : str; 

	}
	else
		return NULL; 
} 


static char * diting_common_inside_get_taskname(struct task_struct *task, char *kbuf)
{
	char *rtnstr = NULL;
	struct file *file_fp = NULL;
        struct mm_struct *mm;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
	struct vm_area_struct *vma = NULL;
#endif

        mm = get_task_mm(task);
        if (!mm)
                return NULL;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
	down_read(&mm->mmap_sem);
	vma = mm->mmap;
	while(vma)
	{
		if((vma->vm_flags & VM_EXECUTABLE) && vma->vm_file)
		{
			file_fp = vma->vm_file;	
			if (file_fp || !IS_ERR(file_fp))
			{
				get_file(file_fp);
				mntget(file_fp->f_vfsmnt);
				dget(file_fp->f_dentry);
			}
			break;
		}
		vma = vma->vm_next;
	}
	up_read(&mm->mmap_sem);
	mmput(mm);
	if (file_fp && !IS_ERR(file_fp))
	{
		rtnstr = d_path(file_fp->f_dentry, file_fp->f_vfsmnt, kbuf, PAGE_SIZE);
		dput(file_fp->f_dentry);
		mntput(file_fp->f_vfsmnt);
		fput(file_fp);
	}
#else
        down_read(&mm->mmap_sem);
        file_fp = mm->exe_file;
        if(file_fp && !IS_ERR(file_fp))
        {   
		get_file(file_fp);
		path_get(&file_fp->f_path);
        }
        up_read(&mm->mmap_sem);
        mmput(mm);

	if(file_fp && !IS_ERR(file_fp))
	{
		rtnstr = d_path(&file_fp->f_path, kbuf, PAGE_SIZE);
		path_put(&file_fp->f_path);
		fput(file_fp);
	}
#endif
	
	return rtnstr;
}

static char * diting_common_inside_get_accessname(struct dentry *dentry, char *kbuf)
{
	char *pathroot=NULL;
	int size;
	struct list_head *a_head = NULL, *a_pos = NULL;
	struct dentry *a_root = NULL, *a_dentry = NULL; 
	struct vfsmount *a_mnt = NULL;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
	struct vfsmount *p_vfsmnt = NULL;
	struct dentry *p_dentry = dentry;
#else
	struct path path;
	path.dentry = dentry;
#endif

	a_root = dget(dentry->d_sb->s_root);
	a_dentry = dentry;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 32)
	a_head = &(current->nsproxy->mnt_ns->root->mnt_list);
#elif (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 18) && LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 32))
	a_head = &(current->nsproxy->mnt_ns->list);
#elif (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18))
	a_head = &(current->namespace->list);
#endif
	a_pos = a_head->next;
	while(a_pos != a_head)
	{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 32)
		struct mount *mnt;
		mnt = list_entry(a_pos, struct mount, mnt_list);
		a_mnt = &(mnt->mnt);
#else
		a_mnt = list_entry(a_pos,struct vfsmount,mnt_list);
#endif
		a_pos=a_pos->next;
		if(a_root==a_mnt->mnt_root)
		{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
			p_vfsmnt =  a_mnt;
			mntget(p_vfsmnt);
			pathroot = d_path(p_dentry, p_vfsmnt, kbuf, PAGE_SIZE);
			mntput(p_vfsmnt);
#else
			path.mnt = a_mnt;
			path_get(&path);
			pathroot = d_path(&path, kbuf, PAGE_SIZE);
			path_put(&path);
#endif
			if(IS_ERR(pathroot))
				pathroot=NULL;
			else
				size = strlen(pathroot);
			break;
		}

	}
	dput(a_root);

	return pathroot;
}

char * diting_common_get_name(struct task_struct *task, char ** name, struct dentry *dentry, int type)
{
	char *p = NULL, *path = NULL;

	path = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!path || IS_ERR(path)){
		path = NULL;
		return NULL;
	}
	memset(path, 0x0, PAGE_SIZE);

	if (DITING_FULLFILE_ACCESS_TYPE == type)
		p = diting_common_inside_get_accessname(dentry, path);
	else
		p = diting_common_inside_get_taskname(task, path);

	if (!p || IS_ERR(p))
		kfree(path);

	*name = path;

	return p;
}


static int diting_common_inside_get_parameters(struct task_struct *tsk, unsigned long addr, void *buf, int len)
{
	struct mm_struct *mm;
	struct vm_area_struct *vma;
	void * old_buf = buf;

	mm = get_task_mm(tsk);
	if (!mm)
		return 0;

	if (addr + len < addr)
		return 0;

	down_read(&mm->mmap_sem);
	while(len)
	{
		int bytes, ret, offset;
		void *maddr;
		struct page *page = NULL;

		ret = get_user_pages(tsk, mm, addr, 1,
				0, 1, &page, &vma);
		if (ret <= 0)
		{
			break;
			bytes = ret;
		}
		else
		{
			bytes = len;
			offset = addr & (PAGE_SIZE - 1);
			if (bytes > PAGE_SIZE - offset)
				bytes = PAGE_SIZE - offset;

			maddr = kmap(page);
			copy_from_user_page(vma, page, addr,
				buf, maddr + offset, bytes);
			kunmap(page);
			page_cache_release(page);
		}
		len -= bytes;
		buf += bytes;
		addr += bytes;	
	}

	up_read(&mm->mmap_sem);
	mmput(mm);

	return buf - old_buf;
}

int diting_common_getuser(struct task_struct *p, char *username)
{
	int res = 0, ret = -1, i, j = 0;
	unsigned int len;
	char *parameters;
	struct mm_struct *mm;

	parameters = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if(!parameters)
		goto out;
	memset(parameters, 0x0, PAGE_SIZE);

	mm = get_task_mm(p);
	if (!mm)
		goto out;
	if (!mm->env_end)
		goto out_mm;

	len = mm->env_end - mm->env_start;
	if (len > PAGE_SIZE)
		len = PAGE_SIZE - 1;
	res = diting_common_inside_get_parameters(p, mm->env_start, parameters, len);
	if(res > 0)
	{
		for(i = 0; i < len - 5; i++){
			if('H' == parameters[i] && 'O' == parameters[i+1] && 'M' == parameters[i+2] &&
					'E' == parameters[i+3] && '=' == parameters[i+4]){

				i += 6;
				while((i < len) && ('\0' != parameters[i]))
					username[j++] = parameters[i++];
				ret = 0;
				break;
			}
		}
	}

out_mm:
	mmput(mm);
out:
	if(parameters && !IS_ERR(parameters))
		kfree(parameters);
	return ret;
}

