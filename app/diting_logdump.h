#ifndef __diting_logdump_h__
#define __diting_logdump_h__

#define DITING_LOGDUMP_PROC "/var/log/diting/proc"
#define DITING_LOGDUMP_ACCESS "/var/log/diting/access"
#define DITING_LOGDUMP_KILLER	"/var/log/diting/killer"
#define DITING_LOGDUMP_SOCKET	"/var/log/diting/socket"
#define DITING_LOGDUMP_CHROOT	"/var/log/diting/chroot"

#define DITING_LOGDUMP_PATTERN	"diting"

#define DITING_LOGDUMP_RING_HEIGH (4 << 10)

#define DITING_LOGDUMP_MAX_SIZE (50 << 20)

struct diting_logdump_module
{
	int (*init)(void);
	int (*run)(void);
	int (*stop)(void);
	int (*push)(char *msg, ...);
};

extern struct diting_logdump_module diting_logdump_module;


#endif
