#ifndef __diting_accessfile_h__
#define __diting_accessfile_h__

#define DITING_ACCESSFILE_LOADER "/etc/diting/accessinclude"

struct diting_accessfile_node
{
	struct list_head node;
	char file[1024];
}__attribute__((packed));

struct diting_accessfile_module
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

extern struct diting_accessfile_module diting_accessfile_module;

#endif
