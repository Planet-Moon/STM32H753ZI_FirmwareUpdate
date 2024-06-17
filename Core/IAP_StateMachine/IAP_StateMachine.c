/*
 * IAP_StateMachine.c
 *
 *  Created on: Jun 14, 2024
 *      Author: schroeder
 */


#include "IAP_StateMachine.h"
#include "main.h"
#include "flash_if.h"
#include "string.h"

IAP_StateMachine iap;

static void IAPswitchReqToState(){
    iap.state = iap.reqState;
    iap.reqState = IAP_STATE_None;
}

bool IAPinit(){
    iap.state = IAP_STATE_Idle;
    iap.reqState = IAP_STATE_None;
    iap.FlashProtection = 0;
    iap.JumpToApplication = NULL;
    iap.JumpAddress = 0;
    iap.userCmd = 0;
    return true;
}

bool IAPrun() {
    switch(iap.state){
        case IAP_STATE_Idle:
            HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
            break;
        case IAP_STATE_Menu:
            HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
            iap.FlashProtection = FLASH_If_GetWriteProtectionStatus();
            break;
        case IAP_STATE_Loaded:
            break;
        default:
            iap.state = IAP_STATE_Idle;
    }

    // switch states
    switch(iap.reqState){
        case IAP_STATE_Menu:
            if(iap.state == IAP_STATE_Idle){
                // Initialise Flash
                // FLASH_If_Init();
                // Execute the IAP driver in order to reprogram the Flash
                IAPswitchReqToState();
            }
            break;
        case IAP_STATE_Idle:
            IAPswitchReqToState();
            break;
        case IAP_STATE_Loaded:
        default:
            iap.reqState = IAP_STATE_None;
    }
    return true;
}

bool IAPrequestState(IAP_State state) {
    if(iap.reqState != IAP_STATE_None) return false;
    iap.reqState = state;
    return true;
}

bool IAP_TCP_request(char* req, size_t size) {
    if(iap.state == IAP_STATE_Menu){
        if(strncmp(req, "1", (uint8_t)1) == 0){
            uint32_t data = 5;
            if(FLASH_If_Write(0x08000000, &data, 1) == 0){
                req = "OK\n";
                return true;
            }
        }
    }
    memset(req,0,size);
    return false;
}

