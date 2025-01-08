#ifndef IAP_STATEMACHINE_H_
#define IAP_STATEMACHINE_H_


#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"
#include "timemanagement.h"


#define IAP_FLASH_BUFFER_SIZE 32 // size of a flash write queue in Bytes (32*8 = 256 bit)

typedef struct {
    uint32_t FlashProtection;
    uint8_t flashBuffer[IAP_FLASH_BUFFER_SIZE]; // 256 bits for address alignment
    size_t flashBufferEndIdx;
    uint32_t writeFlashAddress;
    uint16_t bytesWrittenToFlash;
    uint32_t error;
} IAP_StateMachine;

extern IAP_StateMachine iap;
bool IAPinit();
bool IAP_TCP_request(char* req, size_t size);
void IAP_FlashWriteInit();
bool IAP_FlashWrite(const void* data, size_t len);
void IAP_FlashWriteRemaining();
bool IAP_UpdatePrepared();
void IAP_FlashBank2Clear();

void test_OBGetConfig();
void bankSwap();
bool compareFlashBankData();
bool copyFlashFromBank1ToBank2();
bool BankSwapBit();
bool BankSwapNextReset();

#endif // IAP_STATEMACHINE_H_
