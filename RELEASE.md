# Linux I2C Touchpad Driver

Refer to the [README.md](./README.md) and the Linux I2C Touchpad driver [Reference Guide](./doc/REFERENCE_GUIDE.md) for a complete description.

## What's Changed?
- Added new interrupt source to report Touchdown and Liftoff events
- Added Makefile options to enable configurable builds
- Updates to user documentation
- Copyright's updated

## Known Issues or Limitations
- Tested only on Raspberry Pi 4 (RPi4)
- Not validated with I2C clock speeds above 400 kHz (1 MHz not tested)

## Defect Fixes
- No bug fixes

## Supported Software and Tools

This driver was validated with the following software and tools:

| Software/Tool                | Version         |
|------------------------------|-----------------|
| Linux Kernel                 | 6.12       	 |
| Device Tree Compiler (dtc)   | 1.6.1           |
| Arm GNU Toolchain            | 14.2.1 20241119 |

## More Information

For more information, refer to the following documents:

* [Linux I2C Touchpad driver README](./README.md)
* [Linux I2C Touchpad driver Reference Guide](./doc/REFERENCE_GUIDE.md)

---
© 2025, Infineon Technologies AG, or an affiliate of Infineon Technologies AG. All rights reserved.
