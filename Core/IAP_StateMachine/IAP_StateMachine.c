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

// length = strlen(data)
 int numofwords(size_t length) {
     // bytes/4 = word count
     // 1 word has 4 bytes
     return (length/4)+((length%4)!=0);
 }

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

    IAP_BufferClear();
    return true;
}

bool IAPrun() {
    switch(iap.state){
        case IAP_STATE_Idle:
            HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
            break;
        case IAP_STATE_Menu:
            HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
            break;
        case IAP_STATE_LoadWait:
            timer_init(&iap.loadWaitTimer, 200, &TimeSinceStartup64);
            iap.state = IAP_STATE_Load;
            break;
        case IAP_STATE_Load:
            if(timer_run(&iap.loadWaitTimer)){
                IAP_LoadApplication();
                iap.reqState = IAP_STATE_Idle;
            }
            break;
        default:
            iap.state = IAP_STATE_Idle;
    }

    // switch states
    switch(iap.reqState){
        case IAP_STATE_Menu:
            if(iap.state == IAP_STATE_Idle){
                IAPswitchReqToState();
            }
            break;
        case IAP_STATE_Idle:
            IAPswitchReqToState();
            break;
        case IAP_STATE_Load:
            iap.reqState = IAP_STATE_None;
            break;
        case IAP_STATE_LoadWait:
            IAPswitchReqToState();
            break;
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

    if(strncmp(req, "1", (uint8_t)1) == 0){
        // example for reading some flash address
        if(iap.state == IAP_STATE_Menu){
            uint32_t data[10];
            memset(data, 0, 10*sizeof(uint32_t));
            //Flash_Read_Data(0x08000000, data, numofwords(10*sizeof(uint32_t)));
            goto OkResult;
        }
        else goto NotInMenu;
    }

    else if(strncmp(req, "2", (uint8_t)1)==0){
        // load received binary file to buffer
        if(iap.state == IAP_STATE_Menu){
            memcpy(iap.binaryFileBuffer, req+2, size-2);
            goto OkResult;
        }
        else goto NotInMenu;
    }

    else if(strncmp(req, "3", (uint8_t)1) == 0){
        // reset file buffer
        if(iap.state == IAP_STATE_Menu){
            memset(iap.binaryFileBuffer,0,IAP_BINARY_FILE_SIZE);
            goto OkResult;
        }
        else goto NotInMenu;
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
        uint16_t wordcount = numofwords(IAP_BINARY_FILE_SIZE);
        FLASH_If_Write(APPLICATION_ADDRESS, (uint32_t*)iap.binaryFileBuffer, wordcount);
        return true;

        // start application execution
        iap.JumpAddress = *(__IO uint32_t*)(APPLICATION_ADDRESS+4);
        // jump to user application
        iap.JumpToApplication = (pFunction)iap.JumpAddress;
        /* Initialize user application's Stack Pointer */
        __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
        iap.JumpToApplication();
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

void IAP_BufferClear() {
    iap.binaryFileEndIdx = 0;
    memset(iap.binaryFileBuffer, 0, IAP_BINARY_FILE_SIZE);
}

bool IAP_BufferAppend(const char* data, size_t len) {
    size_t remaining_space = IAP_BINARY_FILE_SIZE - iap.binaryFileEndIdx;
    if(remaining_space < len){
        return false;
    }
    memcpy(iap.binaryFileBuffer+iap.binaryFileEndIdx, data, len);
    iap.binaryFileEndIdx += len;
    return true;
}

bool IAP_CopyBufferToFlash() {
    uint32_t wordcount = numofwords(IAP_BINARY_FILE_SIZE);
    bool result = FLASH_If_Write(APPLICATION_ADDRESS, (uint32_t*)iap.binaryFileBuffer, wordcount) == 0;
    return result;
}

bool IAP_CopyFlashToBuffer() {
    for(uint32_t i = 0; i < IAP_BINARY_FILE_SIZE; ++i){
        iap.binaryFileBuffer[i] = *(__IO uint32_t*)(APPLICATION_ADDRESS+i);
    }
    return true;
}

bool IAP_LoadApplication() {
    /* Test if user code is programmed starting from address "APPLICATION_ADDRESS" */
    //uint32_t application_address_value = (*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000;
    //if (application_address_value == 0x20000000) {
        /* Jump to user application */
        iap.JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
        iap.JumpToApplication = (pFunction) iap.JumpAddress;
        /* Initialize user application's Stack Pointer */
        __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
        iap.JumpToApplication();
    //}
    return false; // should not reach here if running correctly
}

bool IAP_TryLoadApplication() {
    /* Test if user code is programmed starting from address "APPLICATION_ADDRESS" */
    if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000 ) == 0x20000000) {
        return IAP_LoadApplication();
    }
    return false;
}
