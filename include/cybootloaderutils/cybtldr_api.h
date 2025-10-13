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

#ifndef __CYBTLDR_API_H__
#define __CYBTLDR_API_H__

#include <linux/types.h>

#include "cybtldr_command.h"
#include "cybtldr_utils.h"

/**
 * This struct defines all of the items necessary for the bootloader
 * host to communicate over an arbitrary communication protocol. The
 * caller must provide implementations of these items to use their
 * desired communication protocol.
 */
struct CyBtldr_CommunicationsData {
	/** Function used to open the communications connection */
	int (*OpenConnection)(void);
	/** Function used to close the communications connection */
	int (*CloseConnection)(void);
	/** Function used to read data over the communications connection */
	int (*ReadData)(u8 *buffer, int size);
	/** Function used to write data over the communications connection */
	int (*WriteData)(u8 *buffer, int size);
	/** Value used to specify the number of bytes that should be transferred at a time */
	unsigned int DataPacketSize;
	/** Value used to specify the maximum number of bytes that can be transferred at a time */
	unsigned int MaxTransferSize;
};

/**
 * This struct defines response structure for custom command
 * defined by user.
 */
struct CyBtldr_CustomCommandResponseData {
	/** Status Code for command transmission */
	u8 statusCode;
	/** Length of response data */
	u16 responseLength;
	/** Data payload */
	u8 *response;
	/** Saves fields that were defined inside JSON file by setting appropriate bit:
	 *  | statusCode | responseLength | response |
	 */
	char fieldBits;
};

/** Mask for response bit of fieldBits */
#define responseMask 4
/** Mask for responseLength bit of fieldBits */
#define responseLengthMask 2
/** Mask for statusCode bit of fieldBits */
#define statusCodeMask 1

/** Command IDs */
#define CMD_BOOTLOADER_APP_ACTIVE 0xEE
#define CMD_BOOTLOADER_APP_ACTIVE_ACK 0x04

/**
 * This struct defines all metadata from json header
 * defined by user.
 */
struct CyBtldr_CustomCommandHeaderData {
	/** File Version */
	unsigned int fileVersion;
	/** Product Id */
	unsigned long long productId;
	/** Application Id */
	unsigned int applicationId;
	/** Application Start */
	unsigned int applicationStart;
	/** Application Length */
	unsigned int applicationLength;
	/** Packet checksum type */
	unsigned char checksumType;
	/** Flash Row Length */
	unsigned int flashRowLength;
	/** Timeout (in milliseconds) */
	unsigned int timeoutMS;
};

int CyBtldr_isBootloaderAppActive(struct CyBtldr_CommunicationsData *comm);

/*******************************************************************************
 * Function Name: CyBtldr_TransferData
 ********************************************************************************
 * Summary:
 *   This function is responsible for transferring a buffer of data to the target
 *   device and then reading a response packet back from the device.
 *
 * Parameters:
 *   inBuf   - The buffer containing data to send to the target device
 *   inSize  - The number of bytes to send to the target device
 *   outBuf  - The buffer to store the data read from the device
 *   outSize - The number of bytes to read from the target device
 *
 * Returns:
 *   CYRET_SUCCESS  - The transfer completed successfully
 *   CYRET_ERR_COMM - There was a communication error talking to the device
 *
 *******************************************************************************/
int CyBtldr_TransferData(u8 *inBuf, int inSize, u8 *outBuf, int outSize);

int CyBtldr_WriteData(u8 *inBuf, int inSize);

/*******************************************************************************
 * Function Name: CyBtldr_ReadData
 ********************************************************************************
 * Summary:
 *   This function is responsible only for reading a response packet back from
 *   the device, e.g. in case we don't know initial response packet length and we
 *   need to firstly read dataLength field.
 *
 * Parameters:
 *   outBuf  - The buffer to store the data read from the device
 *   outSize - The number of bytes to read from the target device
 *
 * Returns:
 *   CYRET_SUCCESS  - The transfer completed successfully
 *   CYRET_ERR_COMM - There was a communication error talking to the device
 *
 *******************************************************************************/
