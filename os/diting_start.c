#include <linux/module.h>
#include <linux/security.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kallsyms.h>
#include <linux/list.h>
#include <net/sock.h>
#include <linux/fs.h>
#include <linux/file.h>

#include "diting_util.h"
#include "diting_door.h"
#include "diting_nolockqueue.h"
#include "diting_ktask.h"
#include "diting_config.h"
#include "diting_euidfk.h"

static diting_nolockqueue_t *diting_security_queue = NULL;
static struct security_operations *diting_security_ops = NULL;
static unsigned long diting_security_cr0 = 0;

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

	diting_euidfk_module.init();
	diting_security_cr0 = diting_door_module.bitopen();
	diting_door_module.interfaceset(diting_security_ops);
	diting_euidfk_module.replace();
	diting_door_module.bitclose(diting_security_cr0);

out:
	return ret;
}

static void __exit diting_exit(void)
{
	diting_security_cr0 = diting_door_module.bitopen();
	diting_door_module.interfacereset(diting_security_ops);
	diting_euidfk_module.reset();
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
