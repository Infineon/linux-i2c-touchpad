/* SPDX-License-Identifier: GPL-2.0 */
/*
* Copyright (C) 2025 Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.
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
* along with this program; if not, see <https://www.gnu.org/licenses/>
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

#include "psoc4-i2c.h"

// Register offsets
#define REG_FW_VER              (REG_FW_VER_MAJ)
#define REG_FW_VER_MAJ          0x00
#define REG_FW_VER_MIN          0x01
#define REG_FW_VER_BLD          0x03
#define REG_RST_CAUSE           0x05
#define REG_CMD                 0x06
#define REG_TEST_STATUS         0x08
#define REG_SHORTED_SNS_ID		0x09
#define REG_INT_SRC_EN          0x0D
#define REG_INT_STATUS          0x0E
#define REG_ERROR_STATUS		0x0F
#define REG_SCAN_MODE           0x10
#define REG_SHIELD_EN			0x11
#define REG_WEAR_DET_EN			0x12
#define REG_SNS_AUTO_CAL_EN		0x23
#define REG_SNS_FILT_CFG        0x24
#define REG_SNS_REF_RATE_ACT    0x26
#define REG_SNS_REF_RATE_ALR    0x27
#define REG_TCH0_POS			(REG_TCH0_POS_X)
#define REG_TCH0_POS_X          0x28
#define REG_TCH0_POS_Y          0x2A
#define REG_TCH0_POS_Z          0x2C
#define REG_TCH1_POS			(REG_TCH1_POS_X)
#define REG_TCH1_POS_X          0x2E
#define REG_TCH1_POS_Y          0x30
#define REG_TCH1_POS_Z          0x32
#define REG_NUM_TOUCH			0x34
#define REG_GESTURE_DET			0x35
#define REG_NUM_SNS				0xBA
#define REG_SNS_RAW				0x39
#define REG_SNS_BSLN(x)			(REG_SNS_RAW + 2 * x)
#define REG_SNS_CP_MEASURE(x)	(REG_SNS_RAW + 4 * x)

// Register sizes
#define REG_FW_VER_SIZE		(REG_FW_VER_MAJ_SIZE + \
							REG_FW_VER_MIN_SIZE + \
							REG_FW_VER_BLD_SIZE)
#define REG_FW_VER_MAJ_SIZE					1
#define REG_FW_VER_MIN_SIZE					2
#define REG_FW_VER_BLD_SIZE					2
#define REG_RST_CAUSE_SIZE					1
#define REG_CMD_SIZE						2
#define REG_TEST_STATUS_SIZE				1
#define REG_SHORTED_SNS_ID_SIZE				4
#define REG_INT_SRC_EN_SIZE					1
#define REG_INT_STATUS_SIZE					1
#define REG_ERROR_STATUS_SIZE				1
#define REG_SCAN_MODE_SIZE					1
#define REG_SHIELD_EN_SIZE					1
#define REG_WEAR_DET_EN_SIZE				1
#define REG_SNS_AUTO_CAL_EN_SIZE			1
#define REG_SNS_FILT_CFG_SIZE				2
#define REG_SNS_REF_RATE_ACT_SIZE			1
#define REG_SNS_REF_RATE_ALR_SIZE			1
#define REG_TCH_XYZ_SIZE_WORD				3
#define REG_TCH_XYZ_SIZE_BYTES				(REG_TCH_XYZ_SIZE_WORD * 2)
#define REG_TCH0_POS_X_SIZE					2
#define REG_TCH0_POS_Y_SIZE					2
#define REG_TCH0_POS_Z_SIZE					2
#define REG_TCH1_POS_X_SIZE					2
#define REG_TCH1_POS_Y_SIZE					2
#define REG_TCH1_POS_Z_SIZE					2
#define REG_NUM_TOUCH_SIZE					1
#define REG_GESTURE_DET_SIZE				4
#define REG_NUM_SNS_SIZE					1

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

// Gesture key mappings
#define GEST_SINGLE_CLICK_KEY	KEY_PLAYPAUSE
#define GEST_DOUBLE_CLICK_KEY	KEY_SHUFFLE
#define GEST_SWIPE_UP_KEY		KEY_VOLUMEUP
#define GEST_SWIPE_DOWN_KEY		KEY_VOLUMEDOWN
#define GEST_SWIPE_RIGHT_KEY	KEY_REWIND
#define GEST_SWIPE_LEFT_KEY		KEY_FASTFORWARD
#define GEST_TOUCHDOWN_KEY		BTN_TOUCH
#define GEST_LIFTOFF_KEY		BTN_TOUCH

// Bitfield masks for gesture direction
#define GEST_SCROLL_DIRECTION_MASK     (0x3 << 15) // bits 15-16 for one-finger scroll gesture
#define GEST_FLICK_DIRECTION_MASK      (0x3 << 23) // bits 23-24 for one-finger flick gesture

// Number of touch slots
#define NUM_TOUCH_SLOTS  2

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

#endif // I2C_PSOC4_H
