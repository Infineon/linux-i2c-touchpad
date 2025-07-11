# Makefile for building a Linux kernel module

OUT_DIR := $(PWD)/output
$(shell mkdir -p $(OUT_DIR))

SRC_DIR := $(PWD)/src
INCLUDE_DIR := $(PWD)/include

all: oldconfig prepare modules_prepare build-driver dt
	@echo "Running make all..."

prepare:
	make -C $(KERNEL_SOURCES) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) prepare

oldconfig:
	yes '' | make -C $(KERNEL_SOURCES) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) oldconfig

modules_prepare:
	make -C $(KERNEL_SOURCES) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules_prepare -j8

build-driver:
	make -C $(KERNEL_SOURCES) M=$(SRC_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) EXTRA_CFLAGS=-I$(INCLUDE_DIR) modules
	mv $(SRC_DIR)/i2c-psoc4-driver.ko $(OUT_DIR)/i2c-psoc4-driver.ko

dt:
	dtc -I dts -O dtb -o $(OUT_DIR)/psoc4-capsense.dtbo $(SRC_DIR)/psoc4-capsense.dts

clean:
	make -C $(KERNEL_SOURCES) M=$(SRC_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
