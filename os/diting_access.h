#ifndef __diting_access_h__
#define __diting_access_h__

extern int diting_dentry_has_permission(struct task_struct*task, struct dentry *new_dentry, struct dentry *old_dentry, int mode, int type, const char *arg);

#endif
