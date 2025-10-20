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
