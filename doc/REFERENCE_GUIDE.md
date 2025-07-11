# Linux I2C Touchpad Driver

> ⚠️ **Attention:** This driver was tested with Raspberry Pi 4 (RPi4) only and has not been tested with an I2C clock speed of 1 MHz.

## Build Instructions
For detailed build instructions, refer to the [HOW_TO_BUILD.md](HOW_TO_BUILD.md) file.

## Using Instructions

### 1. Device Registration
The device is automatically registered when the driver is loaded, provided the corresponding device tree overlay (`psoc4-capsense.dtbo`) is applied.

#### Using the Device Tree Overlay

The `psoc4-capsense.dtbo` file is used to configure the device tree for the PSOC4 CapSense driver. Below are the options for using the `.dtbo` file:

#### 1. Apply the Overlay Temporarily
To apply the overlay without rebooting, use the following command:
```bash
sudo dtoverlay psoc4-capsense.dtbo
```

#### 2. Remove the Overlay
To remove the overlay after it has been applied:
```bash
sudo dtoverlay -r psoc4-capsense
```

#### 3. Apply the Overlay Permanently
To ensure the overlay is applied automatically on system boot, add the overlay to the `/boot/config.txt` file:
```bash
echo "dtoverlay=psoc4-capsense" | sudo tee -a /boot/config.txt
```

After adding the overlay, reboot the system for the changes to take effect:
```bash
sudo reboot
```

> **Note:** Ensure the `psoc4-capsense.dtbo` file is located in the `/boot/overlays/` directory before applying it. You can copy the file to this directory using:
```bash
sudo cp <path_to_dtbo>/psoc4-capsense.dtbo /boot/overlays/
```

### 2. Loading and Unloading the Driver
To load the driver module (`i2c-psoc4-driver.ko`) located in your directory:
```bash
sudo insmod <path_to_your_dir>/i2c-psoc4-driver.ko
```

To unload the driver module:
```bash
sudo rmmod i2c-psoc4-driver
```

#### Checking Driver Logs
After loading or unloading the driver, you can check the kernel logs for messages related to the driver using the `dmesg` command.

