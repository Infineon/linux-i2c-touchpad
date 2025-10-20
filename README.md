# Linux I2C Touchpad Driver

## Overview

The **Linux I2C Touchpad Driver** simplifies the integration of I2C-based touchpad devices into Linux systems. It provides a robust and efficient implementation for communicating with touchpad hardware over the I2C bus. Designed for compatibility with Raspberry Pi and other ARM-based platforms, this driver ensures seamless operation and adherence to Linux kernel standards. Infineon offers this driver as an open-source solution to facilitate rapid development and deployment in embedded systems.

### Features:

* Linux kernel module for Touchpad based on PSOC4xxxT CapSense I2C devices
* Automatic device registration via Device Tree overlay
* Sysfs interface for device configuration, control, and status
* Debugfs interface for raw data from touchpad
* Integration with Linux input subsystem for touch event reporting
* Interrupt handling for CapSense events
* Netlink notifications for interrupt events
* Added DFU functionality for updating touchpad firmware via I2C
* Compatible with Raspberry Pi and other ARM-based platforms
* Open-source and easy to integrate into embedded Linux systems

### Supported input and touch events:
* Single touch detection (X, Y, Z/pressure)
* Multi-touch detection (up to 2 fingers)
* Touch release detection
* One-finger tap (single click) gesture
* One-finger double-tap (double click) gesture
* One-finger swipe/flick gestures (up, down, left, right)
* Touchdown and liftoff events

## License

This kernel module is dual-licensed under either:
 - [GPL-2.0](LICENSES/GPL-2.0.txt)
 - [MIT](LICENSES/MIT.txt)

at your option.

### MIT licensed components:

The following files (from the `cybootloaderutils` library) were originally taken from another project at Cypress Semiconductor Corporation (an Infineon company), where they were dual-licensed under Apache-2.0 OR MIT.

**For Linux kernel module purposes, only the MIT license applies**, as the Apache-2.0 license is not compatible with the Linux kernel.

These files have been **adapted** for use in the Linux kernel (functional changes only; no change to their MIT licensing terms):
  - `src/cybootloaderutils/cybtldr_api.c`
  - `src/cybootloaderutils/cybtldr_api2.c`
  - `src/cybootloaderutils/cybtldr_command.c`
  - `src/cybootloaderutils/cybtldr_parse.c`
  - `include/cybootloaderutils/cybtldr_api.h`
  - `include/cybootloaderutils/cybtldr_command.h`
  - `include/cybootloaderutils/cybtldr_utils.h`
  - `include/cybootloaderutils/cybtldr_api2.h`
  - `include/cybootloaderutils/cybtldr_parse.h`

**Origin:**
These files were imported from the **DFU Host Tool** project.

## More information

For more information, refer to the following documents:
* [Linux I2C Touchpad driver Reference Guide](./doc/REFERENCE_GUIDE.md)
* [Release Notes](./RELEASE.md)
* Compatible CE: [mtb-example-psoc4-msclp-smart-glass-touchpad](https://github.com/Infineon/mtb-example-psoc4-msclp-smart-glass-touchpad/)

---
© 2025, Infineon Technologies AG, or an affiliate of Infineon Technologies AG. All rights reserved.
