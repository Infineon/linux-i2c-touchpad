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

#ifndef PSOC4_I2C_H
#define PSOC4_I2C_H

#include <linux/i2c.h>

/* Function prototypes for PSOC4 I2C operations */
int i2c_safe_transfer(struct i2c_client *client, struct i2c_msg *msgs, int num);
int psoc4_read_register(struct i2c_client *client, u8 reg_address, u8 *buffer, int count);
int psoc4_write_register(struct i2c_client *client, u8 reg_address, const u8 *data, int count);
int psoc4_read_xyz_coords(struct i2c_client *client, u8 reg, u16 *x, u16 *y, u16 *z);
int psoc4_read_gestures(struct i2c_client *client, u32 *gestures);

#endif // PSOC4_I2C_H
