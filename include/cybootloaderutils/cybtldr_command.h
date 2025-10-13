/* SPDX-License-Identifier: Apache-2.0 OR MIT */
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

#ifndef __CYBTLDR_COMMAND_H__
#define __CYBTLDR_COMMAND_H__

#include <linux/types.h>

#include "cybtldr_utils.h"

/* Maximum number of bytes to allocate for a single command.  */
#define MAX_COMMAND_SIZE 4103  // 4096 + 7  for packet metadata

// STANDARD PACKET FORMAT:
// Multi byte entries are encoded in LittleEndian.
/*******************************************************************************
 * [1-byte] [1-byte ] [2-byte] [n-byte] [ 2-byte ] [1-byte]
 * [ SOP  ] [Command] [ Size ] [ Data ] [Checksum] [ EOP  ]
 *******************************************************************************/

/* The first byte of any boot loader command. */
#define CMD_START 0x01
/* The last byte of any boot loader command. */
#define CMD_STOP 0x17
/* The minimum number of bytes in a bootloader command. */
#define BASE_CMD_SIZE 0x07

/* Command identifier for verifying the checksum value of the bootloadable project. */
#define CMD_VERIFY_CHECKSUM 0x31
/* Command identifier for erasing a row of flash data from the target device. */
#define CMD_ERASE_ROW 0x34
/* Command identifier for making sure the bootloader host and bootloader are in sync. */
#define CMD_SYNC 0x35
/* Command identifier for sending a block of data to the bootloader
 * without doing anything with it yet.
 */
#define CMD_SEND_DATA 0x37
/* Command identifier for sending a block of data to the bootloader
 * without sending the response back.
 */
#define CMD_SEND_DATA_NO_RSP 0x47
/* Command identifier for starting the boot loader.
 * All other commands ignored until this is sent.
 */
#define CMD_ENTER_BOOTLOADER 0x38
/* Command identifier for exiting the bootloader and restarting the target program. */
#define CMD_EXIT_BOOTLOADER 0x3B
/* Command to erase data */
#define CMD_ERASE_DATA 0x44
/* Command to program data. */
#define CMD_PROGRAM_DATA 0x49
/* Command to verify data */
#define CMD_VERIFY_DATA 0x4A
/* Command to set application metadata in bootloader SDK */
#define CMD_SET_METADATA 0x4C
/* Command to set encryption initial vector */
#define CMD_SET_EIV 0x4D

/*
 * This enum defines the different types of checksums that can be
 * used by the bootloader for ensuring data integrity.
 */
enum CyBtldr_ChecksumType {
	/* Checksum type is a basic inverted summation of all bytes */
	SUM_CHECKSUM = 0x00,
	/* 16-bit CRC checksum using the CCITT implementation */
	CRC_CHECKSUM = 0x01,
};

EXTERN void fillData32(u8 *buf, u32 data);

/*******************************************************************************
 * Function Name: CyBtldr_ComputeChecksum16bit
 ********************************************************************************
 * Summary:
 *   Computes the 2 byte checksum for the provided command data.  The checksum is
 *   either 2's complement or CRC16.
 *
 * Parameters:
 *   buf  - The data to compute the checksum on
 *   size - The number of bytes contained in buf.
 *
 * Returns:
 *   The checksum for the provided data.
 *
 *******************************************************************************/
u16 CyBtldr_ComputeChecksum16bit(u8 *buf, u32 size);

/*******************************************************************************
 * Function Name: CyBtldr_ComputeChecksum32bit
 ********************************************************************************
 * Summary:
 *   Computes the 4 byte checksum for the provided command data.  The checksum is
 *   computed using CRC32-C
 *
 * Parameters:
 *   buf  - The data to compute the checksum on
 *   size - The number of bytes contained in buf.
 *
 * Returns:
 *   The checksum for the provided data.
 *
 *******************************************************************************/
u32 CyBtldr_ComputeChecksum32bit(u8 *buf, u32 size);

