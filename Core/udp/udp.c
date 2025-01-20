/*
 * udp.c
 *
 *  Created on: Jan 15, 2025
 *      Author: schroeder
 */

#include "lwip/udp.h"
#include "udp.h"
#include "string.h"
#include "stddef.h"

#define UDP_MESSAGE_SIZE 1024

ip_addr_t udp_target_ip;
u16_t udp_target_port = 55151;

static struct udp_pcb* my_udp = NULL;
static struct pbuf* udp_buffer = NULL;
static char udp_message[UDP_MESSAGE_SIZE] = "Hello UDP message!\n\r";

static bool udp_send_buffer() {
    udp_buffer = pbuf_alloc(PBUF_TRANSPORT, strlen(udp_message), PBUF_RAM);
    if (udp_buffer != NULL) {
        memcpy(udp_buffer->payload, udp_message, strlen(udp_message));
        udp_send(my_udp, udp_buffer);
        pbuf_free(udp_buffer);
        return true;
    }
    else {
        return false;
    }
}

void udp_create_instance() {
    IP_ADDR4(&udp_target_ip, 192, 168, 0, 1);
    my_udp = udp_new();
    udp_connect(my_udp, &udp_target_ip, udp_target_port);
    udp_send_buffer();
}

bool udp_send_message(const char* message) {
    if(strlen(message) > UDP_MESSAGE_SIZE){
        return false;
    }
    strcpy(udp_message, message);
    return udp_send_buffer();
}
