/*
 * IAP_StateMachine.c
 *
 *  Created on: Jun 14, 2024
 *      Author: schroeder
 */


#include "IAP_StateMachine.h"
#include "main.h"
#include "FLASH_SECTOR_H7.h"
#include "string.h"
#include "stdio.h"
#include "flash_if.h"
#include "STM32H7xx_IT.h"

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
const char* new_version() {
    const char* ptr = (const char*)NEW_VERSION_ADDRESS;
    return ptr;
}
#if defined ( __GNUC__ ) /* GNU Compiler */
__attribute__((section(".version_section"), used)) const char version_string[VERSION_MEMORY_SIZE] = "Version 1.0.0";
#endif

// utils



// length = strlen(data)
 int numofwords(size_t length) {
     // bytes/4 = word count
     // 1 word has 4 bytes
     return (length/4)+((length%4)!=0);
 }

static size_t flash_size = 0;

IRAM void flash_wait(uint8_t bank){
    if(bank == FLASH_BANK_1){
        while(FLASH->SR1 & (1<<0 | 1<<2)) __NOP();
    }
    else if(bank == FLASH_BANK_2){
        while(FLASH->SR2 & (1<<0 | 1<<2)) __NOP();
    }
}

IRAM void flash_clear_err(uint8_t bank){
    flash_wait(bank);
    if(bank == FLASH_BANK_1){
        FLASH->CCR1 = 0x0FEF0000;
    }
    else if(bank == FLASH_BANK_2){
        FLASH->CCR2 = 0x0FEF0000;
    }
}

size_t _flash_size() {
    static uint32_t* reg = (uint32_t*)0x1FF1E880; // register with flash size in kByte
    return *reg * 1024; // convert to byte
}

bool _flash_begin() {
    flash_size = _flash_size();
    return true;
}


uint8_t flash_bank(void* addr) {
    size_t ofs = (char*)addr-(char*)FLASH_BANK1_BASE;
    return ofs < (flash_size>>1) ? FLASH_BANK_1 : FLASH_BANK_2;
}

IAP_StateMachine iap;

void _BufferClear();
void _FlashBank2Clear();

bool _flashCheckEmpty() {
    // check if flash is clear
    uint32_t* flash_bank2_base = (uint32_t*)FLASH_BANK2_BASE;
    uint32_t flash_bank2_base_val = *flash_bank2_base;
    return flash_bank2_base_val == 0xFFFFFFFF;
}

bool IAP_Init(){
    iap.bytesWrittenToFlash = 0;
    iap.FlashProtection = 0;
    _BufferClear();
    _flash_begin();
    return true;
}

bool IAP_TCP_request(char* req, size_t size) {
    if(strncmp(req, "1", (uint8_t)1) == 0){
        // example for reading some flash address
    }

    else if(strncmp(req, "4", (uint8_t)1) == 0){
        // read write protection
        //uint32_t writeProtection = FlashGetWriteProtectionStatus();
        //char str[10];
        //sprintf(str, "wp: %lu", writeProtection);
        //strcpy(req,str);
        strcat(req,"\nOK\n");
        return true;
    }

    else if(strncmp(req, "5", (uint8_t)1) == 0){
        // uint16_t wordcount = numofwords(IAP_BINARY_FILE_SIZE);
        return true;
    }

    else{
        // unrecognized command
        strcpy(req,"IAP undefined command!");
        return false;
    }

    OkResult:
    strcpy(req,"OK\n");
    return true;

    NotInMenu: // not in menu jump label
    strcpy(req, "IAP not in menu\n");
    return false;
}

int16_t _BufferAvailable() {
    return IAP_FLASH_BUFFER_SIZE - iap.flashBufferEndIdx;
}

void IAP_FlashWriteInit() {
    _BufferClear();
    _FlashBank2Clear();
    iap.bytesWrittenToFlash = 0;
}

void _BufferClear() {
    iap.flashBufferEndIdx = 0;
    memset(iap.flashBuffer, 0, IAP_FLASH_BUFFER_SIZE);
}

int16_t _BufferAppend(const char* data, size_t len) {
    int16_t remaining_space = _BufferAvailable();
    if(remaining_space < len){
        memcpy(iap.flashBuffer + iap.flashBufferEndIdx, data, remaining_space);
        iap.flashBufferEndIdx = IAP_FLASH_BUFFER_SIZE;
        return remaining_space - len;
    }
    else {
        memcpy(iap.flashBuffer + iap.flashBufferEndIdx, data, len);
        iap.flashBufferEndIdx += len;
        return _BufferAvailable();
    }
}

void _FlashBank2Clear() {
    FlashErrors flashErrors = FLASH_If_Errors();
    uint32_t eraseResult = FLASH_If_EraseBank2();
    if(eraseResult){
        iap.error = 1;
        return;
    }
    iap.writeFlashAddress = FLASH_BANK2_ADDRESS;
}

bool _FlashBufferWrite() {
    // if buffer is full (8*32bit) write it to flash (or it's the last transmission)
    const uint32_t TypeProgram = FLASH_TYPEPROGRAM_FLASHWORD;
    const uint32_t FlashAddress = iap.writeFlashAddress;
    const uint32_t DataAddress = (uint32_t)iap.flashBuffer;
    flash_wait(flash_bank((void*)iap.writeFlashAddress));
    ARM_DISABLE_IRQ();
    HAL_StatusTypeDef halStatus = HAL_FLASH_Program(TypeProgram, FlashAddress, DataAddress);
    ARM_ENABLE_IRQ();
    if(halStatus != HAL_OK){
       return false;
    }
    _BufferClear();
    iap.writeFlashAddress += IAP_FLASH_BUFFER_SIZE; // bytes written to flash
    iap.bytesWrittenToFlash += IAP_FLASH_BUFFER_SIZE;
    return true;
}

