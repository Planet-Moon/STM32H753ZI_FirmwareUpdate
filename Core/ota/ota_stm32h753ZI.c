/*
 * ota_stm32h753ZI.c
 *
 *  Created on: Jan 13, 2025
 *      Author: schroeder
 */

#include "ota_stm32h753ZI.h"
#include "stdint.h"
#include "stm32h7xx_hal.h"
#include "flash_if.h"
#include "flash.h"
#include "ota.h"
#include "data_buffer.h"
#include "../udp/udp.h"

#if defined(__GNUC__)
#define IRAM __attribute__((noinline, section(".iram"))) // place functions in instruction ram memory (IRAM bool myFunc() {...})
#else
#define IRAM
#endif

#if defined(__GNUC__)
#define ARM_DISABLE_IRQ() asm volatile("cpsid i" : : : "memory")
#define ARM_ENABLE_IRQ() asm volatile("cpsie i" : : : "memory")
#else
#define ARM_DISABLE_IRQ()
#define ARM_ENABLE_IRQ()
#endif

// Firmware version
#define VERSION_MEMORY_SIZE 32
#define NEW_VERSION_ADDRESS 0x081FFF7F
#define MY_VERSION_ADDRESS  0x080FFF7F
#if defined ( __GNUC__ ) /* GNU Compiler */
// __attribute__((section(".debug_section"), used)) const char version_string2[3] = "AB";
// __attribute__((section(".version_section"), used)) const char version_string[VERSION_MEMORY_SIZE] = "Version 1.0.0";
#endif

bool stm32h7_write(void*, const void*, size_t);
bool stm32h7_swap(void);
bool stm32h7_write_begin(void);
bool stm32h7_write_end(void);

bool new_fw_loaded = false;

FlashItf flash_stm32h7 = {
        (void*) 0x08000000, // start
        0,                  // Size, FLASH_SIZE_REG
        128 * 1024,         // Sector size, 128k
        32,                 // Align, 256 bit
        stm32h7_write_begin,
        stm32h7_write_end,
        stm32h7_write,
        stm32h7_swap
};

// DataBuffer flashBuffer;

IRAM void ota_flash_wait(uint8_t bank){
    if(bank == FLASH_BANK_1){
        while(FLASH->SR1 & (1<<0 | 1<<2)) __NOP();
    }
    else if(bank == FLASH_BANK_2){
        while(FLASH->SR2 & (1<<0 | 1<<2)) __NOP();
    }
}

IRAM void ota_flash_clear_err(uint8_t bank){
    ota_flash_wait(bank);
    if(bank == FLASH_BANK_1){
        FLASH->CCR1 = 0x0FEF0000;
    }
    else if(bank == FLASH_BANK_2){
        FLASH->CCR2 = 0x0FEF0000;
    }
}

IRAM void flashUnlock() {
    HAL_FLASHEx_Unlock_Bank1();
    HAL_FLASHEx_Unlock_Bank2();
}

IRAM void flashLock() {
    HAL_FLASHEx_Lock_Bank1();
    HAL_FLASHEx_Lock_Bank2();
}

IRAM bool stm32h7_write_begin() {
    bool ok = true;
    flashUnlock();
    if(ok){
        uint32_t eraseResult = FLASH_If_EraseBank2();
        ok = eraseResult == 0;
    }
    return ok;
}

IRAM bool stm32h7_write_end() {
    flashLock();
    return true;
}

IRAM void flash_wait_bank1() {
    while(FLASH->SR1 & (1<<0 | 1<<2)) __NOP();
}

IRAM void flash_wait_bank2() {
    while(FLASH->SR2 & (1<<0 | 1<<2)) __NOP();
}

IRAM bool stm32h7_write(void* addr, const void* buf, size_t len) {
    if((len % flash_stm32h7.align) != 0){
        // not aligned
        return false;
    }
    bool ok = true;
    char* dst = (char*)addr;
    char* src = (char*)buf;
    char* end = (char*)buf+len;
    const uint32_t TypeProgram = FLASH_TYPEPROGRAM_FLASHWORD;
    ARM_DISABLE_IRQ();
    while(ok && src < end){
        flash_wait_bank1();
        flash_wait_bank2();
        ota_flash_clear_err(FLASH_BANK_1);
        ota_flash_clear_err(FLASH_BANK_2);
        HAL_StatusTypeDef halStatus = HAL_FLASH_Program(
                TypeProgram, (uint32_t)dst, (uint32_t)src);
        ok = halStatus == HAL_OK;
        src += flash_stm32h7.align;
        dst += flash_stm32h7.align;
    }
    ARM_ENABLE_IRQ();
    return ok;
}

bool ota_flash_check_empty() {
    // check if flash is clear
    uint32_t* flash_bank2_base = (uint32_t*)FLASH_BANK2_BASE;
    uint32_t flash_bank2_base_val = *flash_bank2_base;
    return flash_bank2_base_val == 0xFFFFFFFF;
}

