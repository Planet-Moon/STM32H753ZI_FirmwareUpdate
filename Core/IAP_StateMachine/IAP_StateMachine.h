#ifndef IAP_STATEMACHINE_H_
#define IAP_STATEMACHINE_H_


#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"
#include "timemanagement.h"

typedef enum {
    IAP_STATE_Idle,
    IAP_STATE_Menu,
    IAP_STATE_LoadWait,
    IAP_STATE_Load,
    IAP_STATE_None
} IAP_State;
typedef  void (*pFunction)(void);

#define IAP_BINARY_FILE_SIZE 100*1024 // kBytes

typedef struct {
    IAP_State state;
    IAP_State reqState;
    uint32_t FlashProtection;
    pFunction JumpToApplication;
    uint32_t JumpAddress;
    uint32_t userCmd;
    char binaryFileBuffer[IAP_BINARY_FILE_SIZE];
    size_t binaryFileEndIdx;
    Timer loadWaitTimer;
} IAP_StateMachine;

extern IAP_StateMachine iap;
bool IAPinit();
bool IAPrequestState(IAP_State state);
bool IAPrun();
bool IAP_TCP_request(char* req, size_t size);
void IAP_BufferClear();
bool IAP_BufferAppend(const char* data, size_t len);
bool IAP_CopyBufferToFlash();
bool IAP_CopyFlashToBuffer();
bool IAP_LoadApplication();
bool IAP_TryLoadApplication();

#endif // IAP_STATEMACHINE_H_