bool IAP_FlashWrite(const void* data, size_t len) {
    // put data in buffer
    size_t payload_len = len;
    const void* payload = data;
    int16_t available_buffer = _BufferAvailable();
    // payload_len > available_buffer

    while(payload_len > 0){
        available_buffer = _BufferAvailable();
        int16_t buffer_bytes = payload_len;
        if(available_buffer < payload_len){
            buffer_bytes = available_buffer;
        }
        available_buffer = _BufferAppend(payload, buffer_bytes);
        payload += buffer_bytes;
        payload_len -= buffer_bytes;
        if(available_buffer == 0){
            bool res = _FlashBufferWrite();
            if(!res){
                return false;
            }
        }
        else if(available_buffer < 0){
            return false;
        }
    }

    return true;
}

void IAP_FlashWriteRemaining() {
    _FlashBufferWrite();

    FLASH_CRCInitTypeDef CRCInit;
    CRCInit.TypeCRC = FLASH_CRC_BANK;
    CRCInit.BurstSize = FLASH_CRC_BURST_SIZE_4;
    // CRCInit.Bank = FLASH_BANK_2;
    // * only if type is sector
    // * CRCInit.Sector = FLASH_SECTOR_0;
    // * CRCInit.NbSectors = 8;
    // + only if type is by address
    CRCInit.CRCStartAddr = FLASH_BANK2_BASE;
    CRCInit.CRCEndAddr = FLASH_BANK2_BASE + iap.writeFlashAddress;
    uint32_t CRC_Result = 0;
    HAL_StatusTypeDef halStatus = HAL_FLASHEx_ComputeCRC(&CRCInit, &CRC_Result);

    const char newVersion[VERSION_MEMORY_SIZE];
    memcpy(newVersion, new_version(), VERSION_MEMORY_SIZE);

}

bool IAP_UpdatePrepared() {
    return !_flashCheckEmpty();
}

void IAP_FlashBank2Clear() {
    _FlashBank2Clear();
}

void bankSwap() {
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
    if(!IAP_UpdatePrepared()) return;

    // step 1:
    if(HAL_FLASHEx_Unlock_Bank1() != HAL_OK) return;
    if(HAL_FLASHEx_Unlock_Bank2() != HAL_OK) return;
    HAL_FLASH_OB_Unlock();

    // clear errors
    flash_clear_err(FLASH_BANK_1); // mongoose
    flash_clear_err(FLASH_BANK_2); // mongoose

    // unlock OPTLOCK
    bool OPTLOCK = READ_BIT(FLASH->OPTCR, FLASH_OPTCR_OPTLOCK_Msk);
    if(OPTLOCK){
        FLASH->OPTKEYR = 0x08192A3B;
        FLASH->OPTKEYR = 0x4C5D6E7F;
        OPTLOCK = READ_BIT(FLASH->OPTCR, FLASH_OPTCR_OPTLOCK_Msk);
    }
    if(OPTLOCK) return;

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
    HAL_NVIC_SystemReset();

    // lock banks again
    HAL_FLASHEx_Lock_Bank1();
    HAL_FLASHEx_Lock_Bank2();
    SET_BIT(FLASH->OPTCR, FLASH_OPTCR_OPTLOCK_Msk);
    return;
}

bool BankSwapBit() {
    return READ_BIT(FLASH->OPTCR, FLASH_OPTCR_SWAP_BANK_Msk);
}

bool BankSwapNextReset() {
    return READ_BIT(FLASH->OPTSR_PRG, FLASH_OPTSR_SWAP_BANK_OPT_Msk) != BankSwapBit() && READ_BIT(FLASH->OPTCR, FLASH_OPTCR_OPTSTART_Msk);
}

void test_OBGetConfig() {
    FLASH_OBProgramInitTypeDef OBInit;
    HAL_FLASHEx_OBGetConfig(&OBInit);
    return;
}

bool compareFlashBankData() {
    static const uint32_t offset = FLASH_BANK2_BASE - FLASH_BANK1_BASE;
    for(uint32_t addr = FLASH_BANK1_BASE; addr < FLASH_BANK2_BASE; ++addr){
        uint32_t* p1 = (uint32_t*)addr;
        uint32_t* p2 = (uint32_t*)(addr + offset);
        uint32_t v1 = *p1;
        uint32_t v2 = *p2;
        if(v1 != v2) {
            return false;
        }
    }
    return true;
}

bool copyFlashFromBank1ToBank2(){
    // check if flash is clear
    uint32_t* flash_bank2_base = (uint32_t*)FLASH_BANK2_BASE;
    uint32_t flash_bank2_base_val = *flash_bank2_base;
    if(flash_bank2_base_val != 0xFFFFFFFF)
        return true; // flash was already written to

    IAP_FlashWriteInit();
    IAP_FlashWrite((void*)FLASH_BANK1_BASE, 1024*1024);
    IAP_FlashWriteRemaining();
    return true;
}
