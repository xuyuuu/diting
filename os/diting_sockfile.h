#ifndef __diting_sockfile_h__
#define __diting_sockfile_h__

#define DITING_SOCKFILE_LOADER "/etc/diting/socketexclude"

struct diting_sockfile_node
{
	struct list_head node;
	char file[1024];
}__attribute__((packed));

struct diting_sockfile_module
{
	int (* init)(void);
	int (* load)(void);
	int (* unload)(void);
	int (* sync)(void);
	int (* reload)(void);
	int (* search)(char *pattern);
	int (* destroy)(void);
};

extern struct diting_sockfile_module diting_sockfile_module;

#endif
