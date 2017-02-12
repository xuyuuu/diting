#ifndef __diting_sysctl_h__
#define __diting_sysctl_h__

typedef enum DITING_SYSCTL_TYPE{
	DITING_PROCBEHAVIOR_RELOAD = 0,
	DITING_PROCBEHAVIOR_SWITCH,
	DITING_ACCESSBEHAVIOR_RELOAD,
	DITING_ACCESSBEHAVIOR_SWITCH,
	DITING_KILLERBEHAVIOR_RELOAD,
	DITING_KILLERBEHAVIOR_SWITCH,
	DITING_NETWORDBEHAVIOR_RELOAD,
	DITING_NETWORDBEHAVIOR_SWITCH
}sysctl_type_t;

struct diting_sysctl_module 
{
	int (* init)(void);
	int (* chkstatus)(sysctl_type_t, uint32_t *old);
	int (* destroy)(void);
};

extern struct diting_sysctl_module diting_sysctl_module;

#endif
