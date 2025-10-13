// SPDX-License-Identifier: Apache-2.0 OR MIT
/*
 * Copyright 2011-2025 Cypress Semiconductor Corporation (an Infineon company)
 *
 * Licensed under either of
 *
 * Apache License, Version 2.0 <http://www.apache.org/licenses/LICENSE-2.0>)
 * MIT license  <http://opensource.org/licenses/MIT>)
 *
 * at your option.
 *
 * When Licensed under the Apache License, Version 2.0 (the "License");
 *
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

#include "cybootloaderutils/cybtldr_command.h"

/* Variable used to store the currently selected packet checksum type */
enum CyBtldr_ChecksumType CyBtldr_Checksum = SUM_CHECKSUM;

static void fillData16(u8 *buf, u16 data)
{
	buf[0] = (u8)(data);
	buf[1] = (u8)(data >> 8);
}

void fillData32(u8 *buf, u32 data)
{
	fillData16(buf, (u16)data);
	fillData16(buf + 2, (u16)(data >> 16));
}

u16 CyBtldr_ComputeChecksum16bit(u8 *buf, u32 size)
{
	u16 res = 0;

	if (CyBtldr_Checksum == CRC_CHECKSUM) {
		u16 crc = 0xffff;
		u16 tmp;
		int i;

		if (size == 0)
			return (~crc);

		do {
			for (i = 0, tmp = 0x00ff & *buf++; i < 8; i++, tmp >>= 1) {
				if ((crc & 0x0001) ^ (tmp & 0x0001))
					crc = (crc >> 1) ^ 0x8408;
				else
					crc >>= 1;
			}
		} while (--size);

		crc = ~crc;
		tmp = crc;
		crc = (crc << 8) | (tmp >> 8 & 0xFF);

		res = crc;
	} else { /* SUM_CHECKSUM */
		u16 sum = 0;

		while (size-- > 0)
			sum += *buf++;

		res = (1 + ~sum);
	}
	return res;
}

#define G0 0x82F63B78
#define G1 ((G0 >> 1) & 0x7fffffff)
#define G2 ((G0 >> 2) & 0x3fffffff)
#define G3 ((G0 >> 3) & 0x1fffffff)

u32 CyBtldr_ComputeChecksum32bit(u8 *buf, u32 size)
{
	static const u32 table[16] = {
		0,
		(u32)G3,
		(u32)G2,
		(u32)(G2 ^ G3),
		(u32)G1,
		(u32)(G1 ^ G3),
		(u32)(G1 ^ G2),
		(u32)(G1 ^ G2 ^ G3),
		(u32)G0,
		(u32)(G0 ^ G3),
		(u32)(G0 ^ G2),
		(u32)(G0 ^ G2 ^ G3),
		(u32)(G0 ^ G1),
		(u32)(G0 ^ G1 ^ G3),
		(u32)(G0 ^ G1 ^ G2),
		(u32)(G0 ^ G1 ^ G2 ^ G3),
	};

	u8 *data = (u8 *)buf;
	u32 crc = 0xFFFFFFFF;

	while (size != 0) {
		int i;
		--size;
		crc = crc ^ (*data);
		++data;
		for (i = 1; i >= 0; i--)
			crc = (crc >> 4) ^ table[crc & 0xF];
	}
	return ~crc;
}

#undef G0
#undef G1
#undef G2
#undef G3

void CyBtldr_SetCheckSumType(enum CyBtldr_ChecksumType chksumType)
{
	CyBtldr_Checksum = chksumType;
}

static int ParseGenericCmdResult(u8 *cmdBuf, u32 dataSize, u32 expectedSize,
			u8 *status, u8 expectedStatus)
{
	int err = CYRET_SUCCESS;
	u32 cmdSize = dataSize + BASE_CMD_SIZE;
	*status = cmdBuf[1];
	if (cmdSize != expectedSize)
		err = CYRET_ERR_LENGTH;
	else if (*status != expectedStatus)
		if (*status == CYRET_SUCCESS) {
			err = CYRET_ERR_RESPONSE;
		} else {
			err = CYRET_ERR_BTLDR_MASK | (*status);
		}
	else if (cmdBuf[0] != CMD_START || cmdBuf[2] != ((u8)dataSize) ||
			cmdBuf[3] != ((u8)(dataSize >> 8)) ||
			cmdBuf[cmdSize - 1] != CMD_STOP)
		err = CYRET_ERR_DATA;
	return err;
}

int CyBtldr_ParseDefaultCmdResult(u8 *cmdBuf, u32 cmdSize, u8 *status)
{
	return ParseGenericCmdResult(cmdBuf, 0, cmdSize, status, CYRET_SUCCESS);
}

int CyBtldr_ParseCustomCmdResult(u8 *cmdBuf, u32 dataSize, u32 cmdSize,
			u8 *status, u8 expectedStatus)
{
	return ParseGenericCmdResult(cmdBuf, dataSize, cmdSize, status, expectedStatus);
}

