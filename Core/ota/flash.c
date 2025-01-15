/*
 * flash.c
 *
 *  Created on: Jan 13, 2025
 *      Author: schroeder
 */
#include "../ota/flash.h"

#include "stdint.h"
#include "string.h"
#include "../ota/ota.h"
#include "../ota/ota_utils.h"

static char* s_addr; // Current address to write to
static size_t s_size; // Firmware size to flash. In-progress indicator
static uint32_t s_crc32; // Firmware checksum

bool ota_flash_begin(size_t new_fw_size, FlashItf* flashItf){
    bool ok = false;
    if(s_size){
        // OTA already in progress. Call ota_end()
    }
    else {
        size_t half = flashItf->size>>1; // size / 2
        s_crc32 = 0;
        s_addr = (char*)flashItf->start + half;
        if(new_fw_size < half){
            ok = true;
            s_size = new_fw_size;
            // Starting OTA, firmware size s_size
        }
        else{
            // Firmware too big to fit
        }
    }
    return ok;
}

bool ota_flash_write(const void* buf, size_t len, FlashItf* flashItf) {
    bool ok = false;
    if(s_size == 0){
        // OTA is not started, call ota_begin()
    }
    else{
        size_t len_aligned_down = ROUND_DOWN(len, flashItf->align);
        if(len_aligned_down){
            ok = flashItf->write_fn(s_addr, buf, len_aligned_down);
        }
        if(len_aligned_down < len){
            size_t left = len - len_aligned_down;
            char tmp[flashItf->align];
            memset(tmp, 0xff, sizeof(tmp));
            memcpy(tmp, (char*)buf + len_aligned_down, left);
            ok = flashItf->write_fn(s_addr + len_aligned_down, tmp, sizeof(tmp));
        }
        s_crc32 = crc32(s_crc32, (char*)buf, len); // Update CRC
        s_addr += len;
    }
    return ok;
}

bool ota_flash_end(FlashItf* flashItf) {
    char* base = (char*)flashItf->start + flashItf->size/2;
    bool ok = false;
    if(s_size){
        size_t size = (size_t)(s_addr - base);
        uint32_t crc32_ = crc32(0, base, s_size);
        if(size == s_size && crc32_ == s_crc32){
            ok = true;
        }
        s_size = 0;
        if(ok){
            ok = flashItf->swap_fn();
        }
    }
    // Finishing OTA
    return ok;
}
