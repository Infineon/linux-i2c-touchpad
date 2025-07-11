# Linux I2C Touchpad Driver

## Overview

The **Linux I2C Touchpad Driver** simplifies the integration of I2C-based touchpad devices into Linux systems. It provides a robust and efficient implementation for communicating with touchpad hardware over the I2C bus. Designed for compatibility with Raspberry Pi and other ARM-based platforms, this driver ensures seamless operation and adherence to Linux kernel standards. Infineon offers this driver as an open-source solution to facilitate rapid development and deployment in embedded systems.

### Features:

* Linux kernel module for Touchpad based on PSOC4xxxT CapSense I2C devices
* Automatic device registration via Device Tree overlay
* Sysfs interface for device configuration, control, and status
* Integration with Linux input subsystem for touch event reporting
* Interrupt handling for CapSense events
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

## More information

For more information, refer to the following documents:
* [Linux I2C Touchpad driver Reference Guide](./doc/REFERENCE_GUIDE.md)
* [Release Notes](./RELEASE.md)

---
© 2025, Cypress Semiconductor Corporation (an Infineon company) or an affiliate of Cypress Semiconductor Corporation.
