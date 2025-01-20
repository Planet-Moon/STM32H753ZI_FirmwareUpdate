/*
 * data_buffer.c
 *
 *  Created on: Jan 14, 2025
 *      Author: schroeder
 */

#include "data_buffer.h"
#include "string.h"

uint32_t DataBufferAvailable(DataBuffer* db) {
    return DATABUFFER_SIZE - db->bufferEndIdx;
}

void DataBufferClear(DataBuffer* db){
    db->bufferEndIdx = 0;
    memset(db->buf, 0xFF, DATABUFFER_SIZE);
}

int32_t DataBufferAppend(DataBuffer* db, const char* data, size_t len){
    uint32_t remainingSpace = DataBufferAvailable(db);
    if(remainingSpace < len){
        memcpy(db->buf + db->bufferEndIdx, data, remainingSpace);
        db->bufferEndIdx = DATABUFFER_SIZE;
        return remainingSpace - len;
    }
    else{
        memcpy(db->buf + db->bufferEndIdx, data, len);
        db->bufferEndIdx += len;
        return DataBufferAvailable(db);
    }
}

void DataBufferExpect(DataBuffer* db, uint32_t expectedLen) {
    db->expectedLen = expectedLen;
}

int32_t DataBufferExpectReached(DataBuffer* db) {
    return db->bufferEndIdx - db->expectedLen;
}
