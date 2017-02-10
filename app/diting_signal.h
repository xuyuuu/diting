#ifndef __diting_signal_h__
#define __diting_signal_h__

struct diting_signal_module{
	int (*init)(void);
	int (*getstatus)(void);
};

extern struct diting_signal_module diting_signal_module;

#endif
