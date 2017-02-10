#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include <sys/types.h>

#include "diting_signal.h"

static int diting_signal_stop_flag = 0;

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
	signal(SIGUSR1, diting_signal_inside_handler);
	signal(SIGUSR2, diting_signal_inside_handler);

	sigset_t set, old;
	sigemptyset(&set);
        sigfillset(&set);
        sigdelset(&set, SIGUSR1);
        sigdelset(&set, SIGUSR2);
	sigprocmask(SIG_BLOCK, &set, &old);

	return 0;
}

static 
int diting_signal_module_getstatus()
{
	return diting_signal_stop_flag;
}

struct diting_signal_module diting_signal_module = {
	.init		= diting_signal_module_init,
	.getstatus	= diting_signal_module_getstatus
};

