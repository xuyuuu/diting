#ifndef __diting_sockmsg_h__
#define __diting_sockmsg_h__

struct diting_sockmsg_module{
	int (*init)(void);
	int (*syn)(void);
	int (*loop)(void);
	int (*destroy)(void);
};

extern struct diting_sockmsg_module diting_sockmsg_module;

#endif
