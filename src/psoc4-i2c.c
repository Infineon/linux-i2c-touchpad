// SPDX-License-Identifier: GPL-2.0 OR MIT
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

#include <linux/device.h>
#include <linux/kernel.h>
#include "psoc4-i2c.h"
#include "i2c-psoc4-driver.h"
#include <linux/delay.h>

#define MAX_RETRIES 5
#define RETRY_DELAY_MS 5

/* Safe I2C transfer with retries
 * This function attempts to transfer I2C messages with retries in case of errors.
 * It is useful for handling transient errors like bus errors or device not responding.
 */
int i2c_safe_transfer(struct i2c_client *client, struct i2c_msg *msgs, int num)
{
	int ret, retries = 0;

	while (retries < MAX_RETRIES) {
		ret = i2c_transfer(client->adapter, msgs, num);
		if (ret == num)
			return num;

		dev_dbg(&client->dev, "I2C transfer failed (attempt %d, error: %d)\n",
				retries + 1, ret);
		if (ret == -EREMOTEIO || ret == -EIO) {
			msleep(RETRY_DELAY_MS);
			retries++;
			continue;
		}
		return ret;
	}

	dev_err(&client->dev, "I2C transfer failed after %d retries, error: %d\n", retries, ret);
	return ret;
}

/* Reading data from a register */
int psoc4_read_register(struct i2c_client *client, u8 reg_address, u8 *buffer, int count)
{
	int ret;
	struct i2c_msg msgs[2];

	/* Message for sending the register (sub-address) */
	msgs[0].addr = client->addr;
	msgs[0].flags = 0; // Write
	msgs[0].len = 2;   // MSB + LSB
	msgs[0].buf = (u8[]){ 0x00, reg_address & 0xFF }; // Form the sub-address

	/* Message for reading data */
	msgs[1].addr = client->addr;
	msgs[1].flags = I2C_M_RD; // Read
	msgs[1].len = count;      // Number of bytes to read
	msgs[1].buf = buffer;     // Buffer for storing data

	/* Sending the sub-address to the device */
	ret = i2c_safe_transfer(client, msgs, 2);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to read register 0x%04x (I2C error: %d)\n",
				reg_address, ret);
		return ret;
	} else if (ret != 2) {
		dev_err(&client->dev, "Incomplete I2C transfer: expected 2 messages, got %d\n",
				ret);
		return -EIO;
	}

	dev_dbg(&client->dev, "Read %d bytes from 0x%04x successfully\n", count, reg_address);
	return 0;
}

/* Writing data to a register */
int psoc4_write_register(struct i2c_client *client, u8 reg_address, const u8 *data, int count)
{
	int ret;
	u8 *buffer;
	struct i2c_msg msg;

	/* Allocate memory for sub-address + data */
	buffer = kmalloc(count + 2, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;

	/* Form the buffer for writing */
	buffer[0] = 0x00; // MSB
	buffer[1] = reg_address & 0xFF;        // LSB
	memcpy(&buffer[2], data, count);      // Copy data

	/* Message for writing */
	msg.addr = client->addr;
	msg.flags = 0; // Write
	msg.len = count + 2; // MSB + LSB + data
	msg.buf = buffer;

	/* Write data */
	ret = i2c_safe_transfer(client, &msg, 1);

	if (ret < 0) {
		dev_err(&client->dev, "Failed to write to register 0x%04x (I2C error: %d)\n",
				reg_address, ret);
	} else if (ret != 1) {
		dev_err(&client->dev, "Incomplete I2C transfer: expected 1 message, got %d\n",
				ret);
		ret = -EIO;
	} else {
		dev_dbg(&client->dev, "Wrote %d bytes to 0x%04x successfully\n",
				count, reg_address);
		ret = 0;
	}

	kfree(buffer); // Free memory
	return ret;
}

/* Reading X, Y, Z coordinates */
int psoc4_read_xyz_coords(struct i2c_client *client, u8 reg, u16 *x, u16 *y, u16 *z)
{
	int ret;
	u16 buffer[REG_TCH_XYZ_SIZE_WORD];

	/* Read X, Y, Z coordinates in one transaction */
	ret = psoc4_read_register(client, reg, (u8 *)buffer, REG_TCH_XYZ_SIZE_BYTES);
	if (ret < 0)
		return ret;

	*x = le16_to_cpu(buffer[0]);
	*y = le16_to_cpu(buffer[1]);
	*z = le16_to_cpu(buffer[2]);

	return 0;
}

/* Reading gesture detected data */
int psoc4_read_gestures(struct i2c_client *client, u32 *gestures)
{
	int ret;
	u32 value;

	ret = psoc4_read_register(client, REG_GESTURE_DET, (u8 *)&value, REG_GESTURE_DET_SIZE);
	if (ret < 0)
		return ret;

	*gestures = le32_to_cpu(value);

	return 0;
}