int CyBtldr_ReadData(u8 *outBuf, int outSize);

/*******************************************************************************
 * Function Name: CyBtldr_StartBootloadOperation
 ********************************************************************************
 * Summary:
 *   Initiates a new bootload operation. This must be called before any other
 *   request to send data to the bootloader. A corresponding call to
 *   CyBtldr_EndBootloadOperation() should be made once all transactions are
 *   complete. This function is only used for applications using the .cyacd2
 *   format.
 *
 * Parameters:
 *   comm              – Communication struct used for communicating with the target device
 *   expSiId           - The Silicon ID of the device we expect to communicate with
 *   expSiRev          - The Silicon Rev of the device we expect to communicate with
 *   blVer             - The Bootloader version that is running on the device
 *   productID         - The product ID of the cyacd
 *
 * Returns:
 *   CYRET_SUCCESS     - The start request was sent successfully
 *   CYRET_ERR_DEVICE  - The detected device does not match the desired device
 *   CYRET_ERR_VERSION - The detected bootloader version is not compatible
 *   CYRET_ERR_BTLDR   - The bootloader experienced an error
 *   CYRET_ERR_COMM    - There was a communication error talking to the device
 *
 *******************************************************************************/
EXTERN int CyBtldr_StartBootloadOperation(struct CyBtldr_CommunicationsData *comm, u32 expSiId,
							u8 expSiRev, u32 *blVer, u64 productID);

/*******************************************************************************
 * Function Name: CyBtldr_EndBootloadOperation
 ********************************************************************************
 * Summary:
 *   Terminates the current bootload operation. This should be called once all
 *   bootload commands have been sent and no more communication is desired.
 *
 * Parameters:
 *   void.
 *
 * Returns:
 *   CYRET_SUCCESS   - The end request was sent successfully
 *   CYRET_ERR_BTLDR - The bootloader experienced an error
 *   CYRET_ERR_COMM  - There was a communication error talking to the device
 *
 *******************************************************************************/
EXTERN int CyBtldr_EndBootloadOperation(void);

/******************************************************************************
 * The following section contains API for applications using the .cyacd2 format
 ******************************************************************************/

/*******************************************************************************
 * Function Name: CyBtldr_ProgramRow
 ********************************************************************************
 * Summary:
 *   Sends a single row of data to the bootloader to be programmed into flash
 *   This function is only used for applications using the .cyacd2 format.
 *
 * Parameters:
 *   address – The flash address that is to be reprogrammed
 *   buf     – The buffer of data to program into the devices flash
 *   size    – The number of bytes in data that need to be sent to the bootloader
 *
 * Returns:
 *   CYRET_SUCCESS    - The row was programmed successfully
 *   CYRET_ERR_LENGTH - The result packet does not have enough data
 *   CYRET_ERR_DATA   - The result packet does not contain valid data
 *   CYRET_ERR_ARRAY  - The array is not valid for programming
 *   CYRET_ERR_ROW    - The array/row number is not valid for programming
 *   CYRET_ERR_BTLDR  - The bootloader experienced an error
 *   CYRET_ERR_ACTIVE - The application is currently marked as active
 *
 *******************************************************************************/
EXTERN int CyBtldr_ProgramRow(u32 address, u8 *buf, u16 size);

/*******************************************************************************
 * Function Name: CyBtldr_EraseRow
 ********************************************************************************
 * Summary:
 *   Erases a single row of flash data from the device. This function is only
 *   used for applications using the .cyacd2 format.
 *
 * Parameters:
 *   address - The flash address that is to be erased
 *
 * Returns:
 *   CYRET_SUCCESS    - The row was erased successfully
 *   CYRET_ERR_LENGTH - The result packet does not have enough data
 *   CYRET_ERR_DATA   - The result packet does not contain valid data
 *   CYRET_ERR_ARRAY  - The array is not valid for programming
 *   CYRET_ERR_ROW    - The array/row number is not valid for programming
 *   CYRET_ERR_BTLDR  - The bootloader experienced an error
 *   CYRET_ERR_COMM   - There was a communication error talking to the device
 *   CYRET_ERR_ACTIVE - The application is currently marked as active
 *
 *******************************************************************************/
