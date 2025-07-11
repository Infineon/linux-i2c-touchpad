# Linux I2C Touchpad Driver

## What's Included?

This is an initial release provides a Linux kernel driver sources for the I2C Touchpad based on PSOC4000T CapSense solution, enabling I2C communication and integration with the Linux input subsystem.

Refer to the [README.md](./README.md) and the Linux I2C Touchpad driver [Reference Guide](./doc/REFERENCE_GUIDE.md) for a complete description.

## Known Issues or Limitations
- Tested only on Raspberry Pi 4 (RPi4)
- Not validated with I2C clock speeds above 400 kHz (1 MHz not tested)

## Defect Fixes
- Initial release

## Supported Software and Tools

This driver was validated with the following software and tools:

| Software/Tool                | Version         |
|------------------------------|-----------------|
| Linux Kernel                 | 6.10       	 |
| Device Tree Compiler (dtc)   | 1.6.1           |
| Arm GNU Toolchain            | 14.2.1 20241119 |

## More Information

For more information, refer to the following documents:

* [Linux I2C Touchpad driver README](./README.md)
* [Linux I2C Touchpad driver Reference Guide](./doc/REFERENCE_GUIDE.md)

---
© 2025, Cypress Semiconductor Corporation (an Infineon company) or an affiliate of Cypress Semiconductor Corporation.
