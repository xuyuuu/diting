#include <linux/types.h>
#include <linux/fs.h>
#include <linux/security.h> 
#include <linux/mount.h> 
#include <linux/list.h> 
#include <linux/fs_struct.h> 
#include <linux/sched.h>
#include <linux/net.h>
#include <asm/current.h> 
#include <asm/signal.h>
#include <linux/dcache.h> 
#include <linux/fs_struct.h>
#include <linux/stat.h>
#include <linux/version.h>
#include <net/sock.h>

#include "diting_killer.h"
#include "diting_util.h"
#include "diting_nolockqueue.h"

static struct diting_killer_table_s{
	int nsig;
	char *ssig;
} diting_killer_table[] = {
	{SIGHUP, "SIGHUP"},
	{SIGINT, "SIGINT"},
	{SIGQUIT, "SIGQUIT"},
	{SIGILL, "SIGILL"},
	{SIGTRAP, "SIGTRAP"},
	{SIGABRT, "SIGABRT"},
	{SIGIOT, "SIGIOT"},
	{SIGBUS, "SIGBUS"},
	{SIGFPE, "SIGFPE"},
	{SIGKILL, "SIGKILL"},
	{SIGUSR1, "SIGUSR1"},
	{SIGSEGV, "SIGSEGV"},
	{SIGUSR2, "SIGUSR2"},
	{SIGPIPE, "SIGPIPE"},
	{SIGALRM, "SIGALRM"},
	{SIGSTKFLT, "SIGSTKFLT"},
	{SIGCHLD, "SIGCHLD"},
	{SIGCONT, "SIGCONT"},
	{SIGSTOP, "SIGSTOP"},
	{SIGTSTP, "SIGTSTP"},
	{SIGTTIN, "SIGTTIN"},
	{SIGTTOU, "SIGTTOU"},
	{SIGURG, "SIGURG"},
	{SIGXCPU, "SIGXCPU"},
	{SIGXFSZ, "SIGXFSZ"},
	{SIGVTALRM, "SIGVTALRM"},
	{SIGPROF, "SIGPROF"},
	{SIGWINCH, "SIGWINCH"},
	{SIGIO, "SIGIO"},
	{SIGPOLL, "SIGPOLL"},
	{SIGPWR, "SIGPWR"},
	{SIGSYS, "SIGSYS"},
	{SIGUNUSED, "SIGUNUSED"},
	{SIGRTMIN, "SIGRTMIN"},
	{SIGRTMAX, "SIGRTMAX"},
};


int diting_module_inside_task_kill(struct task_struct *p, struct siginfo *info, int sig, u32 secid)
{

	return 0;
}



