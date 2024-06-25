/*
 * TCPTransmissionHandler.h
 *
 *  Created on: Jun 24, 2024
 *      Author: schroeder
 */

#ifndef INC_TCPTRANSMISSIONHANDLER_H_
#define INC_TCPTRANSMISSIONHANDLER_H_
#include "stdint.h"
#include "tcpServerRAW.h"

typedef struct {
    uint32_t incoming_bytes;
    uint32_t received_byted;
    char (*buffer)[TCP_BUFFER_SIZE];
} TCPTransmissionHandler;

void TCP_PackageHandler_init(TCPTransmissionHandler* tcpHandler, char (*buffer)[TCP_BUFFER_SIZE]);

#endif /* INC_TCPTRANSMISSIONHANDLER_H_ */
