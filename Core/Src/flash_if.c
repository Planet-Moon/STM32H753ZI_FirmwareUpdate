/**
  ******************************************************************************
  * @file    IAP_Main/Src/flash_if.c
  * @author  MCD Application Team
  * @brief   This file provides all the memory related operation functions.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/** @addtogroup STM32H7xx_IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "flash_if.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static uint32_t GetSector(uint32_t Address);

/* Private functions ---------------------------------------------------------*/


FlashErrors FLASH_If_Errors() {
    uint32_t errors = HAL_FLASH_GetError();
    FlashErrors result;
    result.bank1.WriteProtErr = errors & HAL_FLASH_ERROR_WRP_BANK1;
    result.bank1.ProgSeqErr = errors & HAL_FLASH_ERROR_PGS_BANK1;
    result.bank1.StrobeErr = errors & HAL_FLASH_ERROR_STRB_BANK1;
    result.bank1.InconErr = errors & HAL_FLASH_ERROR_INC_BANK1;
    result.bank1.OpErr = errors & HAL_FLASH_ERROR_OPE_BANK1;
    result.bank1.ReadProtErr = errors & HAL_FLASH_ERROR_RDP_BANK1;
    result.bank1.ReadSecErr = errors & HAL_FLASH_ERROR_RDS_BANK1;
    result.bank1.SECCErr = errors & HAL_FLASH_ERROR_SNECC_BANK1;
    result.bank1.DECCErr = errors & HAL_FLASH_ERROR_DBECC_BANK1;
    result.bank1.CRCRErr = errors & HAL_FLASH_ERROR_CRCRD_BANK1;

    result.bank2.WriteProtErr = errors & HAL_FLASH_ERROR_WRP_BANK2;
    result.bank2.ProgSeqErr = errors & HAL_FLASH_ERROR_PGS_BANK2;
    result.bank2.StrobeErr = errors & HAL_FLASH_ERROR_STRB_BANK2;
    result.bank2.InconErr = errors & HAL_FLASH_ERROR_INC_BANK2;
    result.bank2.OpErr = errors & HAL_FLASH_ERROR_OPE_BANK2;
    result.bank2.ReadProtErr = errors & HAL_FLASH_ERROR_RDP_BANK2;
    result.bank2.ReadSecErr = errors & HAL_FLASH_ERROR_RDS_BANK2;
    result.bank2.SECCErr = errors & HAL_FLASH_ERROR_SNECC_BANK2;
    result.bank2.DECCErr = errors & HAL_FLASH_ERROR_DBECC_BANK2;
    result.bank2.CRCRErr = errors & HAL_FLASH_ERROR_CRCRD_BANK2;
    return result;
}

/**
  * @brief  Unlocks Flash for write access
  * @param  None
  * @retval None
  */
void FLASH_If_Init(void)
{

  /* Clear pending flags (if any) */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                         FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR);
}

/**
  * @brief  This function does an erase of all user flash area
  * @param  StartSector: start of user flash area
  * @retval 0: user flash area successfully erased
  *         1: error occurred
  */
uint32_t FLASH_If_Erase(uint32_t StartSector)
{
  uint32_t _StartSector;
  uint32_t SectorError;
  FLASH_EraseInitTypeDef pEraseInit;

  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();
  FLASH_If_Init();

  /* Get the sector where start the user flash area */
  _StartSector = GetSector(FLASH_BANK2_ADDRESS);

  pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
  pEraseInit.Sector = _StartSector;
  pEraseInit.NbSectors = 8 - _StartSector;
  pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

  if (FLASH_BANK2_ADDRESS < ADDR_FLASH_SECTOR_0_BANK2)
  {
    pEraseInit.Banks = FLASH_BANK_1;
    SCB_DisableICache();
    if (HAL_FLASHEx_Erase(&pEraseInit, &SectorError) != HAL_OK)
    {
      /* Error occurred while sector erase */
      return (1);
    }
    SCB_EnableICache();

    /* Mass erase of second bank */
    pEraseInit.TypeErase = FLASH_TYPEERASE_MASSERASE;
    pEraseInit.Banks = FLASH_BANK_2;
    SCB_DisableICache();
    if (HAL_FLASHEx_Erase(&pEraseInit, &SectorError) != HAL_OK)
    {
      /* Error occurred while sector erase */
      return (1);
    }
    SCB_EnableICache();
  }
  else
  {
    pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    pEraseInit.Banks = FLASH_BANK_2;
    SCB_DisableICache();
    if (HAL_FLASHEx_Erase(&pEraseInit, &SectorError) != HAL_OK)
    {
      /* Error occurred while sector erase */
      return (1);
    }
    SCB_EnableICache();
  }
  HAL_FLASH_Lock();
  return (0);
}


