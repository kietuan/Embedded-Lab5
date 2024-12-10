/*
 * uart.h
 *
 *  Created on: Sep 26, 2023
 *      Author: HaHuyen
 */
#ifndef INC_UART_H_
#define INC_UART_H_

#include "usart.h"
#include <stdio.h>
#include "utils.h"
#include <stdlib.h>
#include <stdint.h>

typedef enum {
    NOTHING,
    NUM1, NUM2, RESULT
} UART_state;

extern UART_state uartState;
extern int8_t uart_result;
extern int8_t resultDone;

extern uint8_t write_index, read_index;

void process_uart_buffer(void);

void uart_init_rs232();

void uart_Rs232SendString(uint8_t* str);

void uart_Rs232SendBytes(uint8_t* bytes, uint16_t size);

void uart_Rs232SendNum(uint32_t num);

void uart_Rs232SendNumPercent(uint32_t num);

#endif /* INC_UART_H_ */

