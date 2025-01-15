/*
 * flash.h
 *
 *  Created on: Jan 13, 2025
 *      Author: schroeder
 */

#ifndef OTA_FLASH_H_
#define OTA_FLASH_H_

#include "stddef.h"
#include "stdbool.h"

typedef struct {
    void* start; // Address at which flash starts
    size_t size; // Flash size
    size_t secsz; // Sector size
    size_t align; // Write alignment
    bool (*write_bgn_fn)(); // flash write begin function
    bool (*write_end_fn)(); // flash write end function
    bool (*write_fn)(void*, const void*, size_t); // Write function
    bool (*swap_fn)(void); // Swap partitions
} FlashItf;

bool ota_flash_begin(size_t new_fw_size, FlashItf* flashItf);
bool ota_flash_write(const void* buf, size_t len, FlashItf* flashItf);
bool ota_flash_end(FlashItf* flashItf);

#endif /* OTA_FLASH_H_ */
