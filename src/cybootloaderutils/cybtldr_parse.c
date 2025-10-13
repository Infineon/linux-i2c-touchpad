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

#include "cybootloaderutils/cybtldr_parse.h"
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/uio.h>

/* Pointer to the *.cyacd file containing the data that is to be read */
static struct file *dataFile;

static u16 parse2ByteValueLittleEndian(u8 *buf)
{
	return ((u16)buf[0]) | (((u16)buf[1]) << 8);
}

static u32 parse4ByteValueLittleEndian(u8 *buf)
{
	return ((u32)parse2ByteValueLittleEndian(buf)) |
			(((u32)parse2ByteValueLittleEndian(buf + 2)) << 16);
}

u8 CyBtldr_FromHex(char value)
{
	if ('0' <= value && value <= '9')
		return (u8)(value - '0');

	if ('a' <= value && value <= 'f')
		return (u8)(10 + value - 'a');

	if ('A' <= value && value <= 'F')
		return (u8)(10 + value - 'A');

	return 0;
}

int CyBtldr_FromAscii(u32 bufSize, char *buffer, u16 *rowSize, u8 *rowData)
{
	u16 i;
	int err = CYRET_SUCCESS;

	if (bufSize & 1)  // Make sure even number of bytes
		err = CYRET_ERR_LENGTH;
	else {
		for (i = 0; i < bufSize / 2; i++)
			rowData[i] = (CyBtldr_FromHex(buffer[i * 2]) << 4) |
						CyBtldr_FromHex(buffer[i * 2 + 1]);
		*rowSize = i;
	}

	return err;
}

static int CyBtldr_FileGetString(struct file *file, char *buffer, size_t size)
{
	char ch;
	ssize_t bytes_read;
	int count = 0;

	if (!file || !buffer || size == 0)
		return -1;

	while (count < size) {
		bytes_read = kernel_read(file, &ch, 1, &file->f_pos);

		if (bytes_read < 0)
			return bytes_read; // Reading error

		if (bytes_read == 0) {
			// EOF reached
			if (count == 0)
				return 0; // Empty line at EOF
			break;
		}

		buffer[count++] = ch;

		if (ch == '\n')
			break; // End of line
	}

	buffer[count] = '\0'; // Null-terminate the string

	return count; // Return number of characters read
}

int CyBtldr_ReadLine(u32 *size, char *buffer)
{
	int err = CYRET_SUCCESS;
	u32 len;
	ssize_t bytes_read;

	/* line that start with '#' are assumed to be comments
	 * continue reading if we read a comment
	 */
	do {
		len = 0;
		if (dataFile != NULL) {
			bytes_read = CyBtldr_FileGetString(dataFile, buffer, MAX_BUFFER_SIZE * 2);

			if (bytes_read > 0) {
				// Remove trailing newline characters
				len = (u32) strlen(buffer);
				while (len > 0 && ('\n' == buffer[len - 1] ||
						'\r' == buffer[len - 1]))
					--len;
			} else if (bytes_read == 0) {
				err = CYRET_ERR_EOF;
			} else {
				err = CYRET_ERR_FILE;
			}
		} else
			err = CYRET_ERR_FILE;
	} while (err == CYRET_SUCCESS && buffer[0] == '#');

	*size = len;
	return err;
}

int CyBtldr_OpenDataFile(const char *file)
{
	dataFile = filp_open(file, O_RDONLY, 0);
	if (IS_ERR(dataFile)) {
		pr_err("Failed to open data file: %ld\n", PTR_ERR(dataFile));
		return CYRET_ERR_FILE;
	}

	return CYRET_SUCCESS;
}

int CyBtldr_CheckCyacdFileVersion(u32 bufSize, char *header, u8 *version)
{
	// check file extension of the file, if extension is cyacd, version 0
	int err = CYRET_SUCCESS;

	if (bufSize == 0)
		err = CYRET_ERR_FILE;

	if (bufSize < 2)
		err = CYRET_ERR_FILE;
	// .cyacd2 file stores version information in the first byte of the file header.
	if (err == CYRET_SUCCESS) {
		*version = CyBtldr_FromHex(header[0]) << 4 | CyBtldr_FromHex(header[1]);
		if (*version != 1)
			err = CYRET_ERR_DATA;
	}

	return err;
}

int CyBtldr_ParseHeader(u32 bufSize, char *buffer, u32 *siliconId, u8 *siliconRev,
			u8 *chksum, u8 *appID, u64 *productID)
{
	int err = CYRET_SUCCESS;
	u16 rowSize = 0;
	u8 rowData[MAX_BUFFER_SIZE];

	if (err == CYRET_SUCCESS)
		err = CyBtldr_FromAscii(bufSize, buffer, &rowSize, rowData);

	if (err == CYRET_SUCCESS) {
		if (rowSize == 12) {
			*siliconId = parse4ByteValueLittleEndian(rowData + 1);
			*siliconRev = rowData[5];
			*chksum = rowData[6];
			*appID = rowData[7];
			*productID = parse4ByteValueLittleEndian(rowData + 8);
		} else {
			err = CYRET_ERR_LENGTH;
		}
	}
	return err;
}