EXTERN int CyBtldr_EraseRow(u32 address);

/*******************************************************************************
 * Function Name: CyBtldr_VerifyRow
 ********************************************************************************
 * Summary:
 *   Verifies that the data contained within the specified flash array and row
 *   matches the expected value. This function is only used for applications
 *   using the .cyacd2 format.
 *
 * Parameters:
 *   address  - The flash address that is to be verified
 *   buf      - The data to be verified in flash
 *   size     - The amount of data to verify
 *
 * Returns:
 *   CYRET_SUCCESS      - The row was verified successfully
 *   CYRET_ERR_LENGTH   - The result packet does not have enough data
 *   CYRET_ERR_DATA     - The result packet does not contain valid data
 *   CYRET_ERR_ARRAY	   - The array is not valid for programming
 *   CYRET_ERR_ROW      - The array/row number is not valid for programming
 *   CYRET_ERR_CHECKSUM - The checksum does not match the expected value
 *   CYRET_ERR_BTLDR    - The bootloader experienced an error
 *   CYRET_ERR_COMM     - There was a communication error talking to the device
 *
 *******************************************************************************/
EXTERN int CyBtldr_VerifyRow(u32 address, u8 *buf, u16 size);

/*******************************************************************************
 * Function Name: CyBtldr_VerifyApplication
 ********************************************************************************
 * Summary:
 *   Verifies that the checksum for the entire bootloadable application matches
 *   the expected value.  This is used to verify that the entire bootloadable
 *   image is valid and ready to execute. This function is only used for
 *   applications using the .cyacd2 format.
 *
 * Parameters:
 *   appId              - The application number
 *
 * Returns:
 *   CYRET_SUCCESS      - The application was verified successfully
 *   CYRET_ERR_LENGTH   - The result packet does not have enough data
 *   CYRET_ERR_DATA     - The result packet does not contain valid data
 *   CYRET_ERR_CHECKSUM - The checksum does not match the expected value
 *   CYRET_ERR_BTLDR    - The bootloader experienced an error
 *   CYRET_ERR_COMM     - There was a communication error talking to the device
 *
 *******************************************************************************/
EXTERN int CyBtldr_VerifyApplication(u8 appId);

/*******************************************************************************
 * Function Name: CyBtldr_SetApplicationStartAddress
 ********************************************************************************
 * Summary:
 *   Set the metadata for the giving application ID. This function is only used
 *   for applications using the .cyacd2 format.
 *
 * Parameters:
 *   appId               - Application ID number
 *   appStartAddr        - The Start Address to put into the metadata
 *   appSize             - The number of bytes in the application
 *
 * Returns:
 *   CYRET_SUCCESS      - The application was verified successfully
 *   CYRET_ERR_CMD      - ThE command was invalid
 *   CYRET_ERR_LENGTH   - The command length was incorrect
 *   CYRET_ERR_DATA     - The result packet does not contain valid data
 *   CYRET_ERR_CHECKSUM - The checksum does not match the expected value
 *
 *******************************************************************************/
EXTERN int CyBtldr_SetApplicationMetaData(u8 appId, u32 appStartAddr, u32 appSize);

/*******************************************************************************
 * Function Name: CyBtldr_SetEncryptionInitialVector
 ********************************************************************************
 * Summary:
 *   Set Encryption Initial Vector. This function is only used for applications
 *   using the .cyacd2 format.
 *
 * Parameters:
 *   size               - size of encryption initial vector
 *   buf                - encryption initial vector buffer
 *
 * Returns:
 *   CYRET_SUCCESS      - The application was verified successfully
 *   CYRET_ERR_CMD      - The command was invalid
 *   CYRET_ERR_LENGTH   - The command length was incorrect
 *   CYRET_ERR_DATA     - The result packet does not contain valid data
 *   CYRET_ERR_CHECKSUM - The checksum does not match the expected value
 *
 *******************************************************************************/
EXTERN int CyBtldr_SetEncryptionInitialVector(u16 size, u8 *buf);

#endif