/**
  * @brief  This function does an erase of Flash bank 2
  * @retval 0: user flash area successfully erased
  *         1: error occurred
  */
uint32_t FLASH_If_EraseBank2() {
    // from RM0433 page 167:
    // 1. Check and clear (optional) all the error flags due to previous programming/erase
    // operation. Refer to Section 4.7: FLASH error management for details.
    // 2. Unlock FLASH_OPTCR register, as described in Section 4.5.1: FLASH configuration
    // protection (only if register is not already unlocked).
    // 3. If a PCROP-protected area exists set DMEP1/2 bit in FLASH_PRAR_PRG1/2 register.
    // In addition, program the PCROP area end and start addresses so that the difference is
    // negative, i.e. PROT_AREA_END1/2 < PROT_AREA_START1/2.
    // 4. If a secure-only area exists set DMES1/2 bit in FLASH_SCAR_PRG1/2 register. In
    // addition, program the secure-only area end and start addresses so that the difference
    // is negative, i.e. SEC_AREA_END1/2 < SEC_AREA_START1/2.
    // 5. Set all WRPSn1/2 bits in FLASH_WPSN_PRG1/2R to 1 to disable all sector write
    // protection.
    // 6. Unlock FLASH_CR1/2 register, only if register is not already unlocked.
    // 7. Set the BER1/2 bit in the FLASH_CR1/2 register corresponding to the target bank.
    // 8. Set the START1/2 bit in the FLASH_CR1/2 register to start the bank erase with
    // protection removal operation. Then wait until the QW1/2 bit is cleared in the
    // corresponding FLASH_SR1/2 register. At that point a bank erase operation has erased
    // the whole bank including the sectors containing PCROP-protected and/or secure-only
    // data, and an option byte change has been automatically performed so that all the
    // protections are disabled.
    // Note: BER1/2 and START1/2 bits can be set together, so above steps 8 and 9 can be merged.
    // Be aware of the following warnings regarding to above sequence:
    // • It is not possible to perform the above sequence on one bank while modifying the
    // protection parameters of the other bank.
    // • No other option bytes than the one indicated above must be changed, and no
    // protection change must be performed in the bank that is not targeted by the bank erase
    // with protection removal request.
    // • When one or both of the events above occurs, a simple bank erase occurs, no option
    // byte change is performed and no option change error is set.

    { // 1. Check error flags
        uint32_t flashError = HAL_FLASH_GetError();
        if(flashError){
            return 1;
        }
    }
    {
        HAL_StatusTypeDef halStatus = HAL_FLASHEx_Unlock_Bank2();
        if(halStatus != HAL_OK){
            return 2;
        }
    }
    {
        HAL_StatusTypeDef halStatus = HAL_FLASH_OB_Unlock();
        if(halStatus != HAL_OK){
            return 3;
        }
    }
    {
        FLASH_EraseInitTypeDef pEraseInit;
        pEraseInit.TypeErase = FLASH_TYPEERASE_MASSERASE; // or FLASH_TYPEERASE_SECTORS
        pEraseInit.Banks = FLASH_BANK_2;
        pEraseInit.Sector = FLASH_SECTOR_0; // initial flash sector to be erased. Optional for mass erase
        pEraseInit.NbSectors = 1; // number of sectors to be erased
        pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_4; // Flash program/erase by 64 bits
        uint32_t SectorError = 0; // SectorError only valid in sector erase
        HAL_StatusTypeDef halStatus = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
        if(halStatus != HAL_OK){
            return 4;
        }
    }

    HAL_FLASH_OB_Lock();
    HAL_FLASHEx_Lock_Bank1();
    // uint32_t flashError = HAL_FLASH_GetError();
    return 0;
}

/**
  * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @param  FlashAddress: start address for writing data buffer
  * @param  Data: pointer on data buffer
  * @param  DataLength: length of data buffer (unit is 32-bit word)
  * @retval 0: Data successfully written to Flash memory
  *         1: Error occurred while writing data in Flash memory
  *         2: Written Data in flash memory is different from expected one
  */
