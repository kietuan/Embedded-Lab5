/*
 * uart.c
 *
 *  Created on: Sep 26, 2023
 *      Author: HaHuyen
 */
#include "uart.h"
#include "lcd.h"
#include "clock_fsm.h"
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
uint8_t receive_buffer1 = 0;
uint8_t hour, min, second;
uint8_t msg[100];

void uart_init_rs232(){
	HAL_UART_Receive_IT(&huart1, &receive_buffer1, 1);
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
        switch (uartState)
        {
        case NOTHING:
            if ((0 <= receive_buffer1 - '0') && (receive_buffer1 - '0' <=9))
            {
                uart_num1 = receive_buffer1 - '0';
                uartState = NUM1;
            }
            else
            {
                uart_num1 = 0;
                uart_num2 = 0;
                Operator = NO;
                uartState = NOTHING;
            }
            break;
        case NUM1:
            /*if ((0 <= receive_buffer1 - '0') && (receive_buffer1 - '0' <=9))
            {
                NUM1 = NUM1*10 + receive_buffer1 - '0';
                uartState = NUM1;
            }
            else */if (receive_buffer1 == '+')
            {
                Operator = ADD;
                uartState = NUM2;
            }
            else if (receive_buffer1 == '-')
            {
                Operator = SUB;
                uartState = NUM2;
            }
            else if (receive_buffer1 == '*')
            {
                Operator = MUL;
                uartState = NUM2;
            }
            else
            {
                uart_num1 = 0;
                uart_num2 = 0;
                Operator = NO;
                uartState = NOTHING;
            }
            break;
        case NUM2:
            if ((0 <= receive_buffer1 - '0') && (receive_buffer1 - '0' <=9))
            {
                uart_num2 = receive_buffer1 - '0';
                switch (Operator)
                {
                case ADD:
                    uart_result = uart_num1 + uart_num2;
                    resultDone = 1;
                    break;
                case SUB:
                    uart_result = uart_num1 - uart_num2;
                    resultDone = 1;
                    break;
                case MUL:
                    uart_result = uart_num1 * uart_num2;
                    resultDone = 1;
                    break;
                default:
                    break;
                }
                Operator = NO;
            }
            else
            {
                uart_num1 = 0;
                uart_num2 = 0;
                Operator = NO;
                // uartState = NOTHING
            }
            uartState = NOTHING;
            break;

        default:
            uartState = NOTHING;
            break;
        }

		// turn on the receice interrupt
		HAL_UART_Receive_IT(&huart1, &receive_buffer1, 1);
//		lcd_StrCenter(0, 2, &receive_buffer1, RED, BLUE, 16,
//				 1);

	}
}


