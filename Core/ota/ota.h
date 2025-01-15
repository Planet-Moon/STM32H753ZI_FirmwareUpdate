/*
 * ota.h
 *
 *  Created on: Jan 13, 2025
 *      Author: schroeder
 */

#ifndef OTA_OTA_H_
#define OTA_OTA_H_

#include "stdbool.h"
#include "stddef.h"

bool ota_begin(size_t new_fw_size);
bool ota_write(const void* buf, size_t len);
bool ota_end(void);

#endif /* OTA_OTA_H_ */
