#ifndef __diting_door_h__ 
#define __diting_door_h__

struct diting_door_module
{
	ulong	(* bitopen)(void);
	void	(* bitclose)(ulong);
	int	(* interfaceset)(struct security_operations *security_point);
	int	(* interfacereset)(struct security_operations * security_point);
};

extern struct diting_door_module diting_door_module;


#endif
