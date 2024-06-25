#ifndef IAP_STATEMACHINE_H_
#define IAP_STATEMACHINE_H_


#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"

typedef enum {
    IAP_STATE_Idle,
    IAP_STATE_Menu,
    IAP_STATE_Loaded,
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
} IAP_StateMachine;

extern IAP_StateMachine iap;
bool IAPinit();
bool IAPrequestState(IAP_State state);
bool IAPrun();
bool IAP_TCP_request(char* req, size_t size);

#endif // IAP_STATEMACHINE_H_