/*******************************************************************************
 * Function Name: CyBtldr_SetCheckSumType
 ********************************************************************************
 * Summary:
 *   Updates what checksum algorithm is used when generating packets
 *
 * Parameters:
 *   chksumType - The type of checksum to use when creating packets
 *
 * Returns:
 *   NA
 *
 *******************************************************************************/
void CyBtldr_SetCheckSumType(enum CyBtldr_ChecksumType chksumType);

/*******************************************************************************
 * Function Name: CyBtldr_ParseDefaultCmdResult
 ********************************************************************************
 * Summary:
 *   Parses the output from any command that returns the default result packet
 *   data.  The default result is just a status byte
 *
 * Response Size: 7
 *
 * Parameters:
 *   cmdBuf  - The preallocated buffer to store command data in.
 *   cmdSize - The number of bytes in the command.
 *   status  - The status code returned by the bootloader.
 *
 * Returns:
 *   CYRET_SUCCESS    - The command was constructed successfully
 *   CYRET_ERR_LENGTH - The packet does not contain enough data
 *   CYRET_ERR_DATA   - The packet's contents are not correct
 *
 *******************************************************************************/
EXTERN int CyBtldr_ParseDefaultCmdResult(u8 *cmdBuf, u32 cmdSize, u8 *status);

/*******************************************************************************
 * Function Name: CyBtldr_ParseDefaultCmdResult
 ********************************************************************************
 * Summary:
 *   Parses the output from custom command that returns the response packet
 *   of any length.
 *
 * Parameters:
 *   cmdBuf         - The preallocated buffer to store command data in.
 *   dataSize       - The number of bytes which represent real data.
 *   cmdSize        - The number of all bytes in the command.
 *   status         - The status code returned by the bootloader.
 *   expectedStatus - The status code which is expected by the host.
 *
 * Returns:
 *   CYRET_SUCCESS    - The command was constructed successfully
 *   CYRET_ERR_LENGTH - The packet does not contain enough data
 *   CYRET_ERR_DATA   - The packet's contents are not correct
 *
 *******************************************************************************/
EXTERN int CyBtldr_ParseCustomCmdResult(u8 *cmdBuf, u32 dataSize, u32 cmdSize, u8 *status,
										u8 expectedStatus);

/*******************************************************************************
 * Function Name: CyBtldr_CreateEnterBootLoaderCmd
 ********************************************************************************
 * Summary:
 *   Creates the command used to startup the bootloader. This function is only
 *   used for applications using the .cyacd2 format.
 *   NB: This command must be sent before the bootloader will respond to any
 *       other command.
 *
 * Command Size: 13
 *
 * Parameters:
 *   protect         - The flash protection settings.
 *   cmdBuf          - The preallocated buffer to store command data in.
 *   cmdSize         - The number of bytes in the command.
 *   resSize         - The number of bytes expected in the bootloader's response packet.
 *   productID       - The product ID of the device
 *
 * Returns:
 *   CYRET_SUCCESS  - The command was constructed successfully
 *
 *******************************************************************************/
EXTERN int CyBtldr_CreateEnterBootLoaderCmd(u8 *cmdBuf, u32 *cmdSize, u32 *resSize, u64 productID);

/*******************************************************************************
 * Function Name: CyBtldr_ParseEnterBootLoaderCmdResult
 ********************************************************************************
 * Summary:
 *   Parses the output from the EnterBootLoader command to get the resultant
 *   data.
 *
 * Response Size: 15
 *
 * Parameters:
 *   cmdBuf     - The buffer containing the output from the bootloader.
 *   cmdSize    - The number of bytes in cmdBuf.
 *   siliconId  - The silicon ID of the device being communicated with.
 *   siliconRev - The silicon Revision of the device being communicated with.
 *   blVersion  - The bootloader version being communicated with.
 *   status     - The status code returned by the bootloader.
 *
 * Returns:
 *   CYRET_SUCCESS    - The command was constructed successfully
 *   CYRET_ERR_LENGTH - The packet does not contain enough data
 *   CYRET_ERR_DATA   - The packet's contents are not correct
 *
 *******************************************************************************/
