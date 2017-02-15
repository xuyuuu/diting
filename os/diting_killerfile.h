#ifndef __diting_killerfile_h__
#define __diting_killerfile_h__

#define DITING_KILLERFILE_LOADER "/etc/diting/killerinclude"

struct diting_killerfile_node
{
	struct list_head node;
	char file[1024];
}__attribute__((packed));

struct diting_killerfile_module
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

extern struct diting_killerfile_module diting_killerfile_module;

#endif