IRAM bool stm32h7_swap(void) {
    /*
     * See ST RM0433 Rev 9 Page 171:
     *
     * The SWAP_BANK bit in FLASH_OPTCR register is loaded from the SWAP_BANK_OPT option bit only after system reset or POR
     *
     * Sequence:
     * 1. Unlock OPTLOCK bit, if not already unlocked
     *
     * 2. Set the new desired SWAP_BANK_OPT value in the FLASH_OPTSR_PRG register
     *
     * 3. Start the option byte change sequence by setting the OPTSTART bit in the FLASH_OPTCR register
     *
     * 4. Once the option byte change has completed, FLASH_OPTSR_CUR contains the expected SWAP_BANK_OPT value, but SWAP_BANK bit in FLASH_OPTCR has not yet been modified and the bank swapping is not yet effective
     *
     * 5. Force a system reset or a POR. When the reset rises up, the bank swapping is effective (SWAP_BANK value updated in FLASH_OPTCR) and the new firmware shall be executed
     */

    // if no update was loaded -> don't swap
    if(ota_flash_check_empty()) return false;

    // step 1:
    if(HAL_FLASHEx_Unlock_Bank1() != HAL_OK) return false;
    if(HAL_FLASHEx_Unlock_Bank2() != HAL_OK) return false;
    HAL_FLASH_OB_Unlock();

    // clear errors
    ota_flash_clear_err(FLASH_BANK_1); // mongoose
    ota_flash_clear_err(FLASH_BANK_2); // mongoose

    // unlock OPTLOCK
    bool OPTLOCK = READ_BIT(FLASH->OPTCR, FLASH_OPTCR_OPTLOCK_Msk);
    if(OPTLOCK){
        FLASH->OPTKEYR = 0x08192A3B;
        FLASH->OPTKEYR = 0x4C5D6E7F;
        OPTLOCK = READ_BIT(FLASH->OPTCR, FLASH_OPTCR_OPTLOCK_Msk);
    }
    if(OPTLOCK) return false;

    // step 2:
    bool SWAP_BANK_CURR = READ_BIT(FLASH->OPTSR_PRG, FLASH_OPTSR_SWAP_BANK_OPT_Msk);
    if(SWAP_BANK_CURR){
        CLEAR_BIT(FLASH->OPTSR_PRG, FLASH_OPTSR_SWAP_BANK_OPT_Msk);
    }
    else{
        SET_BIT(FLASH->OPTSR_PRG, FLASH_OPTSR_SWAP_BANK_OPT_Msk);
    }

    // step 3:
    SET_BIT(FLASH->OPTCR, FLASH_OPTCR_OPTSTART_Msk);
    while(FLASH->OPTSR_CUR & FLASH_OPTSR_SWAP_BANK_OPT_Msk) __NOP(); // wait for value to be set in OPTSR_CUR register

    // step 4:
    // HAL_NVIC_SystemReset();

    // lock banks again
    HAL_FLASHEx_Lock_Bank1();
    HAL_FLASHEx_Lock_Bank2();
    SET_BIT(FLASH->OPTCR, FLASH_OPTCR_OPTLOCK_Msk);
    return true;
}

size_t flash_size() {
    static uint32_t* reg = (uint32_t*)0x1FF1E880; // register with flash size in kByte
    return *reg * 1024; // convert to byte
}

bool ota_begin(size_t new_fw_size) {
    flash_stm32h7.size = flash_size();
    // DataBufferClear(&flashBuffer);
    bool ok = ota_flash_begin(new_fw_size, &flash_stm32h7);
    if(ok){
        stm32h7_write_begin();
    }
    return ok;
}

bool ota_write(const void* buf, size_t len){
    /*
    size_t ptrLen = len;
    const char* ptr = (char*)buf;
    while(ptrLen > 0){
        int32_t availableBuffer = DataBufferAvailable(&flashBuffer);
        size_t writeToBufferLen = availableBuffer < ptrLen ? availableBuffer : ptrLen;
        availableBuffer = DataBufferAppend(&flashBuffer, ptr, writeToBufferLen);
        ptr += writeToBufferLen;
        ptrLen -= writeToBufferLen;
        if(availableBuffer == 0){
            bool res = ota_flash_write(flashBuffer.buf, DATABUFFER_SIZE, &flash_stm32h7);
            if(!res){
                return false;
            }
        }
        else if(availableBuffer < 0) {
            return false;
        }
    }
    return true;
    */
    return ota_flash_write(buf, len, &flash_stm32h7);
}

bool ota_end(void) {
    stm32h7_write_end();
    if(ota_flash_end(&flash_stm32h7)){
        new_fw_loaded = true;
    }
    return false;
}

bool BankSwapBit(void) {
    return READ_BIT(FLASH->OPTCR, FLASH_OPTCR_SWAP_BANK_Msk);
}
