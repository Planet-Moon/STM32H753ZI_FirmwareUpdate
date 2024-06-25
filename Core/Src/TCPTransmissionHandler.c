/*
 * TCPTransmissionHandler.c
 *
 *  Created on: Jun 24, 2024
 *      Author: schroeder
 */

#include "TCPTransmissionHandler.h"

const char csetNumbers[] = "1234567890";

void TCP_PackageHandler_init(TCPTransmissionHandler* tcpHandler, char (*buffer)[TCP_BUFFER_SIZE]){
    tcpHandler->incoming_bytes = 0;
    tcpHandler->received_byted = 0;
    tcpHandler->buffer = buffer;
}
