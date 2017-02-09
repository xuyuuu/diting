#ifndef __diting_util_h__
#define __diting_util_h__

#define		EOF	-1
#define DITING_SYMBOL_DEFAULT_ADDRESS		"/proc/kallsyms"

#define DITING_FULLFILE_TASK_TYPE      0
#define DITING_FULLFILE_ACCESS_TYPE    1

#define DITING_MAY_EXEC               1
#define DITING_MAY_WRITE              2
#define DITING_MAY_READ               4       
#define DITING_MAY_APPEND             8       
#define DITING_MAY_ACCESS             16
#define DITING_MAY_OPEN               32
#define DITING_MAY_CHDIR              64
#define DITING_MAY_DELETE             256
#define DITING_MAY_RENAME             512

extern char *diting_common_filp_fgets(char *str, int size, struct file *filp); 
extern char * diting_common_get_name(struct task_struct *task, 
		char* name, struct dentry *dentry, int type);
extern int diting_common_getuser(struct task_struct *p, char *username);

#endif
