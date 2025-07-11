# How to Build the Linux I2C Driver

This document provides step-by-step instructions to build the Linux I2C driver for your kernel.

## Prerequisites

Before building the driver, ensure the following dependencies and tools are available:

1. **Linux Kernel Sources**:
   - The kernel sources for the target system must be available. These are required to build the driver against the correct kernel version.
   - Set the `KERNEL_SOURCES` environment variable to the path of the kernel sources.

2. **GNU Make**:
   - Ensure `make` is installed on your system.

3. **GCC Compiler**:
   - If building directly on the target system, ensure the native GCC compiler is installed.

4. **Cross-Compiler** (Optional):
   - If cross-compiling, a GCC cross-compiler for the target architecture is required.
   - Set the `CROSS_COMPILE` environment variable to the prefix of the cross-compiler (e.g., `aarch64-none-linux-gnu-`).

5. **Device Tree Compiler (DTC)**:
   - The `dtc` tool is required to compile the device tree source (`.dts`) files into binary overlays (`.dtbo`).
   - Install it using your package manager (e.g., `sudo apt install device-tree-compiler` on Ubuntu).

6. **Target Architecture**:
   - Specify the target architecture (e.g., `arm`, `arm64`, `x86_64`).
   - Set the `ARCH` environment variable to the target architecture.

## Build Steps

Follow these steps to build the driver:

### 1. Set Up the Environment

#### For Cross-Compilation:
Export the required environment variables:
```bash
export CROSS_COMPILE=<path-to-cross-compiler-prefix>
export ARCH=<target-architecture>
export KERNEL_SOURCES=<path-to-kernel-sources>
```

Example:
```bash
export CROSS_COMPILE=/usr/bin/aarch64-none-linux-gnu-
export ARCH=arm64
export KERNEL_SOURCES=/home/user/linux-kernel
```

#### For Native Build:
If building directly on the target system, set the following:
```bash
export ARCH=$(uname -m)
export KERNEL_SOURCES=<path-to-kernel-sources>
```

Example:
```bash
export ARCH=arm64
export KERNEL_SOURCES=/usr/src/linux
```

### 2. Prepare the Kernel Sources

The kernel sources preparation steps are handled automatically by the `Makefile`. Ensure that the `KERNEL_SOURCES`, `ARCH`, and `CROSS_COMPILE` environment variables are set correctly before proceeding with the build.

### 3. Build the Driver and Device Tree Overlay

Navigate to the root directory of the driver source code and run:
```bash
make
```

This will invoke the top-level `Makefile` and build both the driver and the device tree overlay. The compiled kernel module (`i2c-psoc4-driver.ko`) and the device tree overlay (`psoc4-capsense.dtbo`) will be placed in the `output/` directory.

#### Build Only the Driver Module

If you want to build only the driver module without preparing the kernel sources, you can directly invoke the `build-driver` target:
```bash
make build-driver
```

This will skip the kernel source preparation steps and compile only the driver module. Ensure that the kernel sources are already prepared before using this option.

#### Build Only the Device Tree Overlay

If you want to build only the device tree overlay, you can use the `dt` target:
```bash
make dt
```

This will compile the `psoc4-capsense.dts` file into a `psoc4-capsense.dtbo` file and place it in the `output/` directory.

### 4. Clean the Build Artifacts

To clean the build artifacts, run:
```bash
make clean
```

## Output

After a successful build, the compiled kernel module (`i2c-psoc4-driver.ko`) and the device tree overlay (`.dtbo`) file will be located in the `output/` directory.

## Notes

- Ensure the kernel sources match the kernel version of the target system.
- If you encounter issues, verify that the `CROSS_COMPILE`, `ARCH`, and `KERNEL_SOURCES` variables are set correctly.
- Use `dmesg` to check for any kernel messages related to the driver after loading it.

---
© 2025, Cypress Semiconductor Corporation (an Infineon company) or an affiliate of Cypress Semiconductor Corporation.
