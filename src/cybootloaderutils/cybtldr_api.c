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

#include "cybootloaderutils/cybtldr_api.h"
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>

#define DFU_MAX_RETRY 10

static struct CyBtldr_CommunicationsData *g_comm;

static u16 min_uint16(u16 a, u16 b) { return (a < b) ? a : b; }

int CyBtldr_TransferData(u8 *inBuf, int inSize, u8 *outBuf, int outSize)
{
	int err = g_comm->WriteData(inBuf, inSize);

	if (err == CYRET_SUCCESS && outSize != 0)
		err = g_comm->ReadData(outBuf, outSize);

	if (err != CYRET_SUCCESS)
		err |= CYRET_ERR_COMM_MASK;

	return err;
}

int CyBtldr_ReadData(u8 *outBuf, int outSize)
{
	int err = g_comm->ReadData(outBuf, outSize);

	if (err != CYRET_SUCCESS)
		err |= CYRET_ERR_COMM_MASK;

	return err;
}

int CyBtldr_StartBootloadOperation(struct CyBtldr_CommunicationsData *comm, u32 expSiId,
			u8 expSiRev, u32 *blVer, u64 productID)
{
	u32 inSize = 0;
	u32 outSize = 0;
	u32 siliconId = 0;
	u8 *inBuf;
	u8 *outBuf;
	u8 siliconRev = 0;
	u8 status = CYRET_SUCCESS;
	int err;

	inBuf = kmalloc(MAX_COMMAND_SIZE, GFP_KERNEL);
	if (!inBuf)
		return -ENOMEM;

	outBuf = kmalloc(MAX_COMMAND_SIZE, GFP_KERNEL);
	if (!outBuf) {
		kfree(inBuf);
		return -ENOMEM;
	}

	g_comm = comm;

	err = g_comm->OpenConnection();
	if (err != CYRET_SUCCESS)
		err |= CYRET_ERR_COMM_MASK;

	if (err == CYRET_SUCCESS) {
		err = CyBtldr_CreateEnterBootLoaderCmd(inBuf, &inSize, &outSize, productID);
		if (err == CYRET_SUCCESS) {
			err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
			if (err == CYRET_SUCCESS) {
				err = CyBtldr_ParseEnterBootLoaderCmdResult(outBuf, outSize,
					&siliconId, &siliconRev, blVer, &status);
			} else if (CyBtldr_TryParsePacketStatus(outBuf, outSize, &status)
					== CYRET_SUCCESS) {
				/* if the response we get back is a valid packet
				 * override the err with the response's status
				 */
				err = status | CYRET_ERR_BTLDR_MASK;
			}
		}
	}
	if (err == CYRET_SUCCESS) {
		if (status != CYRET_SUCCESS)
			err = status | CYRET_ERR_BTLDR_MASK;

		if (expSiId != siliconId || expSiRev != siliconRev)
			err = CYRET_ERR_DEVICE;
	}

	kfree(inBuf);
	kfree(outBuf);

	return err;
}

int CyBtldr_EndBootloadOperation(void)
{
	u32 inSize;
	u32 outSize;
	u8 *inBuf;

	inBuf = kmalloc(MAX_COMMAND_SIZE, GFP_KERNEL);
	if (!inBuf)
		return -ENOMEM;

	int err = CyBtldr_CreateExitBootLoaderCmd(inBuf, &inSize, &outSize);

	if (err == CYRET_SUCCESS) {
		err = g_comm->WriteData(inBuf, inSize);

		if (err == CYRET_SUCCESS)
			err = g_comm->CloseConnection();

		if (err != CYRET_SUCCESS)
			err |= CYRET_ERR_COMM_MASK;
	}
	g_comm = NULL;

	kfree(inBuf);

	return err;
}

