// SPDX-License-Identifier: GPL-2.0 OR MIT
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

#include "i2c-psoc4-driver.h"
#include "cybootloaderutils/cybtldr_api.h"
#include "cybootloaderutils/cybtldr_api2.h"
#include "cybootloaderutils/cybtldr_command.h"
#include "cybootloaderutils/cybtldr_parse.h"

static void psoc4_dfu_deinit(void) __maybe_unused;
static int psoc4_dfu_is_bootloader_mode(struct CyBtldr_CommunicationsData *comm) __maybe_unused;
static int _dfu_open_connection(void) __maybe_unused;
static int _dfu_close_connection(void) __maybe_unused;
static int _dfu_read_data(u8 *buffer, int size) __maybe_unused;
static int _dfu_write_data(u8 *buffer, int size) __maybe_unused;

static struct CyBtldr_CommunicationsData dfu_comm_data = {
	.OpenConnection = _dfu_open_connection,
	.CloseConnection = _dfu_close_connection,
	.ReadData = _dfu_read_data,
	.WriteData = _dfu_write_data,
	.DataPacketSize = PSOC4_DFU_DATA_PACKET_SIZE,
	.MaxTransferSize = PSOC4_DFU_MAX_TRANSFER_SIZE
};

static struct i2c_client *dfu_client;
static u32 dfu_address;

static bool isPacketStarted = true; // Tracks status of reading response packet
static bool isDfuUpdateSuccess = true; // Tracks if DFU update was successful

int psoc4_dfu_start(struct i2c_client *client)
{
	struct device_node *of_node = client->dev.of_node;
	int ret;

	isPacketStarted = false; // Reset packet status
	isDfuUpdateSuccess = false; // Reset status before starting DFU

	if (!client) {
		dev_err(&client->dev, "Invalid I2C client pointer\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(of_node, "dfu-address", &dfu_address);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to read dfu-address\n");
		return ret;
	}
	dev_dbg(&client->dev, "DFU address: 0x%02x\n", dfu_address);

	dfu_client = client;

	ret = psoc4_dfu_jump_to_bootloader(dfu_client);
	if (ret < 0) {
		dev_err(&dfu_client->dev, "Failed to jump to bootloader\n");
		return ret;
	}

	return 0;
}

int psoc4_dfu_program(char *dfu_filepath)
{
	int ret;

	if (!dfu_filepath) {
		dev_err(&dfu_client->dev, "Invalid DFU file path\n");
		return -EINVAL;
	}

	ret = psoc4_dfu_is_bootloader_mode(&dfu_comm_data);
	if (ret == CYRET_SUCCESS) {
		/* Program */
		ret = CyBtldr_Program(dfu_filepath, &dfu_comm_data, NULL);
		if (ret != CYRET_SUCCESS) {
			dev_err(&dfu_client->dev, "DFU programming failed: %d\n", ret);
			isDfuUpdateSuccess = false;
			return ret;
		}
		dev_info(&dfu_client->dev, "DFU programming succeeded\n");
		isDfuUpdateSuccess = true;
	} else {
		dev_err(&dfu_client->dev, "Device is not in bootloader mode, cannot program\n");
		return -EBUSY;
	}

	psoc4_dfu_deinit();

	return 0;
}

int psoc4_dfu_jump_to_bootloader(struct i2c_client *client)
{
	int ret;

	u8 mask = CMD_BIT_BOOTLOADER_JUMP;

	ret = psoc4_write_register(client, REG_CMD, &mask, 1);
	if (ret < 0)
		return -EIO;

	usleep_range(10000, 11000);

	return 0;
}

bool psoc4_dfu_get_status(void)
{
	return isDfuUpdateSuccess;
}

static void psoc4_dfu_deinit(void)
{
	dfu_client = NULL;
}

static int psoc4_dfu_is_bootloader_mode(struct CyBtldr_CommunicationsData *comm)
{
	int ret;

	ret = CyBtldr_isBootloaderAppActive(comm);
	if (ret < 0) {
		dev_err(&dfu_client->dev, "Target FW is not in Bootloader.\n");
		return ret;
	}

	return ret;
}

