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

#ifndef __CYBTLDR_UTILS_H__
#define __CYBTLDR_UTILS_H__

#define EXTERN extern
#define CALL_CON /**/

/******************************************************************************
 *    HOST ERROR CODES
 ******************************************************************************
 *
 * Different return codes from the bootloader host.  Functions are not
 * limited to these values, but are encouraged to use them when returning
 * standard error values.
 *
 * 0 is successful, all other values indicate a failure.
 *****************************************************************************/
/* Completed successfully */
#define CYRET_SUCCESS 0x00
/* File is not accessible */
#define CYRET_ERR_FILE 0x01
/* Reached the end of the file */
#define CYRET_ERR_EOF 0x02
/* The amount of data available is outside the expected range */
#define CYRET_ERR_LENGTH 0x03
/* The data is not of the proper form */
#define CYRET_ERR_DATA 0x04
/* The command is not recognized */
#define CYRET_ERR_CMD 0x05
/* The expected device does not match the detected device */
#define CYRET_ERR_DEVICE 0x06
/* The bootloader version detected is not supported */
#define CYRET_ERR_VERSION 0x07
/* The checksum does not match the expected value */
#define CYRET_ERR_CHECKSUM 0x08
/* The flash array is not valid */
#define CYRET_ERR_ARRAY 0x09
/* The flash row is not valid */
#define CYRET_ERR_ROW 0x0A
/* The bootloader is not ready to process data */
#define CYRET_ERR_BTLDR 0x0B
/* The application is currently marked as active */
#define CYRET_ERR_ACTIVE 0x0C
/* An unknown error occurred */
#define CYRET_ERR_UNK 0x0F
/* The unexpected data was received in a response packet */
#define CYRET_ERR_RESPONSE 0x10
/* The response data is too big */
#define CYRET_ERR_DATA_LEN 0x12
/* The operation was aborted */
#define CYRET_ABORT 0xAB

/* The communications object reported an error */
#define CYRET_ERR_COMM_MASK 0x2000
/* The bootloader reported an error */
#define CYRET_ERR_BTLDR_MASK 0x4000

/******************************************************************************
 *    BOOTLOADER STATUS CODES
 ******************************************************************************
 *
 * Different return status codes from the bootloader.
 *
 * 0 is successful, all other values indicate a failure.
 *****************************************************************************/
/* Completed successfully */
#define CYBTLDR_STAT_SUCCESS 0x00
/* The provided key does not match the expected value */
#define CYBTLDR_STAT_ERR_KEY 0x01
/* The verification of flash failed */
#define CYBTLDR_STAT_ERR_VERIFY 0x02
/* The amount of data available is outside the expected range */
#define CYBTLDR_STAT_ERR_LENGTH 0x03
/* The data is not of the proper form */
#define CYBTLDR_STAT_ERR_DATA 0x04
/* The command is not recognized */
#define CYBTLDR_STAT_ERR_CMD 0x05
/* The expected device does not match the detected device */
#define CYBTLDR_STAT_ERR_DEVICE 0x06
/* The bootloader version detected is not supported */
#define CYBTLDR_STAT_ERR_VERSION 0x07
/* The checksum does not match the expected value */
#define CYBTLDR_STAT_ERR_CHECKSUM 0x08
/* The flash array is not valid */
#define CYBTLDR_STAT_ERR_ARRAY 0x09
/* The flash row is not valid */
#define CYBTLDR_STAT_ERR_ROW 0x0A
/* The flash row is protected and can not be programmed */
#define CYBTLDR_STAT_ERR_PROTECT 0x0B
/* The application is not valid and cannot be set as active */
#define CYBTLDR_STAT_ERR_APP 0x0C
/* The application is currently marked as active */
#define CYBTLDR_STAT_ERR_ACTIVE 0x0D
/* An unknown error occurred */
#define CYBTLDR_STAT_ERR_UNK 0x0F

#endif
