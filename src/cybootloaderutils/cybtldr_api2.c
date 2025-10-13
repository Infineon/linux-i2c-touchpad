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

#include <linux/string.h>
#include "cybootloaderutils/cybtldr_api.h"
#include "cybootloaderutils/cybtldr_api2.h"

u8 g_abort;

static int ProcessDataRow(enum CyBtldr_Action action, u32 rowSize, char *rowData,
	CyBtldr_ProgressUpdate *update, u32 applicationStartAddr, u32 applicationDataLines,
	u32 *applicationDataLinesSeen)
{
	u8 buffer[MAX_BUFFER_SIZE];
	u16 bufSize;
	u32 address;
	u8 checksum;

	int err = CyBtldr_ParseCyAcd2RowData(rowSize, rowData, &address,
			buffer, &bufSize, &checksum);

	if (err == CYRET_SUCCESS) {
		switch (action) {
		case ERASE:
			err = CyBtldr_EraseRow(address);
			break;
		case PROGRAM:
			err = CyBtldr_ProgramRow(address, buffer, bufSize);
			break;
		case VERIFY:
			err = CyBtldr_VerifyRow(address, buffer, bufSize);
			break;
		}
	}

	return err;
}

static int ProcessMetaRow(u32 rowSize, char *rowData)
{
	const u32 EIV_META_HEADER_SIZE = 5;
	static const char EIV_META_HEADER[] = "@EIV:";

	u8 buffer[MAX_BUFFER_SIZE];
	u16 bufSize = 0;

	int err = CYRET_SUCCESS;

	if (rowSize >= EIV_META_HEADER_SIZE &&
		strncmp(rowData, EIV_META_HEADER, EIV_META_HEADER_SIZE) == 0) {
		err = CyBtldr_FromAscii(rowSize - EIV_META_HEADER_SIZE,
			rowData + EIV_META_HEADER_SIZE, &bufSize, buffer);
		if (err == CYRET_SUCCESS)
			err = CyBtldr_SetEncryptionInitialVector(bufSize, buffer);
	}
	return err;
}

int CyBtldr_RunAction(enum CyBtldr_Action action, struct CyBtldr_CommunicationsData *comm,
		CyBtldr_ProgressUpdate *update, const char *file)
{
	g_abort = 0;
	u32 lineLen;
	char line[MAX_BUFFER_SIZE * 2];  // 2 hex characters per byte
	u8 fileVersion = 0;

	int err = CyBtldr_OpenDataFile(file);

	if (err == CYRET_SUCCESS) {
		err = CyBtldr_ReadLine(&lineLen, line);
		// the following functions checks
		if (err == CYRET_SUCCESS)
			err = CyBtldr_CheckCyacdFileVersion(lineLen, line, &fileVersion);

		if (err == CYRET_SUCCESS) {
			u32 blVer = 0;
			u32 siliconId = 0;
			u8 siliconRev = 0;
			u8 chksumtype = SUM_CHECKSUM;
			u8 appId = 0;
			u8 bootloaderEntered = 0;
			u32 applicationStartAddr = 0xffffffff;
			u32 applicationSize = 0;
			u32 applicationDataLines = 255;
			u32 applicationDataLinesSeen = 0;
			u64 productId = 0;

			err = CyBtldr_ParseHeader(lineLen, line, &siliconId, &siliconRev,
				&chksumtype, &appId, &productId);

			if (err == CYRET_SUCCESS) {
				CyBtldr_SetCheckSumType(chksumtype);

				// send ENTER DFU command to start communication
				err = CyBtldr_StartBootloadOperation(comm, siliconId,
					siliconRev, &blVer, productId);

				// send Set Application Metadata command
				if (err == CYRET_SUCCESS)
					err = CyBtldr_ParseCyAcdAppStartAndSize(
						&applicationStartAddr, &applicationSize,
						&applicationDataLines, line);
				if (err == CYRET_SUCCESS)
					err = CyBtldr_SetApplicationMetaData(appId,
						applicationStartAddr, applicationSize);
				bootloaderEntered = 1;
			}


			while (err == CYRET_SUCCESS) {
				if (g_abort) {
					g_abort = 0;
					err = CYRET_ABORT;
					break;
				}

				err = CyBtldr_ReadLine(&lineLen, line);
				if (err == CYRET_SUCCESS) {
					switch (line[0]) {
					case '@':
						err = ProcessMetaRow(lineLen, line);
						break;
					case ':':
						err = ProcessDataRow(action, lineLen, line, update,
							applicationStartAddr, applicationDataLines,
							&applicationDataLinesSeen);
						break;
					}
				} else if (err == CYRET_ERR_EOF) {
					err = CYRET_SUCCESS;
					break;
				}
			}
			if (err == CYRET_SUCCESS && (action == PROGRAM || action == VERIFY)) {
				err = CyBtldr_VerifyApplication(appId);
				CyBtldr_EndBootloadOperation();
			} else if (CYRET_ERR_COMM_MASK != (CYRET_ERR_COMM_MASK & err) &&
						bootloaderEntered) {
				CyBtldr_EndBootloadOperation();
			}
		}
		CyBtldr_CloseDataFile();
	}
	return err;
}

int CyBtldr_Program(const char *file, struct CyBtldr_CommunicationsData *comm,
				CyBtldr_ProgressUpdate *update)
{
	return CyBtldr_RunAction(PROGRAM, comm, update, file);
}

int CyBtldr_Erase(const char *file, struct CyBtldr_CommunicationsData *comm,
				CyBtldr_ProgressUpdate *update)
{
	return CyBtldr_RunAction(ERASE, comm, update, file);
}

int CyBtldr_Verify(const char *file, struct CyBtldr_CommunicationsData *comm,
				CyBtldr_ProgressUpdate *update)
{
	return CyBtldr_RunAction(VERIFY, comm, update, file);
}

int CyBtldr_Abort(void)
{
	g_abort = 1;
	return CYRET_SUCCESS;
}
