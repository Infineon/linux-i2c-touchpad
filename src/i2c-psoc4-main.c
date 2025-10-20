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

#include "i2c-psoc4-driver.h"

#if defined(TOUCHDOWN_LIFTOFF_ON_GESTURE) && defined(TOUCHDOWN_LIFTOFF_ON_IRQ)
#error "Only one of TOUCHDOWN_LIFTOFF_ON_GESTURE or TOUCHDOWN_LIFTOFF_ON_IRQ can be defined at a time!"
#endif

int init_psoc4_config(struct i2c_client *client)
{
	int ret;
	u8 int_src_en;

	// Initial config for this register
	// Read REG_INT_SRC_EN
	ret = psoc4_read_register(client, REG_INT_SRC_EN, &int_src_en, REG_INT_SRC_EN_SIZE);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to read INT_SRC_EN register\n");
		return ret;
	}
	// Clear bit 0 in REG_INT_SRC_EN
	int_src_en &= ~0x01;
	// Write back to REG_INT_SRC_EN
	ret = psoc4_write_register(client, REG_INT_SRC_EN, &int_src_en, REG_INT_SRC_EN_SIZE);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to write INT_SRC_EN register\n");
		return ret;
	}
	dev_dbg(&client->dev, "Updated INT_SRC_EN register: 0x%02x\n", int_src_en);

	psoc4_irq_clear(client);
	return 0;
}

// Probe function
static int psoc4_i2c_probe(struct i2c_client *client)
{
	int ret;

	dev_info(&client->dev, "Probed device with address 0x%02x\n", client->addr);

	ret = init_psoc4_config(client);
	if (ret) {
		dev_err(&client->dev, "Failed to initialize PSOC4 configuration\n");
		return ret;
	}

	ret = psoc4_sysfs_create(client);
	if (ret) {
		dev_err(&client->dev, "Failed to create sysfs entries\n");
		return ret;
	}

	ret = psoc4_debugfs_create(client);
	if (ret) {
		dev_err(&client->dev, "Failed to create debugfs entries\n");
		return ret;
	}

	ret = psoc4_input_dev_create(client);
	if (ret) {
		dev_err(&client->dev, "Failed to register input device\n");
		return ret;
	}

	ret = psoc4_irq_register(client);
	if (ret) {
		dev_err(&client->dev, "Failed to request IRQ\n");
		return ret;
	}

	return 0;
}

// Remove function
static void psoc4_i2c_remove(struct i2c_client *client)
{
	psoc4_nl_exit();
	psoc4_debugfs_remove();
	psoc4_sysfs_remove(client);
	psoc4_input_dev_remove(client);

	dev_info(&client->dev, "Removed device with address 0x%02x\n", client->addr);
}

// Device Tree match table
static const struct of_device_id psoc4_of_match[] = {
	{ .compatible = "infineon,psoc4-capsense" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, psoc4_of_match);

// I2C driver structure
static struct i2c_driver psoc4_i2c_driver = {
	.driver = {
		.name = "psoc4_capsense",
		.of_match_table = psoc4_of_match,
	},
	.probe = psoc4_i2c_probe,
	.remove = psoc4_i2c_remove,
};

// Register the I2C driver
module_i2c_driver(psoc4_i2c_driver);

// Metadata for the module
MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Cypress Semiconductor Corporation (an Infineon company)");
MODULE_DESCRIPTION("I2C Touchpad driver for solution based on PSOC4xxxT MCU and CapSense MW");
MODULE_VERSION("1.2.0");
