KERNELDIR := /home/haika/rk3506_vanxoak_sdk/rk3506_linux6.1_sdk_v1.2.0_iot_evm/kernel-6.1
CURRENT_PATH := $(shell pwd)
obj-m := newchrled.o

ARCH := arm
CROSS_COMPILE ?= /home/haika/rk3506_vanxoak_sdk/rk3506_linux6.1_sdk_v1.2.0_iot_evm/prebuilts/gcc/linux-x86/arm/gcc-arm-10.3-2021.07-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
	