/*
 * ota_utils.c
 *
 *  Created on: Jan 13, 2025
 *      Author: schroeder
 */

#include "../ota/ota_utils.h"

#include <stdlib.h>
#include "string.h"


uint32_t crc32(uint32_t crc, const char* buf, size_t len){
    static const uint32_t crclut[16] = {
        // table for polynomial 0xEDB88320 (reflected)
        0x00000000, 0x1DB71064, 0x3B6E20C8, 0x26D930AC, 0x76DC4190, 0x6B6B51F4,
        0x4DB26158, 0x5005713C, 0xEDB88320, 0xF00F9344, 0xD6D6A3E8, 0xCB61B38C,
        0x9B64C2B0, 0x86D3D2D4, 0xA00AE278, 0xBDBDF21C
    };
    crc = ~crc;
    while (len--) {
        uint8_t b = *(uint8_t *) buf++;
        crc = crclut[(crc ^ b) & 0x0F] ^ (crc >> 4);
        crc = crclut[(crc ^ (b >> 4)) & 0x0F] ^ (crc >> 4);
    }
    return ~crc;
}

uint16_t countAnds(const char* str) {
    uint16_t result = 0;
    const size_t strlen_ = strlen(str);
    for(int i = 0; i < strlen_; ++i){
        if(str[i] == '&'){
            result++;
        }
    }
    return result;
}

bool parseOTABeginRequest(const char* uri, uint16_t* chunks, uint32_t* contentLength) {
    // expected uri format: /upb&<chunks>&<contentLength(bytes)>
    if(strlen(uri) < 8){
        return false; // Uri too short to contain all necessary data
    }
    static const char prefix[] = "/upb&";
    if(memcmp(uri, prefix, 5)!=0){
        return false; // Begin not found
    }

    uint16_t delimCnt = countAnds(uri);
    if(delimCnt>2){
        return false; // Too many '&' found
    }

    char* ptr = strchr(uri+4,'&')+1;
    *chunks = atoi(ptr);

    ptr = strchr(ptr, '&')+1;
    *contentLength = atoi(ptr);
    return true;
}

bool parseOTAWriteRequest(const char* uri, uint16_t* chunkIdx, uint32_t* chunkSize) {
    // expected uri format: /upb&<chunks>&<contentLength(bytes)>
    if(strlen(uri) < 8){
        // uri not long enough
        return false;
    }

    static const char prefix[] = "/upw&";
    if(memcmp(uri, prefix, 5)!=0){
        return false; // Begin not found
    }

    uint16_t delimCount = countAnds(uri);
    if(delimCount > 2){
        // too many '&' found
        return false;
    }

    char* ptr = strchr(uri+4,'&')+1;
    *chunkIdx = atoi(ptr);

    ptr = strchr(ptr, '&')+1;
    *chunkSize = atoi(ptr);
    return true;
}

bool parseOTAEndRequest(const char* uri) {
    return strcmp(uri, "/upe")==0;
}
