#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>


#include "diting_multiring.h"
#include "diting_logdump.h"

#define __USERSPACE__
#include "../os/diting_util.h"

static volatile int diting_logdump_run_flag;

static pthread_mutex_t diting_logdump_lock;
static pthread_cond_t diting_logdump_cond;

static struct diting_multiring *diting_logdump_ring;

static char *diting_logpath[] = {
	"/var/log/diting",
	"/var/log/diting/proc",
	"/var/log/diting/access",
	"/var/log/diting/killer",
	"/var/log/diting/socket",
	"/var/log/diting/chroot",
};


static int diting_logdump_module_inside_inside_dump(char *errmsg)
{
        FILE *fp;
	int type;
	struct stat _stat;
	char *tag = "a+";
        char st[128] = {0};
        time_t t = time(NULL);
	char *file = NULL;

        ctime_r(&t, st);
        st[strlen(st) - 1] = '\0';

	memset(&_stat, 0x0, sizeof(_stat));
	if (!stat(file, &_stat) && _stat.st_size > DITING_LOGDUMP_MAX_SIZE)
		tag = "w+";

	/*check file type*/
	type = strtoul(errmsg, NULL, 10);
	if(DITING_PROCRUN == type)
		file = DITING_LOGDUMP_PROC"/"DITING_LOGDUMP_PATTERN".log";
	else if(DITING_PROCACCESS == type)
		file = DITING_LOGDUMP_ACCESS"/"DITING_LOGDUMP_PATTERN".log";
	else if(DITING_KILLER == type)
		file = DITING_LOGDUMP_KILLER"/"DITING_LOGDUMP_PATTERN".log";
	else if(DITING_CHROOT == type)
		file = DITING_LOGDUMP_CHROOT"/"DITING_LOGDUMP_PATTERN".log";
	else if(DITING_SOCKET == type)
		file = DITING_LOGDUMP_SOCKET"/"DITING_LOGDUMP_PATTERN".log";
	else
		goto out;
		
        fp = fopen(file, tag);
        if (fp == NULL)
        {   
                fprintf(stderr, "[%s] --- fopen failed %s.\n",st, file);
                goto out;
        }

        fprintf(fp, "[%s] --- DUMP: %s\n", st, errmsg);
        fclose(fp);

out:
        return 0;
}


static void * diting_logdump_module_inside_dump(void *arg)
{
	/*wait for running*/
	int ret;
	sigset_t set;

	sigfillset(&set);
        if (pthread_sigmask(SIG_SETMASK, &set, NULL) != 0)
	{
		fprintf(stderr, "pthread_sigmask error in diting_logdump_module_inside_dump\n");	
		_exit(-1);
	}

	pthread_cond_wait(&diting_logdump_cond, &diting_logdump_lock);
	pthread_mutex_unlock(&diting_logdump_lock);

	char *msg = NULL;
	while (diting_logdump_run_flag)
	{
		ret = diting_multiring_module.dequeue(diting_logdump_ring, (void **)&msg);		
		if (ret)
		{
			usleep(1000);	
			continue;
		}
		if (msg)
		{
			diting_logdump_module_inside_inside_dump(msg);
			free(msg);	
		}
	}

	return NULL;
}


static int diting_logdump_module_init(void)
{
	int ret = 0, i;
	//char buff[1024] = {0};
	pthread_t diting_logdump_pid;

	diting_logdump_run_flag = 1;

	pthread_mutex_init(&diting_logdump_lock, NULL);
	pthread_cond_init(&diting_logdump_cond, NULL);

	diting_logdump_ring = diting_multiring_module.create(DITING_LOGDUMP_RING_HEIGH);
	if (!diting_logdump_ring)
	{
		ret = -1;	
		goto out;
	}
			

	//sprintf(buff, "mkdir -p %s > /dev/null", DITING_LOGDUMP_CONTENT);
	//system(buff);

	/*make sure getting lock before run*/
	pthread_mutex_lock(&diting_logdump_lock);
	ret = pthread_create(&diting_logdump_pid, NULL, diting_logdump_module_inside_dump, NULL);
        if (ret != 0)
	{
		ret = -1;
		pthread_mutex_unlock(&diting_logdump_lock);
		goto out;
        }    
        pthread_detach(diting_logdump_pid);

	/*init log path*/
	for(i = 0; i < sizeof(diting_logpath) / sizeof(char *); i++){
		if(access(diting_logpath[i], F_OK))	
			mkdir(diting_logpath[i], 0777);
	}

out:
	return ret;
}


static int diting_logdump_module_run(void)
{
	pthread_mutex_lock(&diting_logdump_lock);
	pthread_cond_signal(&diting_logdump_cond);
	pthread_mutex_unlock(&diting_logdump_lock);

	return 0;
}

static int diting_logdump_module_stop(void)
{
	diting_logdump_run_flag = 0;

	return 0;
}

static int diting_logdump_module_push(char *msg, ...)
{
	int ret = 0;
	char *buff = NULL;
	va_list list;
	va_start(list, msg);

	buff = (char *)malloc(2048);
	if (!buff)
	{
		ret = -1;	
		goto out;
	}

	memset(buff, 0x0, 2048);
	vsprintf(buff, msg, list);

	ret = diting_multiring_module.enqueue(diting_logdump_ring, buff);
	if (ret)
	{
		ret = -1;	
		free(buff);
	}

out:
	va_end(list);

	return ret;
}


struct diting_logdump_module diting_logdump_module = 
{
	.init		= diting_logdump_module_init,
	.run		= diting_logdump_module_run,
	.stop		= diting_logdump_module_stop,
	.push		= diting_logdump_module_push,
};