int CyBtldr_isBootloaderAppActive(struct CyBtldr_CommunicationsData *comm)
{
	u32 inSize = 5;
	u32 outSize = 7;
	u8 inBuf[5] = {0x00, 0x00, CMD_START, CMD_BOOTLOADER_APP_ACTIVE, CMD_STOP};
	u8 outBuf[7] = {0x00};
	int ret = CYRET_SUCCESS;
	int err_cnt = 0;
	int is_bootloader_app_active = -1;

	g_comm = comm;

	ret = g_comm->OpenConnection();

	if (ret != CYRET_SUCCESS)
		ret |= CYRET_ERR_COMM_MASK;

	if (ret != CYRET_SUCCESS)
		return ret;

	while (err_cnt < DFU_MAX_RETRY) {
		ret = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);

		if (ret == CYRET_SUCCESS && outBuf[0] == CMD_START &&
			outBuf[1] == CMD_BOOTLOADER_APP_ACTIVE_ACK &&
			outBuf[outSize - 1] == CMD_STOP) {
			is_bootloader_app_active = CYRET_SUCCESS;
			break;
		}

		++err_cnt;
	}

	ret = g_comm->CloseConnection();

	if (ret != CYRET_SUCCESS)
		ret |= CYRET_ERR_COMM_MASK;

	if (is_bootloader_app_active != CYRET_SUCCESS)
		ret |= CYRET_ERR_COMM_MASK;

	g_comm = NULL;

	return ret;
}

static int SendData(u8 *buf, u16 size, u16 *offset, u16 maxRemainingDataSize, u8 *inBuf, u8 *outBuf)
{
	u8 status = CYRET_SUCCESS;
	u32 inSize = 0, outSize = 0;
	// size is the total bytes of data to transfer.
	// offset is the amount of data already transferred.
	// a is maximum amount of data allowed to be left over when this function ends.
	// (we can leave some data for caller (programRow, VerifyRow,...) to send.
	// TRANSFER_HEADER_SIZE is the amount of bytes this command header takes up.
	const u16 TRANSFER_HEADER_SIZE = 7;
	u16 subBufSize = min_uint16((u16)(g_comm->DataPacketSize - TRANSFER_HEADER_SIZE), size);
	int err = CYRET_SUCCESS;
	u16 cmdLen = 0;
	// Break row into pieces to ensure we don't send too much for the transfer protocol
	while ((err == CYRET_SUCCESS) && ((size - (*offset)) > maxRemainingDataSize)) {
		if ((size - (*offset)) > subBufSize)
			cmdLen = subBufSize;
		else
			cmdLen = size - (*offset);

		err = CyBtldr_CreateSendDataCmd(&buf[*offset], cmdLen, inBuf, &inSize, &outSize);
		if (err == CYRET_SUCCESS) {
			err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
			if (err == CYRET_SUCCESS)
				err = CyBtldr_ParseSendDataCmdResult(outBuf, outSize, &status);
		}
		if (status != CYRET_SUCCESS)
			err = status | CYRET_ERR_BTLDR_MASK;

		(*offset) += cmdLen;
	}
	return err;
}

int CyBtldr_ProgramRow(u32 address, u8 *buf, u16 size)
{
	const size_t TRANSFER_HEADER_SIZE = 15;

	u8 *inBuf;
	u8 *outBuf;
	u32 inSize;
	u32 outSize;
	u16 offset = 0;
	u16 subBufSize;
	u8 status = CYRET_SUCCESS;
	int err = CYRET_SUCCESS;

	inBuf = kmalloc(MAX_COMMAND_SIZE, GFP_KERNEL);
	if (!inBuf)
		return -ENOMEM;

	outBuf = kmalloc(MAX_COMMAND_SIZE, GFP_KERNEL);
	if (!outBuf) {
		kfree(inBuf);
		return -ENOMEM;
	}

	u32 chksum = CyBtldr_ComputeChecksum32bit(buf, size);

	u16 maxDataTransferSize = (g_comm->DataPacketSize >= TRANSFER_HEADER_SIZE)
					? (u16)(g_comm->DataPacketSize - TRANSFER_HEADER_SIZE)
					: 0;
	if (err == CYRET_SUCCESS)
		err = SendData(buf, size, &offset, maxDataTransferSize, inBuf, outBuf);

	if (err == CYRET_SUCCESS) {
		subBufSize = (u16)(size - offset);
		err = CyBtldr_CreateProgramDataCmd(address, chksum, &buf[offset], subBufSize,
					inBuf, &inSize, &outSize);
		if (err == CYRET_SUCCESS) {
			err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
			if (err == CYRET_SUCCESS)
				err = CyBtldr_ParseDefaultCmdResult(outBuf, outSize, &status);
		}
		if (status != CYRET_SUCCESS)
			err = status | CYRET_ERR_BTLDR_MASK;
	}

	kfree(inBuf);
	kfree(outBuf);

	return err;
}

