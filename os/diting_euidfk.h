#ifndef __diting_euidfk_h__
#define __diting_euidfk_h__

struct diting_euidfk_module
{
	int (* init)(void);
	int (* replace)(void);
	int (* reset)(void);
};

extern struct diting_euidfk_module diting_euidfk_module;

#endif
