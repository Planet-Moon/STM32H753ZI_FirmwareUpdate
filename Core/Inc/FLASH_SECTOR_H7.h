#define EXCLUDE_FROM_BUILD
#ifndef EXCLUDE_FROM_BUILD
/**
  ***************************************************************************************************************
  ***************************************************************************************************************
  ***************************************************************************************************************

  File:       FLASH_SECTOR_H7.h
  Modifier:   ControllersTech.com
  Updated:    27th MAY 2021

  ***************************************************************************************************************
  Copyright (C) 2017 ControllersTech.com

  This is a free software under the GNU license, you can redistribute it and/or modify it under the terms
  of the GNU General Public License version 3 as published by the Free Software Foundation.
  This software library is shared with public for educational purposes, without WARRANTY and Author is not liable for any damages caused directly
  or indirectly by this software, read more about this on the GNU General Public License.

  ***************************************************************************************************************
*/

// https://github.com/controllerstech/STM32/blob/master/FLASH_PROGRAM/H7%20SERIES

#ifndef INC_FLASH_SECTOR_H7_H_
#define INC_FLASH_SECTOR_H7_H_

#include "stm32h7xx_hal.h"

#define APPLICATION_ADDRESS (uint32_t)0x08100000
#define APPLICATION_END_ADDRESS (uint32_t)0x081FFFFF

/* Exported constants --------------------------------------------------------*/


/* Error code */
typedef enum
{
  FLASHIF_OK = 0,
  FLASHIF_ERASEKO,
  FLASHIF_WRITINGCTRL_ERROR,
  FLASHIF_WRITING_ERROR,
  FLASHIF_PROTECTION_ERRROR
} FlashErrorCode;

/* protection type */
typedef enum{
  FLASHIF_PROTECTION_NONE         = 0,
  FLASHIF_PROTECTION_PCROPENABLED = 0x1,
  FLASHIF_PROTECTION_WRPENABLED   = 0x2,
  FLASHIF_PROTECTION_RDPENABLED   = 0x4,
} FlashProtectionType;

/* protection update */
typedef enum {
    FLASHIF_WRP_ENABLE,
    FLASHIF_WRP_DISABLE
} FlashProtectionUpdate;

/* Functions */
int numofwords(size_t length);
uint32_t Flash_Write_Data (uint32_t StartSectorAddress, uint32_t *data, uint16_t numberofwords);
void Flash_Read_Data (uint32_t StartSectorAddress, uint32_t *data, uint16_t numberofwords);
void Convert_To_Str (uint32_t *Data, char *Buf);
void FlashDataToStr(uint32_t* data, uint16_t size, char* buf);
FlashProtectionType FlashGetWriteProtectionStatus();

void Flash_Write_NUM (uint32_t StartSectorAddress, float Num);
float Flash_Read_NUM (uint32_t StartSectorAddress);

#endif /* INC_FLASH_SECTOR_H7_H_ */
#endif
