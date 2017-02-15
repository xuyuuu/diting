#ifndef __diting_procfile_h__
#define __diting_procfile_h__

#define DITING_PROCFILE_LOADER "/etc/diting/procexclude"

struct diting_procfile_node
{
	struct list_head node;
	char file[1024];
}__attribute__((packed));

struct diting_procfile_module
{
	int (* init)(void);
	int (* load)(void);
	int (* unload)(void);
	int (* sync)(void);
	int (* reload)(void);
	int (* search)(char *dirname);
	int (* match)(char *dirname);
	int (* destroy)(void);
};

extern struct diting_procfile_module diting_procfile_module;

#endif
