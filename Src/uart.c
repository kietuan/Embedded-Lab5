/*
 * uart.c
 *
 *  Created on: Sep 26, 2023
 *      Author: HaHuyen
 */
#include "uart.h"
#include "lcd.h"
#include "clock_fsm.h"
#define BUFFER_SIZE 255
typedef enum {
    NO,
    ADD, SUB, MUL
} OPERATOR;

UART_state uartState = NOTHING;
OPERATOR Operator = NOTHING;
int8_t uart_num1=0;
int8_t uart_num2=0;
int8_t uart_result=0; 
int8_t resultDone=0;
uint8_t receive_buffer1[255];
uint8_t hour, min, second;
uint8_t msg[100];
uint8_t write_index=0;
uint8_t read_index=0;

void uart_init_rs232(){
	HAL_UART_Receive_IT(&huart1, receive_buffer1, 1);
}

void uart_Rs232SendString(uint8_t* str){
	HAL_UART_Transmit(&huart1, (void*)msg, sprintf((void*)msg,"%s",str), 10);
}

void uart_Rs232SendBytes(uint8_t* bytes, uint16_t size){
	HAL_UART_Transmit(&huart1, bytes, size, 10);
}

void uart_Rs232SendNum(uint32_t num){
	if(num == 0){
		uart_Rs232SendString("0");
		return;
	}
    uint8_t num_flag = 0;
    int i;
	if(num < 0) uart_Rs232SendString("-");
    for(i = 10; i > 0; i--)
    {
        if((num / mypow(10, i-1)) != 0)
        {
            num_flag = 1;
            sprintf((void*)msg,"%d",num/mypow(10, i-1));
            uart_Rs232SendString(msg);
        }
        else
        {
            if(num_flag != 0)
            	uart_Rs232SendString("0");
        }
        num %= mypow(10, i-1);
    }
}

void uart_Rs232SendNumPercent(uint32_t num)
{
	sprintf((void*)msg,"%ld",num/100);
    uart_Rs232SendString(msg);
    uart_Rs232SendString(".");
    sprintf((void*)msg,"%ld",num%100);
    uart_Rs232SendString(msg);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart->Instance == USART1){
		// rs232 isr
		// can be modified
		// HAL_UART_Transmit(&huart1, &receive_buffer1, 1, 10);
        if (write_index != read_index-1)
        {
		    HAL_UART_Receive_IT(&huart1, receive_buffer1 + write_index, 1);
            ++write_index;
        }
	}
}


void process_uart_buffer(void) {
    if (receive_buffer1[write_index] == '#' && read_index != write_index) 
    {
        char temp_buffer[BUFFER_SIZE];
        int temp_index = 0;

        // Read characters from the buffer until '#'
        while (read_index != write_index) {
            temp_buffer[temp_index++] = receive_buffer1[read_index];
            read_index = (read_index + 1) % BUFFER_SIZE;
            if (receive_buffer1[read_index] == '#') {
                break;
            }
        }
        temp_buffer[temp_index] = '\0'; // Null-terminate the string

        // Parse and calculate the expression
        int num1, num2;
        char operator;
        sscanf(temp_buffer, "%d%c%d", &num1, &operator, &num2);

        switch (operator) {
            case '+':
                uart_result = num1 + num2;
                resultDone = 1;
                break;
            case '-':
                uart_result = num1 - num2;
                resultDone = 1;
                break;
            case '*':
                uart_result = num1 * num2;
                resultDone = 1;
                break;
            case '/':
                if (num2 != 0) {
                    uart_result = num1 / num2;
                    resultDone = 1;
                } else {
                    // Handle division by zero if needed
                    resultDone = 0;
                    uart_result = 0;
                }
                break;
            default:
                // Handle unknown operator if needed
                uart_result = 0;
                resultDone = 0;
                break;
        }

        // Reset read_index to the next position after '#'
        read_index = (read_index + 1) % BUFFER_SIZE;
    }
}

