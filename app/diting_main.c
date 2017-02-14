#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <dlfcn.h>
#include <getopt.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "diting_signal.h"
#include "diting_common.h"
#include "diting_sockmsg.h"

static pid_t diting_main_global_pid;

#define DITING_AGENT_DYNAMIC_START_FNAME	"getopt"
extern int getopt(int argc, char * const argv[], const char * optstring);

/*
#define DITING_AGENT_DYNAMIC_STOP_FNAME	"usleep"
extern int usleep(useconds_t usec);
*/

static int
diting_main_set_procname(char *name, char * const *argv)
{
	int i;
	size_t size;
	for (i = 0, size = 0; argv[i]; i ++) {
		size += strlen(argv[i]) + 1;
	}   
	memcpy(argv[0], name, strlen(name) + 1); 

	if (size > strlen(name)+1) {
		memset(argv[0] + strlen(name) + 1, 0, size - strlen(name) - 1); 
	}   
	return 0;
}

static int 
diting_main_detach_task()
{
	if(diting_sockmsg_module.init())
		return 0;
	diting_sockmsg_module.syn();
	diting_sockmsg_module.loop();
	diting_sockmsg_module.destroy();
	remove(DITING_COMMON_PID_LOCKFILE);

	return 0;
}


static int
diting_main_start(char * const *argv)
{
	pid_t apid;

	if(diting_common_get_lockfile())
		goto out;

	apid = fork();
	if(apid == 0){
		/*subprocess*/
		diting_main_set_procname("diting", argv);

		diting_signal_module.init();
		diting_main_detach_task();	
		exit(0);
	}
	diting_main_global_pid = apid;
	diting_common_put_lockfile(apid);

out:
	return 0;
}

int getopt(int argc, char * const argv[],
	const char *optstring)
{
	diting_main_start(argv);
	int (*old_getopt)(int, char * const *, const char *);
	if(NULL != (old_getopt = dlsym(RTLD_NEXT, DITING_AGENT_DYNAMIC_START_FNAME)))
		return old_getopt(argc, argv, optstring);
	else
		return -1;
}

/*
 *
static int
diting_main_stop()
{
	if(kill(diting_main_global_pid, SIGUSR1))
		kill(diting_main_global_pid, SIGUSR2);
	waitpid(diting_main_global_pid, NULL, 0);

	return 0;
}


int usleep(useconds_t usec)
{
	if(usec > 1000)
		diting_main_stop();

	int (*old_usleep)(useconds_t usec);
	if(NULL != (old_usleep = dlsym(RTLD_NEXT, DITING_AGENT_DYNAMIC_STOP_FNAME)))
		return old_usleep(usec);
	else
		return -1;
}
*/