static int _dfu_open_connection(void)
{
	if (!dfu_client || !dfu_client->adapter) {
		dev_err(&dfu_client->dev, "DFU I2C adapter not initialized\n");
		return -ENODEV;
	}
	return 0;
}

static int _dfu_close_connection(void)
{
	return 0;
}

static int _dfu_read_data_internal(u8 *buffer, int size)
{
	struct i2c_msg msg = {
		.addr = (u16)dfu_address,
		.flags = I2C_M_RD, // Read
		.len = size,
		.buf = buffer
	};

	int ret;

	ret = i2c_transfer(dfu_client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&dfu_client->dev, "I2C read error: %d\n", ret);
		return ret;
	}

	return 0;
}

static bool _dfu_read_first_good_data(u8 *data)
{
	const u32 MAX_READS = DFU_READ_TIMEOUT_MS / 10;
	u32 numReads = 0;
	bool dataIsGood = false;
	int err;

	// Retry reading one byte until valid data is received or timeout
	while (!dataIsGood && numReads < MAX_READS) {
		err = _dfu_read_data_internal(data, 1); // Read one byte
		if (err < 0) {
			dev_err(&dfu_client->dev, "Error reading first byte: %d\n", err);
			return false;
		}

		dataIsGood = (data[0] != DFU_BAD_STATUS_DATA);
		numReads++;
		if (!dataIsGood)
			usleep_range(10000, 11000); // Delay for 10ms
		else if (data[0] == DFU_PACKET_START)
			isPacketStarted = true;
	}

	return dataIsGood;
}

static int _dfu_read_data(u8 *data, int size)
{
	bool dataIsGood = false;
	int err;

	// Initial read for the full packet
	err = _dfu_read_data_internal(data, size);
	if (err < 0) {
		dev_err(&dfu_client->dev, "Initial read failed: %d\n", err);
		return err;
	}

	int numGoodBytes;
	int i; // Index of the first good data received.

	if (!isPacketStarted) {
		// Process the data to find the first good byte
		for (i = 0; i < size; i++) {
			if (data[i] == DFU_PACKET_START)
				isPacketStarted = true;

			if (data[i] != DFU_BAD_STATUS_DATA)
				break;
		}
		if (i == 0 && data[size - 1] == DFU_PACKET_END)
			isPacketStarted = false;

		if (i != 0) {

			if (i == size) {
				/* All data is invalid, try reading one byte at a time
				 * until we get good data
				 */
				dataIsGood = _dfu_read_first_good_data(data);
				numGoodBytes = 1; // Start with one good byte
			} else {
				// Extract good data from the buffer
				dataIsGood = true;
				numGoodBytes = size - i;
				for (int j = 0; j < numGoodBytes; j++)
					data[j] = data[i + j];
			}

			// Read the rest of the packet if there are remaining bytes
			if (dataIsGood && numGoodBytes < size) {
				err = _dfu_read_data_internal(&data[numGoodBytes],
						size - numGoodBytes);
				if (err < 0) {
					dev_err(&dfu_client->dev,
							"Failed to read remaining data: %d\n",
							err);
					return err;
				}
				if (data[size - 1] == DFU_PACKET_END)
					isPacketStarted = false;
			} else if (!dataIsGood) {
				dev_err(&dfu_client->dev, "No good data received after initial read\n");
				err = 0x01; //OPERATION_TIMEOUT
			}
		}
	} else {
		if (data[size - 1] == DFU_PACKET_END)
			isPacketStarted = false;
	}

	return err;
}

static int _dfu_write_data(u8 *buffer, int size)
{
	int ret;

	struct i2c_msg msg = {
		.addr = (u16)dfu_address,
		.flags = 0, // Write
		.len = size,
		.buf = buffer
	};

	ret = i2c_transfer(dfu_client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&dfu_client->dev, "DFU I2C write error: %d\n", ret);
		return CYRET_ERR_DATA;
	}

	return CYRET_SUCCESS;
}