int CyBtldr_EraseRow(u32 address)
{
	u8 *inBuf;
	u8 *outBuf;
	u32 inSize = 0;
	u32 outSize = 0;
	u8 status = CYRET_SUCCESS;
	int err = CYRET_SUCCESS;

	inBuf = kmalloc(MAX_COMMAND_SIZE, GFP_KERNEL);
	if (!inBuf)
		return -ENOMEM;

	outBuf = kmalloc(MAX_COMMAND_SIZE, GFP_KERNEL);
	if (!outBuf) {
		kfree(inBuf);
		return -ENOMEM;
	}


	if (err == CYRET_SUCCESS) {
		err = CyBtldr_CreateEraseDataCmd(address, inBuf, &inSize, &outSize);
		if (err == CYRET_SUCCESS) {
			err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
			if (err == CYRET_SUCCESS)
				err = CyBtldr_ParseDefaultCmdResult(outBuf, outSize, &status);
		}
	}
	if (status != CYRET_SUCCESS)
		err = status | CYRET_ERR_BTLDR_MASK;

	kfree(inBuf);
	kfree(outBuf);

	return err;
}

int CyBtldr_VerifyRow(u32 address, u8 *buf, u16 size)
{
	const size_t TRANSFER_HEADER_SIZE = 15;

	u8 *inBuf;
	u8 *outBuf;
	u32 inSize;
	u32 outSize;
	u16 offset = 0;
	u16 subBufSize;
	u8 status = CYRET_SUCCESS;
	int err = CYRET_SUCCESS;

	inBuf = kmalloc(MAX_COMMAND_SIZE, GFP_KERNEL);
	if (!inBuf)
		return -ENOMEM;

	outBuf = kmalloc(MAX_COMMAND_SIZE, GFP_KERNEL);
	if (!outBuf) {
		kfree(inBuf);
		return -ENOMEM;
	}

	u32 chksum = CyBtldr_ComputeChecksum32bit(buf, size);

	u16 maxDataTransferSize = (g_comm->DataPacketSize >= TRANSFER_HEADER_SIZE)
					? (u16)(g_comm->DataPacketSize - TRANSFER_HEADER_SIZE)
					: 0;
	if (err == CYRET_SUCCESS)
		err = SendData(buf, size, &offset, maxDataTransferSize, inBuf, outBuf);

	if (err == CYRET_SUCCESS) {
		subBufSize = (u16)(size - offset);

		err = CyBtldr_CreateVerifyDataCmd(address, chksum, &buf[offset], subBufSize,
					inBuf, &inSize, &outSize);
		if (err == CYRET_SUCCESS) {
			err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
			if (err == CYRET_SUCCESS)
				err = CyBtldr_ParseDefaultCmdResult(outBuf, outSize, &status);
		}
		if (status != CYRET_SUCCESS)
			err = status | CYRET_ERR_BTLDR_MASK;
	}

	kfree(inBuf);
	kfree(outBuf);

	return err;
}

