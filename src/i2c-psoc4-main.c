// SPDX-License-Identifier: GPL-2.0
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

#include "i2c-psoc4-driver.h"

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
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cypress Semiconductor Corporation (an Infineon company)");
MODULE_DESCRIPTION("I2C Touchpad driver for solution based on PSOC4xxxT MCU and CapSense MW");
MODULE_VERSION("1.0.0");
