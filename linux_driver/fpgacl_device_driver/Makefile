obj-m += fpgacl_device_driver_acp.o
obj-m += fpgacl_device_driver_hp0.o
obj-m += fpgacl_device_driver_hp1.o
obj-m += fpgacl_device_driver_hp2.o
obj-m += fpgacl_device_driver_hp3.o


# Path to the Linux kernel, if not passed in as arg, set default.
ifeq ($(KDIR),)
	KDIR := /lib/modules/$(shell uname -r)/build
endif


all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
