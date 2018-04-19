/* ========================================================================
 * File Name: bc_uart.h
 * This file is part of the TI-MSP432 LibNbiot example
 *
 * Created on: 02.08.2017
 * Author: Edgar Hindemith
 *
 * Copyright (c) 2018, Edgar Hindemith, Yassine Amraue, Thorsten
 * Krautscheid, Kolja Vornholt, T-Systems International GmbH
 * contact: libnbiot@t-systems.com, opensource@telekom.de
 *
 * This file is distributed under the conditions of the Apache License,
 * Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * For details see the file LICENSE at the toplevel.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either expressed or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ========================================================================
*/

#ifndef BC_UART_H_
#define BC_UART_H_

void startBcUart();
void bcPutChar(unsigned char byte);
void bcDbgWrite(const unsigned char* buf, unsigned short len);

#endif /* BC_UART_H_ */