// NOTE: If the cmd contains data bytes, make sure to call this after setting data bytes.
// Otherwise the checksum here will not include the data bytes.
static int CreateCmd(u8 *cmdBuf, u32 cmdSize, u8 cmdCode)
{
	u16 checksum;

	cmdBuf[0] = CMD_START;
	cmdBuf[1] = cmdCode;
	fillData16(cmdBuf + 2, (u16)cmdSize - BASE_CMD_SIZE);
	checksum = CyBtldr_ComputeChecksum16bit(cmdBuf, cmdSize - 3);
	fillData16(cmdBuf + cmdSize - 3, checksum);
	cmdBuf[cmdSize - 1] = CMD_STOP;
	return CYRET_SUCCESS;
}

int CyBtldr_CreateEnterBootLoaderCmd(u8 *cmdBuf, u32 *cmdSize, u32 *resSize, u64 productID)
{
	u16 COMMAND_DATA_SIZE = 4;
	const u16 RESULT_DATA_SIZE = 8;
	*resSize = BASE_CMD_SIZE + RESULT_DATA_SIZE;

	fillData32(cmdBuf + 4, (productID & 0xFFFFFFFF));
	if ((u16)(productID >> 0x20) != 0x0) {
		COMMAND_DATA_SIZE = 6;
		fillData16(cmdBuf + 8, (u16)(productID >> 0x20));
	}
	*cmdSize = BASE_CMD_SIZE + COMMAND_DATA_SIZE;
	return CreateCmd(cmdBuf, *cmdSize, CMD_ENTER_BOOTLOADER);
}

int CyBtldr_ParseEnterBootLoaderCmdResult(u8 *cmdBuf, u32 cmdSize, u32 *siliconId,
			u8 *siliconRev, u32 *blVersion, u8 *status)
{
	const u32 RESULT_DATA_SIZE = 8;
	int err = ParseGenericCmdResult(cmdBuf, RESULT_DATA_SIZE, cmdSize, status, CYRET_SUCCESS);

	if (err == CYRET_SUCCESS) {
		*siliconId = (cmdBuf[7] << 24) | (cmdBuf[6] << 16) | (cmdBuf[5] << 8) | cmdBuf[4];
		*siliconRev = cmdBuf[8];
		*blVersion = (cmdBuf[11] << 16) | (cmdBuf[10] << 8) | cmdBuf[9];
	}
	return err;
}

int CyBtldr_CreateExitBootLoaderCmd(u8 *cmdBuf, u32 *cmdSize, u32 *resSize)
{
	*cmdSize = BASE_CMD_SIZE;
	*resSize = BASE_CMD_SIZE;
	return CreateCmd(cmdBuf, *cmdSize, CMD_EXIT_BOOTLOADER);
}

int CyBtldr_ParseVerifyChecksumCmdResult(u8 *cmdBuf, u32 cmdSize, u8 *checksumValid, u8 *status)
{
	const u16 RESULT_DATA_SIZE = 1;
	int err = ParseGenericCmdResult(cmdBuf, RESULT_DATA_SIZE, cmdSize, status, CYRET_SUCCESS);

	if (err == CYRET_SUCCESS)
		*checksumValid = cmdBuf[4];

	return err;
}

int CyBtldr_CreateSendDataCmd(u8 *buf, u16 size, u8 *cmdBuf, u32 *cmdSize, u32 *resSize)
{
	u16 i;
	*resSize = BASE_CMD_SIZE;
	*cmdSize = size + BASE_CMD_SIZE;

	for (i = 0; i < size; i++)
		cmdBuf[i + 4] = buf[i];

	return CreateCmd(cmdBuf, *cmdSize, CMD_SEND_DATA);
}

int CyBtldr_ParseSendDataCmdResult(u8 *cmdBuf, u32 cmdSize, u8 *status)
{
	return CyBtldr_ParseDefaultCmdResult(cmdBuf, cmdSize, status);
}

int CyBtldr_CreateProgramDataCmd(u32 address, u32 chksum, u8 *buf, u16 size,
			u8 *cmdBuf, u32 *cmdSize, u32 *resSize)
{
	const u16 COMMAND_DATA_SIZE = 8;
	u16 i;
	*resSize = BASE_CMD_SIZE;
	*cmdSize = BASE_CMD_SIZE + COMMAND_DATA_SIZE + size;

	fillData32(cmdBuf + 4, address);
	fillData32(cmdBuf + 8, chksum);
	for (i = 0; i < size; i++)
		cmdBuf[i + 4 + COMMAND_DATA_SIZE] = buf[i];

	return CreateCmd(cmdBuf, *cmdSize, CMD_PROGRAM_DATA);
}

