/*
 * ota_utils.h
 *
 *  Created on: Jan 13, 2025
 *      Author: schroeder
 */

#ifndef OTA_OTA_UTILS_H_
#define OTA_OTA_UTILS_H_

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"

#define ROUND_DOWN(x,a) ((a) == 0 ? (x) : (((x)/(a))*(a)))

uint32_t crc32(uint32_t crc, const char* buf, size_t len);

bool parseOTABeginRequest(const char* uri, uint16_t* chunks, uint32_t* contentLength);
bool parseOTAWriteRequest(const char* uri, uint16_t* chunkIdx, uint32_t* chunkSize);
bool parseOTAEndRequest(const char* uri);

#endif /* OTA_OTA_UTILS_H_ */
