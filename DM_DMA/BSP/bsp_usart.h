#ifndef __BSP_USART_H__
#define __BSP_USART_H__

#include "stm32h7xx.h"
#include "usart.h"

extern uint8_t usart1_rx_buf[256];
extern uint8_t usart2_rx_buf[256];
uint8_t usart_tx_dma_send(UART_HandleTypeDef *huart, uint8_t *data, uint16_t len);
uint8_t usart_rx_dma_start(UART_HandleTypeDef *huart, uint8_t *buf, uint16_t len);
void usart_rx_data_process(UART_HandleTypeDef *huart, uint8_t *buf, uint16_t len);

#endif
