#ifndef __diting_util_h__
#define __diting_util_h__

#ifndef __USERSPACE__

#define		EOF	-1
#define DITING_SYMBOL_DEFAULT_ADDRESS		"/proc/kallsyms"

#define DITING_FULLFILE_TASK_TYPE      0
#define DITING_FULLFILE_ACCESS_TYPE    1

#endif

#define DITING_SOCKMSG_SYN	1024

typedef enum DITING_MSGTYPE{
	DITING_PROCRUN = 0,
	DITING_PROCACCESS,
	DITING_KILLER,
}diting_msgtype_t;

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

#ifndef __USERSPACE__
extern char *diting_common_filp_fgets(char *str, int size, struct file *filp); 
extern char * diting_common_get_name(struct task_struct *task, 
		char** name, struct dentry *dentry, int type);
extern int diting_common_getuser(struct task_struct *p, char *username);
#endif

#endif
