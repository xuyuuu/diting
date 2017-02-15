obj-m := diting.o
diting-objs := diting_start.o diting_nolockqueue.o\
	diting_util.o diting_bprm.o diting_ktask.o\
	diting_door.o diting_access.o diting_sysctl.o\
	diting_sockmsg.o diting_procfile.o diting_accessfile.o\
	diting_killerfile.o diting_sockfile.o diting_killer.o\



KERNELDIR = /lib/modules/$(shell uname -r)/build/
KERNELBIT = $(shell getconf LONG_BIT)
EXTRA_CFLAGS += -D"__"$(KERNELBIT)"bit__" -Wall -DSYSINFO_KERNEL_VERSION=\"V0.0.1\"
PWD = $(shell pwd)

default:
	@echo $(EXTRA_CFLAGS)
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	strip --strip-debug diting.ko
	chmod +x diting.ko

OBJFILE = .*.cmd *.mod.c .*.o.cmd *.o *.ko\
	  *.ko.* modules.order Module.markers Module.symvers
clean:
	rm -f $(OBJFILE)




