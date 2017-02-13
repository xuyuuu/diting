#ifndef __diting_sockmsg_h__
#define __diting_sockmsg_h__

struct diting_sockmsg_module{
	int (*init)(void);
	int (*sendlog)(void *data, int len, int type);
	int (*destroy)(void);
};

extern struct diting_sockmsg_module diting_sockmsg_module;


#endif