int CyBtldr_CreateVerifyDataCmd(u32 address, u32 chksum, u8 *buf, u16 size,
			u8 *cmdBuf, u32 *cmdSize, u32 *resSize)
{
	const u16 COMMAND_DATA_SIZE = 8;
	u16 i;
	*resSize = BASE_CMD_SIZE;
	*cmdSize = BASE_CMD_SIZE + COMMAND_DATA_SIZE + size;

	fillData32(cmdBuf + 4, address);
	fillData32(cmdBuf + 8, chksum);
	for (i = 0; i < size; i++)
		cmdBuf[i + 4 + COMMAND_DATA_SIZE] = buf[i];

	return CreateCmd(cmdBuf, *cmdSize, CMD_VERIFY_DATA);
}

int CyBtldr_CreateCustomDefaultCmd(u8 *buf, u16 size, u8 *cmdBuf, u32 *cmdSize,
			u32 *resSize, u8 cmdCode)
{
	u16 i;
	*resSize = BASE_CMD_SIZE;  // read full packet since no data expected
	*cmdSize = BASE_CMD_SIZE + size;

	for (i = 0; i < size; i++)
		cmdBuf[i + 4] = buf[i];

	return CreateCmd(cmdBuf, *cmdSize, cmdCode);
}

int CyBtldr_CreateCustomDataCmd(u8 *buf, u16 size, u8 *cmdBuf, u32 *cmdSize,
			u32 *resSize, u8 cmdCode)
{
	u16 i;
	*resSize = 0x04; // read until we get data length
	*cmdSize = BASE_CMD_SIZE + size;

	for (i = 0; i < size; i++)
		cmdBuf[i + 4] = buf[i];

	return CreateCmd(cmdBuf, *cmdSize, cmdCode);
}

int CyBtldr_CreateEraseDataCmd(u32 address, u8 *cmdBuf, u32 *cmdSize, u32 *resSize)
{
	const u16 COMMAND_DATA_SIZE = 4;
	*resSize = BASE_CMD_SIZE;
	*cmdSize = BASE_CMD_SIZE + COMMAND_DATA_SIZE;

	fillData32(cmdBuf + 4, address);
	return CreateCmd(cmdBuf, *cmdSize, CMD_ERASE_DATA);
}

int CyBtldr_CreateVerifyChecksumCmd(u8 appId, u8 *cmdBuf, u32 *cmdSize, u32 *resSize)
{
	const u16 COMMAND_DATA_SIZE = 1;
	*resSize = BASE_CMD_SIZE + 1;
	*cmdSize = BASE_CMD_SIZE + COMMAND_DATA_SIZE;

	cmdBuf[4] = appId;
	return CreateCmd(cmdBuf, *cmdSize, CMD_VERIFY_CHECKSUM);
}

int CyBtldr_CreateSetApplicationMetadataCmd(u8 appID, u8 *buf, u8 *cmdBuf,
			u32 *cmdSize, u32 *resSize)
{
	u32 i;
	const u16 BTDLR_SDK_METADATA_SIZE = 8;
	const u16 COMMAND_DATA_SIZE = BTDLR_SDK_METADATA_SIZE + 1;
	*resSize = BASE_CMD_SIZE;
	*cmdSize = BASE_CMD_SIZE + COMMAND_DATA_SIZE;

	cmdBuf[4] = appID;
	for (i = 0; i < BTDLR_SDK_METADATA_SIZE; i++)
		cmdBuf[5 + i] = buf[i];

	return CreateCmd(cmdBuf, *cmdSize, CMD_SET_METADATA);
}

int CyBtldr_CreateSetEncryptionInitialVectorCmd(u8 *buf, u16 size, u8 *cmdBuf,
			u32 *cmdSize, u32 *resSize)
{
	u32 i;
	*resSize = BASE_CMD_SIZE;
	*cmdSize = BASE_CMD_SIZE + size;
	for (i = 0; i < size; i++)
		cmdBuf[4 + i] = buf[i];

	return CreateCmd(cmdBuf, *cmdSize, CMD_SET_EIV);
}

/*
 * Try to parse a packet to determine its validity, if valid then return set the status
 * param to the packet's status.
 * Used to generate useful error messages. return 1 on success 0 otherwise.
 */
int CyBtldr_TryParsePacketStatus(u8 *packet, int packetSize, u8 *status)
{
	u16 dataSize;
	u16 readChecksum;
	u16 computedChecksum;

	if (packet == NULL || packetSize < BASE_CMD_SIZE || packet[0] != CMD_START)
		return CYBTLDR_STAT_ERR_UNK;

	*status = packet[1];
	dataSize = packet[2] | (packet[3] << 8);

	readChecksum = packet[dataSize + 4] | (packet[dataSize + 5] << 8);
	computedChecksum = CyBtldr_ComputeChecksum16bit(packet, BASE_CMD_SIZE + dataSize - 3);

	if (packet[dataSize + BASE_CMD_SIZE - 1] != CMD_STOP || readChecksum != computedChecksum)
		return CYBTLDR_STAT_ERR_UNK;

	return CYRET_SUCCESS;
}