### 3. Supported Sysfs Attributes
| Attribute          | Access Type | Description                                                                                     | Example Usage                                                                 | Option list
|---------------------|-------------|-------------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------|------------------------------------------
| `fw_ver`           | Read-only   | Displays the firmware version of the device.                                                   | `cat /sys/bus/i2c/devices/i2c-1/psoc4-capsense/fw_ver`                       | N/A |
| `rst_cause`        | Read-only   | Displays the reset cause of the device.                                                        | `cat /sys/bus/i2c/devices/i2c-1/psoc4-capsense/rst_cause`                    | 0x01: WDT reset<br>0x08: Protection Fault<br>0x10: Software reset <br> 0x00: Voltage reset (XRES, BOD, or Normal POR) |
| `reset`            | Write-only  | Triggers a software reset.                                                                     | `sudo sh -c 'echo "1" > /sys/bus/i2c/devices/i2c-1/psoc4-capsense/reset'`    | 1 is the only valid option, as setting it to 1 simply triggers the execution of this action. |
| `save_capsense`    | Write-only  | Saves the CapSense configuration.                                                              | `sudo sh -c 'echo "1" > /sys/bus/i2c/devices/i2c-1/psoc4-capsense/save_capsense'` | 1 is the only valid option, as setting it to 1 simply triggers the execution of this action. |
| `restore_capsense` | Write-only  | Restores the CapSense configuration.                                                           | `sudo sh -c 'echo "1" > /sys/bus/i2c/devices/i2c-1/psoc4-capsense/restore_capsense'` | 1 is the only valid option, as setting it to 1 simply triggers the execution of this action. |
| `cp_test`          | Write-only  | Triggers a Cp Test.                                                                            | `sudo sh -c 'echo "1" > /sys/bus/i2c/devices/i2c-1/psoc4-capsense/cp_test'`  | 1 is the only valid option, as setting it to 1 simply triggers the execution of this action. |
| `short_test`       | Write-only  | Triggers a Short Test.                                                                         | `sudo sh -c 'echo "1" > /sys/bus/i2c/devices/i2c-1/psoc4-capsense/short_test'` | 1 is the only valid option, as setting it to 1 simply triggers the execution of this action. |
| `bootloader_jump`  | Write-only  | Triggers a jump to the bootloader.                                                             | `sudo sh -c 'echo "1" > /sys/bus/i2c/devices/i2c-1/psoc4-capsense/bootloader_jump'` | 1 is the only valid option, as setting it to 1 simply triggers the execution of this action. |
| `test_status`      | Read-only   | Displays the test status and shorted sensor ID if applicable.                                  | `cat /sys/bus/i2c/devices/i2c-1/psoc4-capsense/test_status`                  | Test status:<br>0x00: Success<br>0x01: Reserved<br>0x02: CAPSENSE™ hardware was busy<br>0x05: Test failed to complete<br>0x0F: Short detected (Short test only)<br>Indicates a short on the IO ID if TEST_STATUS = 0x0F<br>0x000001: D1<br>0x000002: C3<br>0x000004: C2<br>0x000008: C1<br>0x000010: B1<br>0x000020: B2<br>0x000040: A4<br>0x000080: B4<br>0x000100: A5<br>0x000200: B5<br>0x000400: C5<br>0x000800: C4<br>0x001000: D5<br>0x002000: E5<br>0x004000: D4<br>0x008000: E4<br>0x010000: D3<br>0x020000: E3<br>0x040000: D2<br>0x080000: E2<br>0x100000: E1 |
| `int_src_en`       | Read/Write  | Enables or disables interrupt sources.                                                         | `sudo sh -c 'echo "1F" > /sys/bus/i2c/devices/i2c-1/psoc4-capsense/int_src_en'` | 0x01: Scan Frame Result Ready<br>0x02: Touch Detected<br>0x04: Test Result Ready<br>0x08: Sensing App Running<br>0x10: Gesture Detected<br>0x80: Application Error<br><br>Default: 0x9F (All Enabled) |
| `int_status`       | Read/Write  | Displays or clears interrupt status.                                                           | `sudo sh -c 'echo "00" > /sys/bus/i2c/devices/i2c-1/psoc4-capsense/int_status'` | 0x00: No pending interrupts<br>0x01: Scan-Complete<br>0x02: Touch Detected<br>0x04: Test Result Ready<br>0x08: Sensing App Running<br>0x10: Gesture Detected<br>0x80: Application Error<br><br>Default: 0x00 |
| `error_status`     | Read-only   | Displays the error status of the device.                                                      | `cat /sys/bus/i2c/devices/i2c-1/psoc4-capsense/error_status`                 | 0x00: No errors<br>0x01: Requested parameter is invalid<br>0x02: I2C Timeout Expired<br>0x04: CAPSENSE Auto-Calibration Failed |
| `scan_mode`        | Read-only   | Displays the current scan mode of the device.                                                  | `cat /sys/bus/i2c/devices/i2c-1/psoc4-capsense/scan_mode`                    | 0x00: Not scanning<br>0x01: Active scanning<br>0x02: Active-Low-Refresh (ALR) rate scanning<br>0x04: Wake-on-touch scanning<br> |
| `shield_en`        | Read/Write  | Enables or disables the shield.                                                               | `sudo sh -c 'echo "01" > /sys/bus/i2c/devices/i2c-1/psoc4-capsense/shield_en'` | 0x00: Disables the shield<br>0x01: Enables the shield<br><br>Default: 0x00 |
| `wear_det_en`      | Read/Write  | Enables or disables wear detection.                                                           | `sudo sh -c 'echo "01" > /sys/bus/i2c/devices/i2c-1/psoc4-capsense/wear_det_en'` | 0x00: None enabled<br>0x01: CSD0<br>0x02: CSD1<br>0x04: CSD2<br>0x08: CSD3<br>0x10: CSD4<br>0x20: CSD5<br><br>Default: 0x00 |
| `sns_auto_cal_en`  | Read/Write  | Enables or disables sensor auto-calibration.                                                  | `sudo sh -c 'echo "01" > /sys/bus/i2c/devices/i2c-1/psoc4-capsense/sns_auto_cal_en'` | 0x00: Auto-Calibration is Disabled<br>0x01: Auto-Calibration is Enabled<br><br>Default: 0x01 |
| `sns_filt_cfg`     | Read/Write  | Configures sensor filtering.                                                                   | `sudo sh -c 'echo "1234" > /sys/bus/i2c/devices/i2c-1/psoc4-capsense/sns_filt_cfg'` | Bit 0: Median filter<br>Bit 1: Average filter<br>Bit 2: IIR filter<br>Bits 8-15: SW IIR Coefficient, if 0, SW IIR filter is not applied<br><br>Default: 0x0000 |
| `sns_ref_rate_act` | Read/Write  | Configures the refresh rate of the sensors in active mode.                                     | `sudo sh -c 'echo "05" > /sys/bus/i2c/devices/i2c-1/psoc4-capsense/sns_ref_rate_act'` | Max: 0xFF<br>Min: 0x01<br><br>Default: 0x3C |
| `sns_ref_rate_alr` | Read/Write  | Configures the refresh rate of the sensors in low-refresh mode.                                | `sudo sh -c 'echo "06" > /sys/bus/i2c/devices/i2c-1/psoc4-capsense/sns_ref_rate_alr'` | Max: 0xFF<br>Min: 0x01<br><br>Default: 0x3C |
| `touch0_pos`       | Read-only   | Displays the position of the first touch point (x, y, z).                                      | `cat /sys/bus/i2c/devices/i2c-1/psoc4-capsense/touch0_pos`                   |  N/A |
| `touch1_pos`       | Read-only   | Displays the position of the second touch point (x, y, z).                                     | `cat /sys/bus/i2c/devices/i2c-1/psoc4-capsense/touch1_pos`                   | N/A |
| `num_touch`   | Read-only   | The number of detected touches.                                                					    | `cat /sys/bus/i2c/devices/i2c-1/psoc4-capsense/num_touch`               | N/A |
| `sns_raw`          | Read-only   | Displays the raw counts of the enabled sensors.                                                | `cat /sys/bus/i2c/devices/i2c-1/psoc4-capsense/sns_raw`                      | N/A |
| `sns_bsln`         | Read-only   | Displays the baseline values of the enabled sensors.                                           | `cat /sys/bus/i2c/devices/i2c-1/psoc4-capsense/sns_bsln`                     | N/A |
| `sns_cp_measure`   | Read-only   | Displays the capacitance measurements (in fF) of the enabled sensors.                          | `cat /sys/bus/i2c/devices/i2c-1/psoc4-capsense/sns_cp_measure`               | N/A |