uint32_t FLASH_If_Write(uint32_t FlashAddress, uint32_t* Data ,uint32_t DataLength)
{
  uint32_t i = 0;
 /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();
  SCB_DisableICache();
  for (i = 0; (i < DataLength) && (FlashAddress <= (USER_FLASH_END_ADDRESS-32)); i+=8)
  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, FlashAddress, ((uint32_t)(Data+i))) == HAL_OK)
    {
     /* Check the written value */
      if (*(uint32_t*)FlashAddress != *(uint32_t*)(Data+i))
      {
        /* Flash content doesn't match SRAM content */
        return(FLASHIF_WRITINGCTRL_ERROR);
      }
      /* Increment FLASH destination address */
      FlashAddress += 32;
    }
    else
    {
      /* Error occurred while writing data in Flash memory */
      return (FLASHIF_WRITING_ERROR);
    }
  }
  SCB_EnableICache();
  HAL_FLASH_Lock();

  return (FLASHIF_OK);
}

/**
  * @brief  Returns the write protection status of user flash area.
  * @param  None
  * @retval 0: No write protected sectors inside the user flash area
  *         1: Some sectors inside the user flash area are write protected
  */
uint16_t FLASH_If_GetWriteProtectionStatus(void)
{
  uint32_t ProtectedSECTOR = 0x0;
  FLASH_OBProgramInitTypeDef OptionsBytesStruct;

  if (FLASH_BANK2_ADDRESS < ADDR_FLASH_SECTOR_0_BANK2)
  {
    /* Select Bank1 */
    OptionsBytesStruct.Banks = FLASH_BANK_1;

    /* Check if there are write protected sectors inside the user flash area ****/
    HAL_FLASHEx_OBGetConfig(&OptionsBytesStruct);

    /* Get pages already write protected ****************************************/
    ProtectedSECTOR = OptionsBytesStruct.WRPSector & FLASH_SECTOR_TO_BE_PROTECTED;
  }
  else
  {
  /* Select Bank2*/
  OptionsBytesStruct.Banks = FLASH_BANK_2;

  /* Check if there are write protected sectors inside the user flash area ****/
  HAL_FLASHEx_OBGetConfig(&OptionsBytesStruct);

  /* Get pages already write protected ****************************************/
  ProtectedSECTOR |= OptionsBytesStruct.WRPSector & FLASH_SECTOR_TO_BE_PROTECTED;
  }
  /* Check if desired pages are already write protected ***********************/
  if(ProtectedSECTOR != 0)
  {
    /* Some sectors inside the user flash area are write protected */
    return FLASHIF_PROTECTION_WRPENABLED;
  }
  else
  {
    /* No write protected sectors inside the user flash area */
    return FLASHIF_PROTECTION_NONE;
  }
}

/**
  * @brief  Gets the sector of a given address
  * @param  Address: Flash address
  * @retval The sector of a given address
  */
static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if((Address < ADDR_FLASH_SECTOR_1_BANK1) && (Address >= ADDR_FLASH_SECTOR_0_BANK1))
  {
    sector = FLASH_SECTOR_0;
  }
  else if((Address < ADDR_FLASH_SECTOR_2_BANK1) && (Address >= ADDR_FLASH_SECTOR_1_BANK1))
  {
    sector = FLASH_SECTOR_1;
  }
  else if((Address < ADDR_FLASH_SECTOR_3_BANK1) && (Address >= ADDR_FLASH_SECTOR_2_BANK1))
  {
    sector = FLASH_SECTOR_2;
  }
  else if((Address < ADDR_FLASH_SECTOR_4_BANK1) && (Address >= ADDR_FLASH_SECTOR_3_BANK1))
  {
    sector = FLASH_SECTOR_3;
  }
  else if((Address < ADDR_FLASH_SECTOR_5_BANK1) && (Address >= ADDR_FLASH_SECTOR_4_BANK1))
  {
    sector = FLASH_SECTOR_4;
  }
  else if((Address < ADDR_FLASH_SECTOR_6_BANK1) && (Address >= ADDR_FLASH_SECTOR_5_BANK1))
  {
    sector = FLASH_SECTOR_5;
  }
  else if((Address < ADDR_FLASH_SECTOR_7_BANK1) && (Address >= ADDR_FLASH_SECTOR_6_BANK1))
  {
    sector = FLASH_SECTOR_6;
  }
  else if((Address < ADDR_FLASH_SECTOR_0_BANK2) && (Address >= ADDR_FLASH_SECTOR_7_BANK1))
  {
    sector = FLASH_SECTOR_7;
  }
  else if((Address < ADDR_FLASH_SECTOR_1_BANK2) && (Address >= ADDR_FLASH_SECTOR_0_BANK2))
  {
    sector = FLASH_SECTOR_0;
  }
  else if((Address < ADDR_FLASH_SECTOR_2_BANK2) && (Address >= ADDR_FLASH_SECTOR_1_BANK2))
  {
    sector = FLASH_SECTOR_1;
  }
  else if((Address < ADDR_FLASH_SECTOR_3_BANK2) && (Address >= ADDR_FLASH_SECTOR_2_BANK2))
  {
    sector = FLASH_SECTOR_2;
  }
  else if((Address < ADDR_FLASH_SECTOR_4_BANK2) && (Address >= ADDR_FLASH_SECTOR_3_BANK2))
  {
    sector = FLASH_SECTOR_3;
  }
  else if((Address < ADDR_FLASH_SECTOR_5_BANK2) && (Address >= ADDR_FLASH_SECTOR_4_BANK2))
  {
    sector = FLASH_SECTOR_4;
  }
  else if((Address < ADDR_FLASH_SECTOR_6_BANK2) && (Address >= ADDR_FLASH_SECTOR_5_BANK2))
  {
    sector = FLASH_SECTOR_5;
  }
  else if((Address < ADDR_FLASH_SECTOR_7_BANK2) && (Address >= ADDR_FLASH_SECTOR_6_BANK2))
  {
    sector = FLASH_SECTOR_6;
  }
  else /*if((Address < USER_FLASH_END_ADDRESS) && (Address >= ADDR_FLASH_SECTOR_7_BANK2))*/
  {
    sector = FLASH_SECTOR_7;
  }

  return sector;
}

