/*
 * udp.h
 *
 *  Created on: Jan 15, 2025
 *      Author: schroeder
 */

#ifndef UDP_UDP_H_
#define UDP_UDP_H_

#include "stdbool.h"
#include "ip_addr.h"

#define UDP_MESSAGE_SIZE 1024

void udp_create_instance();
bool udp_send_message(const char* message);

#endif /* UDP_UDP_H_ */