EXTERN int CyBtldr_ParseEnterBootLoaderCmdResult(u8 *cmdBuf, u32 cmdSize, u32 *siliconId,
							u8 *siliconRev, u32 *blVersion, u8 *status);

/*******************************************************************************
 * Function Name: CyBtldr_CreateExitBootLoaderCmd
 ********************************************************************************
 * Summary:
 *   Creates the command used to stop communicating with the boot loader and to
 *   trigger the target device to restart, running the new bootloadable
 *   application.
 *
 * Command Size: 7
 *
 * Parameters:
 *   cmdBuf    - The preallocated buffer to store command data in.
 *   cmdSize   - The number of bytes in the command.
 *   resSize   - The number of bytes expected in the bootloader's response packet.
 *
 * Returns:
 *   CYRET_SUCCESS  - The command was constructed successfully
 *
 *******************************************************************************/
EXTERN int CyBtldr_CreateExitBootLoaderCmd(u8 *cmdBuf, u32 *cmdSize, u32 *resSize);

/*******************************************************************************
 * Function Name: CyBtldr_ParseVerifyChecksumCmdResult
 ********************************************************************************
 * Summary:
 *   Parses the output from the VerifyChecksum command to get the resultant
 *   data.
 *
 * Response Size: 8
 *
 * Parameters:
 *   cmdBuf           - The preallocated buffer to store command data in.
 *   cmdSize          - The number of bytes in the command.
 *   checksumValid    - Whether or not the full checksums match (1 = valid, 0 = invalid)
 *   status           - The status code returned by the bootloader.
 *
 * Returns:
 *   CYRET_SUCCESS    - The command was constructed successfully
 *   CYRET_ERR_LENGTH - The packet does not contain enough data
 *   CYRET_ERR_DATA   - The packet's contents are not correct
 *
 *******************************************************************************/
EXTERN int CyBtldr_ParseVerifyChecksumCmdResult(u8 *cmdBuf, u32 cmdSize, u8 *checksumValid,
											u8 *status);

/*******************************************************************************
 * Function Name: CyBtldr_CreateSendDataCmd
 ********************************************************************************
 * Summary:
 *   Creates the command used to send a block of data to the target.
 *
 * Command Size: greater than 7
 *
 * Parameters:
 *   buf     - The buffer of data to program into the flash row.
 *   size    - The number of bytes in data for the row.
 *   cmdBuf  - The preallocated buffer to store command data in.
 *   cmdSize - The number of bytes in the command.
 *   resSize - The number of bytes expected in the bootloader's response packet.
 *
 * Returns:
 *   CYRET_SUCCESS  - The command was constructed successfully
 *
 *******************************************************************************/
EXTERN int CyBtldr_CreateSendDataCmd(u8 *buf, u16 size, u8 *cmdBuf, u32 *cmdSize, u32 *resSize);

/*******************************************************************************
 * Function Name: CyBtldr_ParseSendDataCmdResult
 ********************************************************************************
 * Summary:
 *   Parses the output from the SendData command to get the resultant
 *   data.
 *
 * Response Size: 7
 *
 * Parameters:
 *   cmdBuf  - The preallocated buffer to store command data in.
 *   cmdSize - The number of bytes in the command.
 *   status  - The status code returned by the bootloader.
 *
 * Returns:
 *   CYRET_SUCCESS    - The command was constructed successfully
 *   CYRET_ERR_LENGTH - The packet does not contain enough data
 *   CYRET_ERR_DATA   - The packet's contents are not correct
 *
 *******************************************************************************/
EXTERN int CyBtldr_ParseSendDataCmdResult(u8 *cmdBuf, u32 cmdSize, u8 *status);

