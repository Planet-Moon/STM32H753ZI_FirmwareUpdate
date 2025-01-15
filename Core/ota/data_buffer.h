/*
 * data_buffer.h
 *
 *  Created on: Jan 14, 2025
 *      Author: schroeder
 */

#ifndef OTA_DATA_BUFFER_H_
#define OTA_DATA_BUFFER_H_

#include "stdint.h"
#include "stddef.h"

#define DATABUFFER_SIZE 128*1024

typedef struct {
    char buf[DATABUFFER_SIZE];
    uint32_t bufferEndIdx;
} DataBuffer;

uint32_t DataBufferAvailable(DataBuffer* db);

void DataBufferClear(DataBuffer* db);

int32_t DataBufferAppend(DataBuffer* db, const char* data, size_t len);

#endif /* OTA_DATA_BUFFER_H_ */
