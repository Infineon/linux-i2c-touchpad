/* SPDX-License-Identifier: GPL-2.0 OR MIT */
/*
 * Copyright (C) 2025 Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.
 *
 * Licensed under either of
 *
 * GNU General Public License, Version 2.0 <https://www.gnu.org/licenses/gpl-2.0.html>
 * MIT license  <http://opensource.org/licenses/MIT>
 *
 * at your option.
 *
 * When Licensed under the GNU General Public License, Version 2.0 (the "License");
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/gpl-2.0.html>
 *
 * When licensed under the MIT license;
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the “Software”), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef I2C_PSOC4_H
#define I2C_PSOC4_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/fs.h>
#include <linux/stat.h>
#include <linux/namei.h>
#include <linux/delay.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <net/sock.h>

#include "psoc4-i2c.h"
#include "i2c-reg-map.h"
#include "cybootloaderutils/cybtldr_api.h"

// CMD register bit masks
#define CMD_BIT_RESET				(1 << 0) // Bit 0: Trigger a software reset
#define CMD_BIT_SAVE_CAPSENSE		(1 << 1) // Bit 1: Trigger storage of CAPSENSE configuration
#define CMD_BIT_RESTORE_CAPSENSE	(1 << 2) // Bit 2: Trigger restoration of CAPSENSE configuration
#define CMD_BIT_CP_TEST				(1 << 3) // Bit 3: Trigger Cp Test
#define CMD_BIT_SHORT_TEST			(1 << 4) // Bit 4: Trigger Short Test
#define CMD_BIT_BOOTLOADER_JUMP		(1 << 5) // Bit 5: Trigger Bootloader Jump

// INT_STATUS register bit masks
#define INT_STATUS_CLEAR_PENDING	(INT_STATUS_NO_PENDING) // Clear all pending interrupts
#define INT_STATUS_NO_PENDING        0x00 // No pending interrupts
#define INT_STATUS_SCAN_COMPLETE     0x01 // Scan Complete
#define INT_STATUS_TOUCH_DETECTED    0x02 // Touch Detected
#define INT_STATUS_TEST_RESULT_READY 0x04 // Test Result Ready
#define INT_STATUS_SENSING_RUNNING   0x08 // Sensing App Running
#define INT_STATUS_GEST_DETECTED     0x10 // Gesture Detected
#define INT_STATUS_APP_ERROR         0x80 // Application Error

// Gestures status bit masks
#define GEST_ONE_FINGER_SINGLE_CLICK   (1 << 0)  // one-finger single click gesture
#define GEST_ONE_FINGER_DOUBLE_CLICK   (1 << 1)  // one-finger double click gesture
#define GEST_ONE_FINGER_SCROLL         (1 << 4)  // one-finger scroll gesture
#define GEST_ONE_FINGER_FLICK          (1 << 7)  // one-finger flick gesture
#define GEST_TOUCHDOWN                 (1 << 13) // touchdown event
#define GEST_LIFTOFF                   (1 << 14) // liftoff event

// Gesture Direction Bits
// Direction encodings: 0x00 = UP, 0x01 = Down, 0x02 = Right, 0x03 = Left
#define GEST_DIRECTION_UP              0x00
#define GEST_DIRECTION_DOWN            0x01
#define GEST_DIRECTION_RIGHT           0x02
#define GEST_DIRECTION_LEFT            0x03

// Bitfield masks for gesture direction
#define GEST_SCROLL_DIRECTION_MASK     (0x3 << 15) // bits 15-16 for one-finger scroll gesture
#define GEST_FLICK_DIRECTION_MASK      (0x3 << 23) // bits 23-24 for one-finger flick gesture

// Number of touch slots
#define NUM_TOUCH_SLOTS  2

// Packet size for DFU operations
#define PSOC4_DFU_DATA_PACKET_SIZE	32
#define PSOC4_DFU_MAX_TRANSFER_SIZE	32

// Maximum number of retries for DFU operations
#define DFU_BAD_STATUS_DATA	0xFF
#define DFU_PACKET_START	0x01
#define DFU_PACKET_END		0x17
#define DFU_MAX_RETRY		10
#define DFU_READ_TIMEOUT_MS	300

// Netlink message type
#define NETLINK_USER_TYPE	31
#define NETLINK_GROUP 		1
#define NETLINK_MSG_LEN		64

// Touch coordinates structure
struct psoc4_touch {
	u16 x;
	u16 y;
	u16 z;
};

// General functions
int init_psoc4_config(struct i2c_client *client);

// Sysfs functions
int psoc4_sysfs_create(struct i2c_client *client);
void psoc4_sysfs_remove(struct i2c_client *client);

// DebugFS functions
int psoc4_debugfs_create(struct i2c_client *client);
void psoc4_debugfs_remove(void);

// Input subsystem functions
int psoc4_input_dev_create(struct i2c_client *client);
void psoc4_input_dev_remove(struct i2c_client *client);
void psoc4_input_report_coord(struct i2c_client *client, u8 num_touches,
								struct psoc4_touch *touches);
void psoc4_input_report_gesture(struct i2c_client *client, u32 gestures);
void report_instant_event(u32 key_code);

// IRQ functions
int psoc4_irq_register(struct i2c_client *client);
int psoc4_irq_clear(struct i2c_client *client);
int psoc4_touch_detected_handler(struct i2c_client *client);
int psoc4_gesture_detected_handler(struct i2c_client *client);

// Netlink functions
void psoc4_nl_exit(void);

// DFU functions
int psoc4_dfu_start(struct i2c_client *client);
int psoc4_dfu_jump_to_bootloader(struct i2c_client *client);
int psoc4_dfu_program(char *dfu_filepath);
bool psoc4_dfu_get_status(void);

#endif // I2C_PSOC4_H