/*******************************************************************************
 * Function Name: CyBtldr_CreateProgramDataCmd
 ********************************************************************************
 * Summary:
 *   Creates the command used to program data.
 *
 * Command Size: At least 15
 *
 * Parameters:
 *   address - The address to program.
 *   chksum  - The checksum all the data being programmed by this command
 *             the preceding send data commands.
 *   buf     - The buffer of data to program into the flash row.
 *   size    - The number of bytes in data for the row.
 *   cmdBuf  - The preallocated buffer to store command data in.
 *   cmdSize - The number of bytes in the command.
 *   resSize - The number of bytes expected in the bootloader's response packet.
 *
 * Returns:
 *   CYRET_SUCCESS  - The command was constructed successfully
 *
 *******************************************************************************/
EXTERN int CyBtldr_CreateProgramDataCmd(u32 address, u32 chksum, u8 *buf, u16 size, u8 *cmdBuf,
									u32 *cmdSize, u32 *resSize);

/*******************************************************************************
 * Function Name: CyBtldr_CreateVerifyDataCmd
 ********************************************************************************
 * Summary:
 *   Creates the command used to verify data.
 *
 * Command Size: At least 15
 *
 * Parameters:
 *   address - The address to verify.
 *   chksum  - The checksum all the data being verified by this command
 *             the preceding send data commands.
 *   buf     - The buffer of data to verify against.
 *   size    - The number of bytes in data for the row.
 *   cmdBuf  - The preallocated buffer to store command data in.
 *   cmdSize - The number of bytes in the command.
 *   resSize - The number of bytes expected in the bootloader's response packet.
 *
 * Returns:
 *   CYRET_SUCCESS  - The command was constructed successfully
 *
 *******************************************************************************/
EXTERN int CyBtldr_CreateVerifyDataCmd(u32 address, u32 chksum, u8 *buf, u16 size, u8 *cmdBuf,
									u32 *cmdSize, u32 *resSize);

/*******************************************************************************
 * Function Name: CyBtldr_CreateEraseDataCmd
 ********************************************************************************
 * Summary:
 *   Creates the command used to erase data.
 *
 * Command Size: 11
 *
 * Parameters:
 *   address     - The address to erase.
 *   cmdBuf      - The preallocated buffer to store command data in.
 *   cmdSize     - The number of bytes in the command.
 *   resSize     - The number of bytes expected in the bootloader's response packet.
 *
 * Returns:
 *   CYRET_SUCCESS  - The command was constructed successfully
 *
 *******************************************************************************/
EXTERN int CyBtldr_CreateEraseDataCmd(u32 address, u8 *cmdBuf, u32 *cmdSize, u32 *resSize);

/*******************************************************************************
 * Function Name: CyBtldr_CreateCustomDefaultCmd
 ********************************************************************************
 * Summary:
 *   Creates the command used to send custom command which doesn't expect
 *   any data payload in response packet.
 *
 * Command Size: At least 15
 *
 * Parameters:
 *   buf     - The buffer of data to send into the flash row.
 *   size    - The number of bytes in data for the row.
 *   cmdBuf  - The preallocated buffer to store command data in.
 *   cmdSize - The number of bytes in the command.
 *   resSize - The number of bytes expected in the bootloader's response packet.
 *   cmdCode - The ID of the command to be sent.
 *
 * Returns:
 *   CYRET_SUCCESS  - The command was constructed successfully
 *
 *******************************************************************************/
EXTERN int CyBtldr_CreateCustomDefaultCmd(u8 *buf, u16 size, u8 *cmdBuf, u32 *cmdSize,
								u32 *resSize, u8 cmdCode);

/*******************************************************************************
 * Function Name: CyBtldr_CreateCustomDataCmd
 ********************************************************************************
 * Summary:
 *   Creates the command used to send custom command.
 *
 * Command Size: At least 15
 *
 * Parameters:
 *   buf     - The buffer of data to send into the flash row.
 *   size    - The number of bytes in data for the row.
 *   cmdBuf  - The preallocated buffer to store command data in.
 *   cmdSize - The number of bytes in the command.
 *   resSize - The number of bytes expected in the bootloader's response packet.
 *   cmdCode - The ID of the command to be sent.
 *
 * Returns:
 *   CYRET_SUCCESS  - The command was constructed successfully
 *
 *******************************************************************************/
