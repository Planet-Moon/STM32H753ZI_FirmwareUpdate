/*
  ***************************************************************************************************************
  ***************************************************************************************************************
  ***************************************************************************************************************

  File:		  tcpServerRAW.h
  Author:     ControllersTech.com
  Updated:    26-Jul-2021

  ***************************************************************************************************************
  Copyright (C) 2017 ControllersTech.com

  This is a free software under the GNU license, you can redistribute it and/or modify it under the terms
  of the GNU General Public License version 3 as published by the Free Software Foundation.
  This software library is shared with public for educational purposes, without WARRANTY and Author is not liable for any damages caused directly
  or indirectly by this software, read more about this on the GNU General Public License.

  ***************************************************************************************************************
*/


#ifndef INC_TCPSERVERRAW_H_
#define INC_TCPSERVERRAW_H_
#include "../ota/IAP_StateMachine.h"

#define TCP_BUFFER_SIZE 8*1024 // 8 kByte, see: https://www.ibm.com/docs/en/was-nd/8.5.5?topic=environment-tuning-tcpip-buffer-sizes

void tcp_server_init(void);

#endif /* INC_TCPSERVERRAW_H_ */
