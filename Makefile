obj-m:=platform_device_setup.o platform_device_driver.o

KERNEL_PATH = /lib/modules/$(shell uname -r)/build


all:
	make -C $(KERNEL_PATH) M=$(shell pwd) modules
clear:
	make -C $(KERNEL_PATH) M=$(shell pwd) clean