int CyBtldr_VerifyApplication(u8 appId)
{
	u8 *inBuf;
	u8 *outBuf;
	u32 inSize = 0;
	u32 outSize = 0;
	u8 checksumValid = 0;
	u8 status = CYRET_SUCCESS;

	inBuf = kmalloc(MAX_COMMAND_SIZE, GFP_KERNEL);
	if (!inBuf)
		return -ENOMEM;

	outBuf = kmalloc(MAX_COMMAND_SIZE, GFP_KERNEL);
	if (!outBuf) {
		kfree(inBuf);
		return -ENOMEM;
	}

	int err = CyBtldr_CreateVerifyChecksumCmd(appId, inBuf, &inSize, &outSize);

	if (err == CYRET_SUCCESS) {
		err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
		if (err == CYRET_SUCCESS)
			err = CyBtldr_ParseVerifyChecksumCmdResult(outBuf, outSize,
					&checksumValid, &status);
	}
	if (status != CYRET_SUCCESS)
		err = status | CYRET_ERR_BTLDR_MASK;

	if ((err == CYRET_SUCCESS) && (!checksumValid))
		err = CYRET_ERR_CHECKSUM;

	kfree(inBuf);
	kfree(outBuf);

	return err;
}

int CyBtldr_SetApplicationMetaData(u8 appId, u32 appStartAddr, u32 appSize)
{
	u8 *inBuf;
	u8 *outBuf;
	u32 inSize = 0;
	u32 outSize = 0;
	u8 status = CYRET_SUCCESS;

	inBuf = kmalloc(MAX_COMMAND_SIZE, GFP_KERNEL);
	if (!inBuf)
		return -ENOMEM;

	outBuf = kmalloc(MAX_COMMAND_SIZE, GFP_KERNEL);
	if (!outBuf) {
		kfree(inBuf);
		return -ENOMEM;
	}

	u8 metadata[8];

	metadata[0] = (u8)appStartAddr;
	metadata[1] = (u8)(appStartAddr >> 8);
	metadata[2] = (u8)(appStartAddr >> 16);
	metadata[3] = (u8)(appStartAddr >> 24);
	metadata[4] = (u8)appSize;
	metadata[5] = (u8)(appSize >> 8);
	metadata[6] = (u8)(appSize >> 16);
	metadata[7] = (u8)(appSize >> 24);

	int err = CyBtldr_CreateSetApplicationMetadataCmd(appId, metadata, inBuf,
					&inSize, &outSize);

	if (err == CYRET_SUCCESS) {
		err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
		if (err == CYRET_SUCCESS)
			err = CyBtldr_ParseDefaultCmdResult(outBuf, outSize, &status);
	}

	if (status != CYRET_SUCCESS)
		err = status | CYRET_ERR_BTLDR_MASK;

	kfree(inBuf);
	kfree(outBuf);

	return err;
}

int CyBtldr_SetEncryptionInitialVector(u16 size, u8 *buf)
{
	u8 *inBuf;
	u8 *outBuf;

	u32 inSize = 0;
	u32 outSize = 0;
	u8 status = CYRET_SUCCESS;

	inBuf = kmalloc(MAX_COMMAND_SIZE, GFP_KERNEL);
	if (!inBuf)
		return -ENOMEM;

	outBuf = kmalloc(MAX_COMMAND_SIZE, GFP_KERNEL);
	if (!outBuf) {
		kfree(inBuf);
		return -ENOMEM;
	}

	int err = CyBtldr_CreateSetEncryptionInitialVectorCmd(buf, size, inBuf, &inSize, &outSize);

	if (err == CYRET_SUCCESS) {
		err = CyBtldr_TransferData(inBuf, inSize, outBuf, outSize);
		if (err == CYRET_SUCCESS)
			err = CyBtldr_ParseDefaultCmdResult(outBuf, outSize, &status);
	}
	if (status != CYRET_SUCCESS)
		err = status | CYRET_ERR_BTLDR_MASK;

	kfree(inBuf);
	kfree(outBuf);

	return err;
}
