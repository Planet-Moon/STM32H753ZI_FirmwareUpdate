/**
  ******************************************************************************
  * @file    IAP_Main/Src/flash_if.c 
  * @author  MCD Application Team
  * @version 1.0.0
  * @date    8-April-2015
  * @brief   This file provides all the memory related operation functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "flash_if.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Unlocks Flash for write access
  * @param  None
  * @retval None
  */
void FLASH_If_Init(void)
{
  /* Unlock the Program memory */
  HAL_FLASH_Unlock();

  /* Clear all FLASH flags */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK1);
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK2);
  /* Unlock the Program memory */
  HAL_FLASH_Lock();
}

/**
  * @brief  This function does an erase of all user flash area
  * @param  start: start of user flash area
  * @retval FLASHIF_OK : user flash area successfully erased
  *         FLASHIF_ERASEKO : error occurred
  */
uint32_t FLASH_If_Erase(uint32_t start)
{
    return 0;
}	

/* Public functions ---------------------------------------------------------*/
/**
  * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @param  destination: start address for target location
  * @param  p_source: pointer on buffer with data to write
  * @param  length: length of data buffer (unit is 32-bit word)
  * @retval uint32_t 0: Data successfully written to Flash memory
  *         1: Error occurred while writing data in Flash memory
  *         2: Written Data in flash memory is different from expected one
  */
uint32_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length)
{
    uint32_t retval = 0;
    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef pEraseInit;
    uint32_t SectorError = 0;
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
    if(status != HAL_OK){
        uint32_t flashError = HAL_FLASH_GetError();
        retval = 1;
    }
    else{
        uint32_t data = 5;
        length = 1;
        for(size_t i = 0; i < length; ++i){
            status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, destination + i*16, data);
            if(status != HAL_OK){
                uint32_t flashError = HAL_FLASH_GetError();
                retval = 2;
                break;
            }
        }
    }
    HAL_FLASH_Lock();
    return retval;
}	

/**
  * @brief  Returns the write protection status of application flash area.
  * @param  None
  * @retval If a sector in application area is write-protected returned value is a combinaison
            of the possible values : FLASHIF_PROTECTION_WRPENABLED, FLASHIF_PROTECTION_PCROPENABLED, ...
  *         If no sector is write-protected FLASHIF_PROTECTION_NONE is returned.
  */
uint32_t FLASH_If_GetWriteProtectionStatus(void)
{

    uint32_t ProtectedPAGE = FLASHIF_PROTECTION_NONE;
    /*
    FLASH_OBProgramInitTypeDef OptionsBytesStruct1, OptionsBytesStruct2, OptionsBytesStruct3, OptionsBytesStruct4;

    // Unlock the Flash to enable the flash control register access
    HAL_FLASH_Unlock();

    OptionsBytesStruct1.Banks = FLASH_BANK_1;
    OptionsBytesStruct2.Banks = FLASH_BANK_2;

    // Check if there are write protected sectors inside the user flash area
    HAL_FLASHEx_OBGetConfig(&OptionsBytesStruct1);
    HAL_FLASHEx_OBGetConfig(&OptionsBytesStruct2);

    // Lock the Flash to disable the flash control register access (recommended
    // to protect the FLASH memory against possible unwanted operation)
    HAL_FLASH_Lock();

    // Check PCROP areas
    if(OptionsBytesStruct1.PCROPEndAddr > OptionsBytesStruct1.PCROPStartAddr)
    {
      // check if user area are included inside this range
      if(OptionsBytesStruct1.PCROPStartAddr > APPLICATION_ADDRESS)
      {
        ProtectedPAGE|= FLASHIF_PROTECTION_PCROPENABLED;
      }
    }

    if(OptionsBytesStruct2.PCROPEndAddr > OptionsBytesStruct2.PCROPStartAddr)
    {
      // check if user area are included inside this range
      if(OptionsBytesStruct1.PCROPStartAddr > APPLICATION_ADDRESS)
      {
         ProtectedPAGE|= FLASHIF_PROTECTION_PCROPENABLED;
      }
    }

    // check WRP
    if(OptionsBytesStruct1.SecureAreaEndAddr > OptionsBytesStruct1.SecureAreaStartAddr)
    {
      // check if area is inside the WRP Range
      if((OptionsBytesStruct1.SecureAreaStartAddr * FLASH_PAGE_SIZE + FLASH_BASE) >= APPLICATION_ADDRESS)
      {
         ProtectedPAGE|= FLASHIF_PROTECTION_WRPENABLED;
      }
    }

    if(OptionsBytesStruct2.WRPEndOffset > OptionsBytesStruct2.WRPStartOffset)
    {
      // check if area is inside the WRP Range
      if((OptionsBytesStruct2.WRPStartOffset * FLASH_PAGE_SIZE + FLASH_BASE) >= APPLICATION_ADDRESS)
      {
         ProtectedPAGE|= FLASHIF_PROTECTION_WRPENABLED;
      }
    }

    if(OptionsBytesStruct3.WRPEndOffset > OptionsBytesStruct3.WRPStartOffset)
    {
      // check if area is inside the WRP Range
      if((OptionsBytesStruct3.WRPStartOffset * FLASH_PAGE_SIZE + FLASH_BASE + FLASH_PAGE_SIZE * FLASH_PAGE_NBPERBANK) >= APPLICATION_ADDRESS)
      {
         ProtectedPAGE|= FLASHIF_PROTECTION_WRPENABLED;
      }
    }

    if(OptionsBytesStruct4.WRPEndOffset > OptionsBytesStruct4.WRPStartOffset)
    {
      // check if area is inside the WRP Range
      if((OptionsBytesStruct4.WRPStartOffset * FLASH_PAGE_SIZE + FLASH_BASE + FLASH_PAGE_SIZE * FLASH_PAGE_NBPERBANK) >= APPLICATION_ADDRESS)
      {
         ProtectedPAGE|= FLASHIF_PROTECTION_WRPENABLED;
      }
    }

    if(OptionsBytesStruct4.RDPLevel == 1)
    {
      ProtectedPAGE|= FLASHIF_PROTECTION_RDPENABLED;
    }
    */

    return ProtectedPAGE;
}

/**
  * @brief  Configure the write protection status of user flash area.
  * @param  protectionstate : FLASHIF_WRP_DISABLE or FLASHIF_WRP_ENABLE the protection
  * @retval uint32_t FLASHIF_OK if change is applied.
  */
uint32_t FLASH_If_WriteProtectionConfig(uint32_t protectionstate)
{
    return 0;
}
/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
