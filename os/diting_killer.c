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

#include "diting_killerfile.h"
#include "diting_sysctl.h"

static struct diting_killer_table_s{
	int nsig;
	char *ssig;
} diting_killer_tables[] = {
	{0, NULL},
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
	{SIGPWR, "SIGPWR"},
	{SIGSYS, "SIGSYS"},
	{SIGRTMIN, "SIGRTMIN"},
};

int diting_module_inside_task_kill(struct task_struct *p, struct siginfo *info, int sig, u32 secid)
{
	uint32_t status = 0;
	char *psig = NULL, *srcpath = NULL, *dstpath = NULL,
	     *srcname = NULL, *dstname = NULL, username[64] = {0};
	struct task_struct *srcp, *dstp;
	struct diting_killer_msgnode *item;

	if(sig > SIGRTMIN)
		goto out;

	srcp = current;
	dstp = p;
	psig = diting_killer_tables[sig].ssig;

	diting_sysctl_module.chkstatus(DITING_KILLERBEHAVIOR_SWITCH, &status);
	if(!status)
		goto out;

	if(diting_killerfile_module.search(psig))
		goto out;

	srcpath = diting_common_get_name(srcp, &srcname, NULL, DITING_FULLFILE_TASK_TYPE);
	if(!srcpath || IS_ERR(srcpath))
		goto out;
	dstpath = diting_common_get_name(dstp, &dstname, NULL, DITING_FULLFILE_TASK_TYPE);
	if(!dstpath || IS_ERR(dstpath))
		goto out;

	if(diting_common_getuser(current, username))
		strncpy(username, "SYSTEM", sizeof("SYSTEM") - 1);

	item = (struct diting_killer_msgnode *)kmalloc(sizeof(struct diting_killer_msgnode), GFP_KERNEL);
	memset(item, 0x0, sizeof(struct diting_killer_msgnode));
	item->type = DITING_KILLER;
	strncpy(item->signal, psig, strlen(psig));
	strncpy(item->proc1, srcpath, sizeof(item->proc1) - 1);
	strncpy(item->proc2, dstpath, sizeof(item->proc2) - 1);
	if(current->cred && !IS_ERR(current->cred))
		item->uid  = current->cred->uid;
	else
		item->uid = -1;
	strncpy(item->username, username, strlen(username));
	diting_nolockqueue_module.enqueue(diting_nolockqueue_module.getque(), item);

out:
	if(srcpath && !IS_ERR(srcpath))
		kfree(srcname);
	if(dstpath && !IS_ERR(dstpath))
		kfree(dstname);

	return 0;
}



