#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>

#include "diting_common.h"


int diting_common_get_lockfile(void)
{
	int ret = -1;

	if(access(DITING_COMMON_PID_LOCKFILE, F_OK))
		ret = 0;

	return ret;
}

int diting_common_put_lockfile(int pid)
{
	int ret;
	FILE *fp = NULL;
	char buffer[64] = {0};

	fp = fopen(DITING_COMMON_PID_LOCKFILE, "w+");
	if(!fp){
		ret = -1;
		goto out;
	}

	sprintf(buffer, "%d", pid);
	fwrite(buffer, 1, strlen(buffer), fp);
	if(fp)
		fclose(fp);

	ret = 0;
out:
	return ret;
}