int CyBtldr_ParseCyAcd2RowData(u32 bufSize, char *buffer, u32 *address,
			u8 *rowData, u16 *size, u8 *checksum)
{
	const u16 MIN_SIZE = 4;  // 4-addr
	const int DATA_OFFSET = 4;

	unsigned int i;
	u16 hexSize;
	u8 hexData[MAX_BUFFER_SIZE];
	int err = CYRET_SUCCESS;

	if (bufSize <= MIN_SIZE)
		err = CYRET_ERR_LENGTH;
	else if (buffer[0] == ':') {
		err = CyBtldr_FromAscii(bufSize - 1, &buffer[1], &hexSize, hexData);

		if (err == CYRET_SUCCESS) {
			*address = parse4ByteValueLittleEndian(hexData);
			*checksum = 0;

			if (hexSize > MIN_SIZE) {
				*size = hexSize - MIN_SIZE;
				for (i = 0; i < *size; i++) {
					rowData[i] = (hexData[DATA_OFFSET + i]);
					*checksum += rowData[i];
				}
			} else
				err = CYRET_ERR_DATA;
		}
	} else
		err = CYRET_ERR_CMD;

	return err;
}

int CyBtldr_ParseCyAcdAppStartAndSize(u32 *appStart, u32 *appSize, u32 *dataLines, char *buf)
{
	const u32 APPINFO_META_HEADER_SIZE = 11;
	static const char APPINFO_META_HEADER[] = "@APPINFO:0x";
	const u32 EIV_META_HEADER_SIZE = 5;
	static const char EIV_META_HEADER[] = "@EIV:";
	const u32 APPINFO_META_SEPARATOR_SIZE = 3;
	static const char APPINFO_META_SEPARATOR[] = ",0x";
	static const char APPINFO_META_SEPARATOR_START[] = ",";

	long fp = dataFile->f_pos; // Save current position in the file
	*appStart = 0xffffffff;
	*appSize = 0;
	*dataLines = 0;
	u32 addr = 0;
	u32 rowLength;
	u16 rowSize;
	u32 separatorIndex;
	u8 rowData[MAX_BUFFER_SIZE];
	u8 checksum;
	int err = CYRET_SUCCESS;
	u32 i;
	bool appInfoFound = false;

	do {
		err = CyBtldr_ReadLine(&rowLength, buf);
		if (err == CYRET_SUCCESS) {
			if (buf[0] == ':') {
				if (!appInfoFound) {
					err = CyBtldr_ParseCyAcd2RowData(rowLength, buf, &addr,
						rowData, &rowSize, &checksum);

					if (addr < (*appStart))
						*appStart = addr;

					(*appSize) += rowSize;
				}
				++(*dataLines);
			} else if (rowLength >= APPINFO_META_HEADER_SIZE &&
					strncmp(buf, APPINFO_META_HEADER,
							APPINFO_META_HEADER_SIZE) == 0) {
				// find separator index
				separatorIndex = (u32)strcspn(buf, APPINFO_META_SEPARATOR_START);
				if (strncmp(buf + separatorIndex,
							APPINFO_META_SEPARATOR,
							APPINFO_META_SEPARATOR_SIZE) == 0) {
					*appStart = 0;
					*appSize = 0;
					for (i = APPINFO_META_HEADER_SIZE;
							i < separatorIndex; i++) {
						*appStart <<= 4;
						*appStart += CyBtldr_FromHex(buf[i]);
					}
					for (i = separatorIndex + APPINFO_META_SEPARATOR_SIZE;
							i < rowLength; i++) {
						*appSize <<= 4;
						*appSize += CyBtldr_FromHex(buf[i]);
					}
				} else {
					err = CYRET_ERR_FILE;
				}
				appInfoFound = true;
			} else if (strncmp(buf, EIV_META_HEADER, EIV_META_HEADER_SIZE) != 0) {
				err = CYRET_ERR_FILE;
			}
		}
	} while (err == CYRET_SUCCESS);
	if (err == CYRET_ERR_EOF)
		err = CYRET_SUCCESS;
	// reset to the file to where we were
	if (err == CYRET_SUCCESS) {
		if (default_llseek(dataFile, fp, SEEK_SET) < 0) {
		// shouldn't be possible, we're just going to somewhere that was valid before
			err = CYRET_ERR_EOF;
		}
	}

	return err;
}

int CyBtldr_CloseDataFile(void)
{
	if (dataFile) {
		filp_close(dataFile, NULL);
		dataFile = NULL;
	}
	return CYRET_SUCCESS;
}
