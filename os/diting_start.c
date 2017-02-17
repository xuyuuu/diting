#include <linux/module.h>
#include <linux/security.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kallsyms.h>
#include <linux/list.h>
#include <net/sock.h>
#include <linux/fs.h>
#include <linux/file.h>

#include "diting_door.h"
#include "diting_nolockqueue.h"
#include "diting_ktask.h"
#include "diting_config.h"

static diting_nolockqueue_t *diting_security_queue = NULL;
static struct security_operations *diting_security_ops = NULL;
static unsigned long diting_security_cr0 = 0;

static unsigned int
diting_get_symlen(const char * str)
{
	unsigned int i = 0;

	if (*(str-1) != ' ' && *(str-1) != '\t')
		return 0xFFFFFFFF;

	while (1)
	{
		if (str[i] == '\0' || str[i] == ' ' || 
				str[i] == '\t' || str[i] == '\n')
			break;
		i++;
	}

	return i;
}

static unsigned long
diting_get_symaddr(const char* hex_str)
{
	int i = 0;
	unsigned long address = 0;

	for(i = 0; i < sizeof(unsigned long) * 2; i++)
	{
		if (hex_str[i] >= '0' && hex_str[i] <= '9')
		{
			address = address << 4;
			address = address + (hex_str[i] - '0');
		}
		else if (hex_str[i] >= 'a' && hex_str[i] <= 'f')
		{
			address = address << 4;
			address = address + (hex_str[i] - 'a') + 10;
		}
		else if (hex_str[i] >= 'A' && hex_str[i] <= 'F')
		{
			address = address << 4;
			address = address + (hex_str[i] - 'A') + 10;
		}
		else break;
	}

	return address;
}

unsigned long
diting_find_symbol_addr(const char* sym)
{
	struct file * fp;
	mm_segment_t old_fs;
	char buffer[128] = {0}, *p = NULL;
	int stop = 0;
	int i = 0, ret = 0;
	unsigned long addr = 0;

#define DITING_DEFAULT_INIT		"/proc/kallsyms"
	fp = filp_open(DITING_DEFAULT_INIT, O_RDONLY, 0);
	if(fp == NULL || IS_ERR(fp))
		return 0;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	while(!stop)
	{
		i = 0;
		memset(buffer, 0, 128);
		do
		{
			ret = fp->f_op->read(fp, buffer + i, 1, &fp->f_pos);
			if (ret <= 0)
				stop = 1;
			if (buffer[i] == '\n')
				break;
			if (i >= 126)
				break; 
			i++;
		}while(ret > 0);

		p = strstr(buffer, sym);
		if(!p)
			continue;
		if(strlen(sym) == diting_get_symlen(p))
		{
			addr = diting_get_symaddr(buffer);
			break;
		}
	}

	set_fs(old_fs);
	fput(fp);

	return addr;
}

static int diting_init_hookaddr(struct security_operations **address)
{
	int ret = 0;
	unsigned long tmpops = 0;


	tmpops = diting_find_symbol_addr("security_ops");
	if (!tmpops){
		printk("can't find security_ops symbol.\n");
		ret = -1;	
	}
	*address = (struct security_operations *)(*(unsigned long *)tmpops);

	return ret;
}


static int __init diting_init(void)
{
	int ret = 0;

	if(diting_init_hookaddr(&diting_security_ops)){
		ret = -1;
		goto out;
	}

	printk("DITING IS RUNNING !!!\n");
	/*init queue*/
	diting_security_queue = diting_nolockqueue_module.create(4096);
	if(!diting_security_queue){
		ret = -1;
		goto out;
	}

	diting_config_module.init();
	diting_config_module.load();
	diting_config_module.sync();

	diting_ktask_module.init();	
	diting_ktask_module.create();
	diting_ktask_module.run();

	diting_security_cr0 = diting_door_module.bitopen();
	diting_door_module.interfaceset(diting_security_ops);
	diting_door_module.bitclose(diting_security_cr0);


out:
	return ret;
}

static void __exit diting_exit(void)
{
	diting_security_cr0 = diting_door_module.bitopen();
	diting_door_module.interfacereset(diting_security_ops);
	diting_door_module.bitclose(diting_security_cr0);

	/*destroy kernel task resource*/
	diting_ktask_module.destroy();

	if(diting_security_queue)
		diting_nolockqueue_module.destroy(diting_security_queue);

	diting_config_module.destroy();
}

module_init(diting_init);
module_exit(diting_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("DiTing Module");
