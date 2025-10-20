
DEFINES := $(foreach opt,$(BUILD_OPTIONS),-D$(opt))
EXTRA_CFLAGS += $(DEFINES) -I$(INCLUDE_DIR)

all: oldconfig prepare modules_prepare build-driver dt
	@echo "Running make all..."

prepare:
	make -C $(KERNEL_SOURCES) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) prepare

oldconfig:
	yes '' | make -C $(KERNEL_SOURCES) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) oldconfig

modules_prepare:
	make -C $(KERNEL_SOURCES) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules_prepare -j8

build-driver:
	make -C $(KERNEL_SOURCES) M=$(SRC_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules
	mv $(SRC_DIR)/i2c-psoc4-driver.ko $(OUT_DIR)/i2c-psoc4-driver.ko

dt:
	dtc -I dts -O dtb -o $(OUT_DIR)/psoc4-capsense.dtbo $(SRC_DIR)/psoc4-capsense.dts

clean:
	make -C $(KERNEL_SOURCES) M=$(SRC_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