> **Note:** The `sudo sh -c` command is used here because writing to sysfs attributes typically requires elevated permissions. Directly using `echo "1F" > /sys/bus/i2c/devices/i2c-1/psoc4-capsense/int_src_en` would fail due to permission restrictions, as the redirection (`>`) is handled by the shell, which may not have the necessary privileges. The `sudo sh -c` ensures that both the `echo` command and the redirection are executed with root permissions.

### 4. Linux input subsystem integration
The driver integrates with the Linux input subsystem and registers an input device named `PSOC4 Touchpad`. Touch and gesture events are reported to user space via standard input event interfaces, making the device compatible with existing Linux tools and applications (such as `evtest`, `libinput`, and graphical environments).

#### Touch Data Reporting
- Supports up to 2 simultaneous touch points (multi-touch).
- Each touch point reports X, Y, and Z (pressure) coordinates.
- The driver uses both legacy (ABS_X, ABS_Y, ABS_PRESSURE) and multi-touch (ABS_MT_POSITION_X, ABS_MT_POSITION_Y, ABS_MT_PRESSURE) axes.

#### Gesture Event Reporting
- Single and double tap gestures are mapped to standard Linux key events (e.g., `KEY_PLAYPAUSE`, `KEY_SHUFFLE`).
- Swipe/flick gestures in all four directions are mapped to key events (e.g., `KEY_VOLUMEUP`, `KEY_VOLUMEDOWN`, `KEY_REWIND`, `KEY_FASTFORWARD`).
- Touchdown and liftoff are mapped to `BTN_TOUCH` events.
- All gesture events are reported instantly to the input subsystem, allowing user-space applications to react accordingly.

> **Note:** The Linux key codes used for gesture events (e.g., `KEY_PLAYPAUSE`, `KEY_SHUFFLE`, `KEY_VOLUMEUP`, etc.) are defined in `i2c-psoc4-driver.h` and may be remapped as needed for your application.

#### Example: Testing with evtest

To observe touch and gesture events, use the `evtest` utility:

```bash
sudo evtest
```

Select the `PSOC4 Touchpad` device from the list and interact with the touchpad to see reported events.

---
© 2025, Cypress Semiconductor Corporation (an Infineon company) or an affiliate of Cypress Semiconductor Corporation.