EXTERN int CyBtldr_CreateCustomDataCmd(u8 *buf, u16 size, u8 *cmdBuf, u32 *cmdSize, u32 *resSize,
										u8 cmdCode);

/*******************************************************************************
 * Function Name: CyBtldr_CreateVerifyChecksumCmd
 ********************************************************************************
 * Summary:
 *   Creates the command used to verify application. This function is only used
 *   for applications using the .cyacd2 format.
 *
 * Command Size: 8
 *
 * Parameters:
 *   appId   - The application number.
 *   cmdBuf  - The preallocated buffer to store command data in.
 *   cmdSize - The number of bytes in the command.
 *   resSize - The number of bytes expected in the bootloader's response packet.
 *
 * Returns:
 *   CYRET_SUCCESS  - The command was constructed successfully
 *
 *******************************************************************************/
EXTERN int CyBtldr_CreateVerifyChecksumCmd(u8 appId, u8 *cmdBuf, u32 *cmdSize, u32 *resSize);

/*******************************************************************************
 * Function Name: CyBtldr_CreateSetApplicationMetadataCmd
 ********************************************************************************
 * Summary:
 *   Set the bootloader SDK's metadata field for a specific application ID. This
 *   function is only used for applications using the .cyacd2 format.
 *
 * Command Size: 16
 *
 * Parameters:
 *   appID       - The ID number of the application.
 *   buf         - The buffer containing the application metadata (8 bytes).
 *   cmdBuf      - The preallocated buffer to store the command data in.
 *   cmdSize     - The number of bytes in the command.
 *   resSize     - The number of bytes expected in the bootloader's response packet.
 *
 * Returns:
 *   CYRET_SUCCESS -The command was constructed successfully
 *******************************************************************************/
EXTERN int CyBtldr_CreateSetApplicationMetadataCmd(u8 appID, u8 *buf, u8 *cmdBuf, u32 *cmdSize,
									u32 *resSize);

/*******************************************************************************
 * Function Name: CyBtldr_CreateSetEncryptionInitialVectorCmd
 ********************************************************************************
 * Summary:
 *   Set the bootloader SDK's encryption initial vector (EIV). This function is
 *   only used for applications using the .cyacd2 format.
 *
 * Command Size: 7 or 15 or 23
 *
 * Parameters:
 *   buf         - The buffer containing the EIV.
 *   size        - The number bytes of the EIV. (Should be 0 or 8 or 16)
 *   cmdBuf      - The preallocated buffer to store the command data in.
 *   cmdSize     - The number of bytes in the command.
 *   resSize     - The number of bytes expected in the bootloader's response packet.
 *
 * Returns:
 *   CYRET_SUCCESS -The command was constructed successfully
 *******************************************************************************/
EXTERN int CyBtldr_CreateSetEncryptionInitialVectorCmd(u8 *buf, u16 size, u8 *cmdBuf, u32 *cmdSize,
									u32 *resSize);

/*******************************************************************************
 * Function Name: CyBtldr_TryParsePacketStatus
 ********************************************************************************
 * Summary:
 *   Parses the output packet data
 *
 * Parameters:
 *   packet      - The preallocated buffer to store command data in.
 *   packetSize  - The number of bytes in the command.
 *   status      - The status code returned by the bootloader.
 *
 * Returns:
 *   CYRET_SUCCESS           - The packet is a valid packet
 *   CYBTLDR_STAT_ERR_UNK    - The packet is not a valid packet
 *
 *******************************************************************************/
EXTERN int CyBtldr_TryParsePacketStatus(u8 *packet, int packetSize, u8 *status);

#endif
