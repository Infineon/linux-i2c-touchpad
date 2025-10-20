/* SPDX-License-Identifier: GPL-2.0 OR MIT */
/*
 * Copyright (C) 2025, Infineon Technologies AG, or an affiliate of Infineon Technologies AG.
 * All rights reserved.
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

#ifndef I2C_REG_MAP_H
#define I2C_REG_MAP_H

/* Register map for the I2C Touchpad device */

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
#define REG_STORED_FLAG			0x39
#define REG_NUM_SNS				0x3A
#define REG_SNS_RAW				0x3B
#define REG_SNS_BSLN(x)			(REG_SNS_RAW + REG_SNS_RAW_SIZE(x))
#define REG_SNS_CP_MEASURE(x)	(REG_SNS_RAW + REG_SNS_RAW_SIZE(x) + REG_SNS_BSLN_SIZE(x))

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
#define REG_STORED_FLAG_SIZE				1
#define REG_NUM_SNS_SIZE					1
#define REG_SNS_RAW_SIZE(x)					(2 * x)
#define REG_SNS_BSLN_SIZE(x)				(2 * x)
#define REG_SNS_CP_MEASURE_SIZE(x)			(4 * x)

#endif /* I2C_REG_MAP_H */
