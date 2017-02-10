#ifndef __diting_common_h__
#define __diting_common_h__


#define DITING_COMMON_PID_LOCKFILE	"/tmp/diting.lock"

extern int diting_common_get_lockfile();
extern int diting_common_put_lockfile(int pid);

#endif
