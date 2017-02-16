#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mman.h>
#include <linux/version.h>
#include <linux/dcache.h>
#include <linux/jiffies.h>
#include <linux/sysctl.h>
#include <linux/err.h>
#include <linux/namei.h>
#include <linux/fs_struct.h>
#include <linux/mount.h>
#include <linux/pid.h>

#include <net/net_namespace.h>
#include <linux/netlink.h>


#include "diting_sysctl.h"

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
#define CTL_UNNUMBERED -2
#endif

static struct ctl_table_header *diting_sysctl_table_header;

static uint32_t diting_sysctl_table_procbehavior_reload;
static uint32_t diting_sysctl_table_procbehavior_switch;
static uint32_t diting_sysctl_table_accessbehavior_reload;
static uint32_t diting_sysctl_table_accessbehavior_switch;
static uint32_t diting_sysctl_table_killerbehavior_reload;
static uint32_t diting_sysctl_table_killerbehavior_switch;
static uint32_t diting_sysctl_table_socketbehavior_reload;
static uint32_t diting_sysctl_table_socketbehavior_switch;


static ctl_table diting_sysctl_register_subtable[] =
{
	{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
		.ctl_name	= CTL_UNNUMBERED,
#endif
		.procname	= "procrun_reload",
		.data		= &(diting_sysctl_table_procbehavior_reload),
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
		.ctl_name	= CTL_UNNUMBERED,
#endif
		.procname	= "procrun_switch",
		.data		= &(diting_sysctl_table_procbehavior_switch), 
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
		.ctl_name	= CTL_UNNUMBERED,
#endif
		.procname	= "access_reload",
		.data		= &(diting_sysctl_table_accessbehavior_reload),
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
		.ctl_name	= CTL_UNNUMBERED,
#endif
		.procname	= "access_switch",
		.data		= &(diting_sysctl_table_accessbehavior_switch), 
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
		.ctl_name	= CTL_UNNUMBERED,
#endif
		.procname	= "killer_reload",
		.data		= &(diting_sysctl_table_killerbehavior_reload),
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
		.ctl_name	= CTL_UNNUMBERED,
#endif
		.procname	= "killer_switch",
		.data		= &(diting_sysctl_table_killerbehavior_switch), 
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
		.ctl_name	= CTL_UNNUMBERED,
#endif
		.procname	= "socket_reload",
		.data		= &(diting_sysctl_table_socketbehavior_reload),
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
		.ctl_name	= CTL_UNNUMBERED,
#endif
		.procname	= "socket_switch",
		.data		= &(diting_sysctl_table_socketbehavior_switch), 
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{0}
};

static ctl_table diting_sysctl_register_table[] =
{
	{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
		.ctl_name	= CTL_KERN,
#endif
		.procname	= "kernel",
		.mode		= 0500,
		.child		= diting_sysctl_register_subtable,
	},
	{0}
};

int diting_sysctl_module_init(void)
{
	if (!(diting_sysctl_table_header = register_sysctl_table(diting_sysctl_register_table
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19) 
					,0
#endif
					))){
		printk("Unable To Register Sysctl Table ...\n");
		return -1;
	}

	diting_sysctl_table_procbehavior_reload = 0;
	diting_sysctl_table_procbehavior_switch = 0;
	diting_sysctl_table_accessbehavior_reload = 0;
	diting_sysctl_table_accessbehavior_switch = 0;
	diting_sysctl_table_killerbehavior_reload = 0;
	diting_sysctl_table_killerbehavior_switch = 0;
	diting_sysctl_table_socketbehavior_reload = 0;
	diting_sysctl_table_socketbehavior_switch = 0;


	return 0;
}

static int
diting_sysctl_module_destroy(void)
{
	unregister_sysctl_table(diting_sysctl_table_header);

	return 0;
}


static int
diting_sysctl_module_chkstatus(sysctl_type_t type, uint32_t *old)
{
	int ret = -1;

	switch(type){
	case DITING_PROCBEHAVIOR_RELOAD:
		if(diting_sysctl_table_procbehavior_reload){
			diting_sysctl_table_procbehavior_reload = 0;
			ret = 0;
		}
		break;
	case DITING_PROCBEHAVIOR_SWITCH:
		if(*old != diting_sysctl_table_procbehavior_switch){
			*old = diting_sysctl_table_procbehavior_switch;
			ret = 0;
		}
		break;
	case DITING_ACCESSBEHAVIOR_RELOAD:
		if(diting_sysctl_table_accessbehavior_reload){
			diting_sysctl_table_accessbehavior_reload = 0;	
			ret = 0;
		}
		break;
	case DITING_ACCESSBEHAVIOR_SWITCH:
		if(*old != diting_sysctl_table_accessbehavior_switch){
			*old = diting_sysctl_table_accessbehavior_switch;
			ret= 0;
		}
		break;
	case DITING_KILLERBEHAVIOR_RELOAD:
		if(diting_sysctl_table_killerbehavior_reload){
			diting_sysctl_table_killerbehavior_reload = 0;	
			ret = 0;
		}
		break;
	case DITING_KILLERBEHAVIOR_SWITCH:
		if(*old != diting_sysctl_table_killerbehavior_switch){
			*old = diting_sysctl_table_killerbehavior_switch;
			ret = 0;
		}
		break;
	case DITING_SOCKETBEHAVIOR_RELOAD:
		if(diting_sysctl_table_socketbehavior_reload){
			diting_sysctl_table_socketbehavior_reload = 0;	
			ret = 0;
		}
		break;
	case DITING_SOCKETBEHAVIOR_SWITCH:
		if(*old != diting_sysctl_table_socketbehavior_switch){
			*old = diting_sysctl_table_socketbehavior_switch;
			ret = 0;
		}
		break;
	default:	
		break;
	}
	

	return ret;
}


struct diting_sysctl_module diting_sysctl_module = {
	.init		= diting_sysctl_module_init,
	.chkstatus	= diting_sysctl_module_chkstatus,
	.destroy	= diting_sysctl_module_destroy
};