/**
  * @brief  Configure the write protection status of user flash area.
  * @param  modifier DISABLE or ENABLE the protection
  * @retval HAL_StatusTypeDef HAL_OK if change is applied.
  */
HAL_StatusTypeDef FLASH_If_WriteProtectionConfig(uint32_t modifier)
{
  uint32_t ProtectedSECTOR = 0xFFF;
  FLASH_OBProgramInitTypeDef config_new, config_old;
  HAL_StatusTypeDef result = HAL_OK;

  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  /* Unlock the Options Bytes *************************************************/
  HAL_FLASH_OB_Unlock();
  if (FLASH_BANK2_ADDRESS < ADDR_FLASH_SECTOR_0_BANK2)
  {
    /* Select Bank1 */
    config_old.Banks = FLASH_BANK_1;
    config_new.Banks = FLASH_BANK_1;

    /* Get pages write protection status ****************************************/
    HAL_FLASHEx_OBGetConfig(&config_old);

    /* The parameter says whether we turn the protection on or off */
    config_new.WRPState = modifier;

    /* We want to modify only the Write protection */
    config_new.OptionType = OPTIONBYTE_WRP;

    /* No read protection, keep BOR and reset settings */
    config_new.RDPLevel = OB_RDP_LEVEL_0;
    config_new.USERConfig = config_old.USERConfig;
    /* Get pages already write protected ****************************************/
    ProtectedSECTOR = config_old.WRPSector | FLASH_SECTOR_TO_BE_PROTECTED;

    config_new.WRPSector    = ProtectedSECTOR;
    result = HAL_FLASHEx_OBProgram(&config_new);

  }
  else
  {
    /* Select Bank2 */
    config_old.Banks = FLASH_BANK_2;
    config_new.Banks = FLASH_BANK_2;

    /* Get pages write protection status ****************************************/
    HAL_FLASHEx_OBGetConfig(&config_old);

    /* The parameter says whether we turn the protection on or off */
    config_new.WRPState = modifier;

    /* We want to modify only the Write protection */
    config_new.OptionType = OPTIONBYTE_WRP;

    /* No read protection, keep BOR and reset settings */
    config_new.RDPLevel = OB_RDP_LEVEL_0;
    config_new.USERConfig = config_old.USERConfig;
    /* Get pages already write protected ****************************************/
    ProtectedSECTOR = config_old.WRPSector | FLASH_SECTOR_TO_BE_PROTECTED;
    config_new.WRPSector    = ProtectedSECTOR;
    result = HAL_FLASHEx_OBProgram(&config_new);

  }
   /* Launch the option byte loading */
  HAL_FLASH_OB_Launch();

  HAL_FLASH_OB_Lock();

  HAL_FLASH_Lock();

  return result;
}

/**
  * @}
  */
