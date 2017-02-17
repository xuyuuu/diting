#ifndef __diting_config_h__
#define __diting_config_h__


struct diting_config_module{
	int (*init)(void);
	int (*load)(void);
	int (*unload)(void);
	int (*sync)(void);
	int (*reload)(int type);
	int (*search)(int type, char *pattern);
	int (*destroy)(void);
};

extern struct diting_config_module diting_config_module;


#endif
