#ifndef __diting_util_h__
#define __diting_util_h__

#ifndef __USERSPACE__

#define		EOF	-1
#define DITING_SYMBOL_DEFAULT_ADDRESS		"/proc/kallsyms"

#define DITING_FULLFILE_TASK_TYPE      0
#define DITING_FULLFILE_ACCESS_TYPE    1

#endif

#define DITING_SOCKMSG_SYN	1024

#define MAY_EXEC 1
#define MAY_WRITE 2
#define MAY_READ 4
#define MAY_APPEND 8
#define MAY_ACCESS 16
#define MAY_OPEN 32

typedef enum DITING_MSGTYPE{
	DITING_PROCRUN = 0,
	DITING_PROCACCESS,
	DITING_KILLER,
	DITING_SOCKET,
	DITING_CHROOT,
}diting_msgtype_t;

typedef enum DITING_SOCKET_OPTYPE{
	DITING_SOCKET_CREATE = 0,
	DITING_SOCKET_LISTEN,
	DITING_SOCKET_ACCEPT,
	DITING_SOCKET_CONNECT,
	DITING_SOCKET_RECVMSG,
	DITING_SOCKET_SENDMSG
}diting_socket_optype;

typedef enum DITING_PROCACCESS_OPTYPE{
	DITING_PROCACCESS_INODE_CREATE = 0,
	DITING_PROCACCESS_INODE_UNLINK,
	DITING_PROCACCESS_INODE_LINK,
	DITING_PROCACCESS_INODE_SYMLINK,
	DITING_PROCACCESS_INODE_RENAME,
	DITING_PROCACCESS_INODE_MKDIR,
	DITING_PROCACCESS_INODE_RMDIR,
	DITING_PROCACCESS_INODE_ACCESS,
	DITING_PROCACCESS_FILE_ACCESS
}diting_procaccess_optype;

struct diting_common_msgnode{
	diting_msgtype_t type;	
}__attribute__((packed));

struct diting_procrun_msgnode{
	diting_msgtype_t type;
	uid_t uid;
	char username[64];
	char proc[1024];
}__attribute__((packed));

struct diting_procaccess_msgnode{
	diting_msgtype_t type;
	int actype;
	int mode;
	uid_t uid;
	char username[64];
	char proc[1024];
	char old_path[1024];
	char new_path[1024];
}__attribute__((packed));

struct diting_killer_msgnode{
	diting_msgtype_t type;
	uid_t uid;
	char username[64];
	char signal[32];
	char proc1[1024];
	char proc2[1024];
}__attribute__((packed));


struct diting_socket_msgnode{
	diting_msgtype_t type;
	int actype;
	uid_t uid;
	char username[64];
	char sockfamily[16];
	char socktype[16];
	char proc[1024];
	uint32_t localaddr;
	uint16_t localport;
	uint32_t remoteaddr;
	uint16_t remoteport;
}__attribute__((packed));


struct diting_chroot_msgnode{
	diting_msgtype_t type;
	uid_t uid;
	char username[64];
	char proc[1024];
}__attribute__((packed));

#ifndef __USERSPACE__
extern char *diting_common_filp_fgets(char *str, int size, struct file *filp); 
extern char * diting_common_get_name(struct task_struct *task, 
		char** name, struct dentry *dentry, int type);
extern int diting_common_getuser(struct task_struct *p, char *username);
extern unsigned long diting_find_symbol_addr(const char *);
#endif

#endif
