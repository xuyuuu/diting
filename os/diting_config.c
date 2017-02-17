#include <linux/types.h>
#include <linux/list.h> 
#include <linux/fs.h>
#include <linux/file.h>

#include "diting_config.h"

#include "diting_procfile.h"
#include "diting_accessfile.h"
#include "diting_killerfile.h"
#include "diting_sockfile.h"
#include "diting_util.h"


static int
diting_config_module_init(void)
{
	diting_procfile_module.init();
	diting_accessfile_module.init();
	diting_killerfile_module.init();
	diting_sockfile_module.init();

	return 0;
}

static int
diting_config_module_load(void)
{
	diting_procfile_module.load();
	diting_accessfile_module.load();
	diting_killerfile_module.load();
	diting_sockfile_module.load();

	return 0;
}

static int
diting_config_module_unload(void)
{
	diting_procfile_module.unload();
	diting_accessfile_module.unload();
	diting_killerfile_module.unload();
	diting_sockfile_module.unload();

	return 0;
}

static int
diting_config_module_sync(void)
{
	diting_procfile_module.sync();
	diting_accessfile_module.sync();
	diting_killerfile_module.sync();
	diting_sockfile_module.sync();

	return 0;
}


static int
diting_config_module_reload(int type)
{
	switch(type){
	case DITING_PROCRUN:
		diting_procfile_module.reload();
		break;
	case DITING_PROCACCESS:	
		diting_accessfile_module.reload();
		break;
	case DITING_KILLER:
		diting_killerfile_module.reload();
		break;
	case DITING_SOCKET:
		diting_sockfile_module.reload();
		break;
	default:
		break;
	}

	return 0;
}


static int
diting_config_module_search(int type, char *pattern)
{
	int ret = 0;
	switch(type){
	case DITING_PROCRUN:
		ret = diting_procfile_module.search(pattern);
		break;
	case DITING_PROCACCESS:	
		ret = diting_accessfile_module.search(pattern);
		break;
	case DITING_KILLER:
		ret = diting_killerfile_module.search(pattern);
		break;
	case DITING_SOCKET:
		ret = diting_sockfile_module.search(pattern);
		break;
	default:
		break;
	}

	return ret;
}


static int
diting_config_module_destroy(void)
{
	diting_procfile_module.destroy();
	diting_accessfile_module.destroy();
	diting_killerfile_module.destroy();
	diting_sockfile_module.destroy();

	return 0;
}


struct diting_config_module diting_config_module = {
	.init		= diting_config_module_init,
	.load		= diting_config_module_load,
	.unload		= diting_config_module_unload,
	.sync		= diting_config_module_sync,
	.reload		= diting_config_module_reload,
	.search		= diting_config_module_search,
	.destroy	= diting_config_module_destroy
};
