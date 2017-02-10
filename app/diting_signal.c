#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "diting_signal.h"

static int diting_signal_stop_flag = 0;
static pid_t diting_signal_hijack_pid = 0;

static void 
diting_signal_inside_handler(int signum)
{
	if(SIGUSR1 == signum || SIGUSR2 == signum)
		diting_signal_stop_flag = 1;

	return;
}

static
int diting_signal_module_init()
{
	sigset_t set;

	struct sigaction sigact;
	memset(&sigact, 0x0, sizeof(struct sigaction));

        sigfillset(&set);
        sigdelset(&set, SIGUSR1);
        sigdelset(&set, SIGUSR2);

	sigact.sa_handler = diting_signal_inside_handler;
	sigact.sa_mask = set;

	sigaction(SIGUSR1, &sigact, NULL);
	sigaction(SIGUSR2, &sigact, NULL);

	return 0;
}

static 
int diting_signal_module_getstatus()
{
	return diting_signal_stop_flag;
}

static 
void diting_signal_module_setpid(pid_t pid)
{
	diting_signal_hijack_pid = pid;	
}

static
void diting_signal_module_stop(void)
{
	if(kill(diting_signal_hijack_pid, SIGUSR1))
		kill(diting_signal_hijack_pid, SIGUSR2);

	waitpid(diting_signal_hijack_pid, NULL, 0);
}


struct diting_signal_module diting_signal_module = {
	.init		= diting_signal_module_init,
	.getstatus	= diting_signal_module_getstatus,
	.setpid		= diting_signal_module_setpid,
	.stop		= diting_signal_module_stop,
};

