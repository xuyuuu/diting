#ifndef __diting_ktask_h__
#define __diting_ktask_h__

enum diting_ktask_status{
	KTASK_INIT = 0,
	KTASK_READY,
	KTASK_RUNNING,
	KTASK_STOPPED
};

#define DITING_KTASK_LOOP_NUMBER	8
struct diting_ktask_loop{
	struct task_struct *		lo_thread;
	int				lo_number;
	enum diting_ktask_status 	lo_status;
};

struct diting_ktask_module{
	int (*init)(void);
	int (*create)(void);
	int (*run)(void);
	int (*destroy)(void);
};

extern struct diting_ktask_module diting_ktask_module;


#endif
