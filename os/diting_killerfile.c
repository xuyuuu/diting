#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>

#include "diting_killerfile.h"
#include "diting_util.h"

static struct list_head diting_killerfile_head[2];
static rwlock_t diting_killerfile_rwlock[2];

static uint8_t diting_killerfile_idle_mark = 0;
static uint8_t diting_killerfile_used_mark = 1;

static int diting_killerfile_module_init(void)
{
	int i, ret = 0;

	for (i = 0; i < 2; i++){
		INIT_LIST_HEAD(&diting_killerfile_head[i]);
		rwlock_init(&diting_killerfile_rwlock[i]);
	}

	diting_killerfile_idle_mark = 0;
	diting_killerfile_used_mark = 1;

	return ret;
}

static int diting_killerfile_module_load_inside_do(char *buffer)
{
	int ret = 0;
	struct diting_killerfile_node *item= NULL;	

	item = kmalloc(sizeof(struct diting_killerfile_node), GFP_KERNEL);
	if (!item || IS_ERR(item)){
		ret = -1;
		goto err0;
	}

	memset(item, 0x0, sizeof(struct diting_killerfile_node));
	INIT_LIST_HEAD(&item->node);
	strncpy(item->file, buffer, strlen(buffer));

	write_lock(&diting_killerfile_rwlock[diting_killerfile_idle_mark]);
	list_add_tail(&item->node, &diting_killerfile_head[diting_killerfile_idle_mark]);
	write_unlock(&diting_killerfile_rwlock[diting_killerfile_idle_mark]);

err0:
	return ret;
}

static int diting_killerfile_module_load(void)
{
	struct file *fp;
	mm_segment_t oldfs;
	char buffer[1024];
	int fsize, nr, ret = 0;

	fp = filp_open(DITING_KILLERFILE_LOADER, O_RDONLY, 0);
	if(fp == NULL || IS_ERR(fp)){
		ret = -1;
		goto err0;
	}

	fsize = fp->f_dentry->d_inode->i_size;	
	if (fsize == 0)
		goto err0;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	while(diting_common_filp_fgets(buffer, sizeof(buffer) - 1, fp)){
		nr = strlen(buffer);
                if (buffer[nr - 1] == '\n')
                          buffer[nr - 1] = '\0';

		nr = strlen(buffer);
		if((buffer[0] == '#') || (nr == 0))
			goto next;

		diting_killerfile_module_load_inside_do(buffer);
next:
		memset(buffer, 0x0, sizeof(buffer));
	}
	set_fs(oldfs);

err0:
	if (fp && !IS_ERR(fp))
		filp_close(fp, 0);

	return ret;
}

static int diting_killerfile_module_unload(void)
{
	int ret = 0;
	struct diting_killerfile_node *item = NULL, *nxt = NULL;

	write_lock(&diting_killerfile_rwlock[diting_killerfile_idle_mark]);
	if (list_empty(&diting_killerfile_head[diting_killerfile_idle_mark])){
		write_unlock(&diting_killerfile_rwlock[diting_killerfile_idle_mark]);
		goto out;
	}
	list_for_each_entry_safe(item, nxt, &diting_killerfile_head[diting_killerfile_idle_mark], node){
		list_del(&item->node);
		kfree(item);
	}
	write_unlock(&diting_killerfile_rwlock[diting_killerfile_idle_mark]);

out:
	return ret;
}

static int diting_killerfile_module_sync(void)
{
	int swap, ret = 0;

	swap = diting_killerfile_idle_mark;
	diting_killerfile_idle_mark = diting_killerfile_used_mark;
	diting_killerfile_used_mark = swap;

	return ret;
}

static int diting_killerfile_module_reload(void)
{
	int ret = 0;

	diting_killerfile_module.unload();
	diting_killerfile_module.load();
	diting_killerfile_module.sync();

	return ret;
}


static int diting_killerfile_module_search(char *file)
{
	int ret = -1, mark;
	struct diting_killerfile_node *item = NULL;

	mark = diting_killerfile_used_mark;
	read_lock(&diting_killerfile_rwlock[mark]);
	list_for_each_entry(item, &diting_killerfile_head[mark], node){
		if(!strncmp(file, item->file, strlen(item->file))){
			ret = 0;
			break;
		}
	}
	read_unlock(&diting_killerfile_rwlock[mark]);

	return ret;
}


static int diting_killerfile_module_destroy(void)
{
	int i, ret = 0;
	struct diting_killerfile_node *item = NULL, *nxt = NULL;

	for (i = 0; i < 2; i++){
		write_lock(&diting_killerfile_rwlock[i]);
		if (list_empty(&diting_killerfile_head[i])){
			write_unlock(&diting_killerfile_rwlock[i]);
			continue;
		}
		list_for_each_entry_safe(item, nxt, &diting_killerfile_head[i], node){
			list_del(&item->node);
			kfree(item);
		}
		write_unlock(&diting_killerfile_rwlock[i]);
	}

	return ret;
}


struct diting_killerfile_module diting_killerfile_module = 
{
	.init	= diting_killerfile_module_init,
	.load	= diting_killerfile_module_load,
	.sync	= diting_killerfile_module_sync,
	.unload	= diting_killerfile_module_unload,
	.reload	= diting_killerfile_module_reload,
	.search	= diting_killerfile_module_search,
	.destroy= diting_killerfile_module_destroy, 
};
