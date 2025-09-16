#include "bsp_usart.h"

// 建议每个串口各自有一个发送完成标志
static volatile uint8_t usart1_tx_dma_done = 1;
static volatile uint8_t usart2_tx_dma_done = 1;
// ... 你有几个串口就定义几个

uint8_t usart1_rx_buf[256];//串口1接收数据
uint8_t usart2_rx_buf[256];//串口2接收数据
//...串口...同理

/**
 * @brief  通用USART DMA发送函数，支持多串口
 * @param  huart  要发送的串口句柄指针（如 &huart1）
 * @param  data   发送数据缓冲区指针
 * @param  len    发送数据长度
 * @retval 1 启动成功，0 启动失败或上一次还未发送完成
 */
uint8_t usart_tx_dma_send(UART_HandleTypeDef *huart, uint8_t *data, uint16_t len)
{
    volatile uint8_t *tx_done = NULL;
    if(huart->Instance == USART1)
        tx_done = &usart1_tx_dma_done;
    else if(huart->Instance == USART2)
        tx_done = &usart2_tx_dma_done;
    // ... 其它串口同理

    if(tx_done && *tx_done)
    {
        *tx_done = 0;
        if(HAL_UART_Transmit_DMA(huart, data, len) == HAL_OK)
            return 1; // 启动成功
        else
        {
            *tx_done = 1; // 启动失败，恢复标志
            return 0;
        }
    }
    return 0; // 上一次还没发完
}

/**
 * @brief  串口DMA发送完成中断回调函数（由HAL库自动调用）
 * @param  huart  触发回调的串口句柄指针
 * @note   该函数在每次DMA发送完成后自动被HAL库调用，用于设置发送完成标志，
 *         以便允许下一次DMA发送。支持多串口。
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
        usart1_tx_dma_done = 1;
    else if(huart->Instance == USART2)
        usart2_tx_dma_done = 1;
    // ... 其它串口同理
}

/**
 * @brief  启动串口DMA接收（带空闲中断，适合变长数据）
 * @param  huart  串口句柄指针（如 &huart1）
 * @param  buf    接收缓冲区指针
 * @param  len    接收缓冲区长度
 * @retval 1 启动成功，0 启动失败
 * @note   使用 HAL_UARTEx_ReceiveToIdle_DMA 启动DMA接收，接收完成或遇到空闲中断时会自动触发回调。
 */
uint8_t usart_rx_dma_start(UART_HandleTypeDef *huart, uint8_t *buf, uint16_t len)
{
    if(HAL_UARTEx_ReceiveToIdle_DMA(huart, buf, len) == HAL_OK)
        return 1;
    else
        return 0;
}

/**
 * @brief  串口DMA接收完成（空闲中断）回调函数（由HAL库自动调用）
 * @param  huart  触发回调的串口句柄指针
 * @param  Size   实际接收到的数据长度
 * @note   该函数在每次DMA接收遇到空闲中断或接收满时自动被HAL库调用。
 *         在这里可以处理接收到的数据，并重新启动DMA接收，保证持续接收。
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    // 用户自定义数据处理
    extern void usart_rx_data_process(UART_HandleTypeDef *huart, uint8_t *buf, uint16_t len);
    if(huart->Instance == USART1)
    {
        usart_rx_data_process(huart, usart1_rx_buf, Size);
        usart_rx_dma_start(huart, usart1_rx_buf, sizeof(usart1_rx_buf));
    }
    else if(huart->Instance == USART2)
    {
        usart_rx_data_process(huart, usart2_rx_buf, Size);
        usart_rx_dma_start(huart, usart2_rx_buf, sizeof(usart2_rx_buf));
    }
    // ... 其它串口同理
}

/**
 * @brief  用户自定义的串口DMA接收数据处理函数（弱定义，可被用户重写）
 * @param  huart  当前接收数据的串口句柄指针
 * @param  buf    接收到的数据缓冲区指针
 * @param  len    实际接收到的数据长度
 * @note   用户可在此函数中实现协议解析、数据拷贝、消息通知等功能。
 *         该函数为弱定义（__weak），用户可在其它文件中重新实现以覆盖默认实现。
 *         下面的 HAL_UART_Transmit_DMA(huart, buf, len) 是回环测试用的，可以删除。
 */
__weak void usart_rx_data_process(UART_HandleTypeDef *huart, uint8_t *buf, uint16_t len)
{
    // 用户自己实现，比如解析协议、拷贝数据等
    // 例：printf("收到%d字节\r\n", len);
    HAL_UART_Transmit_DMA(huart, buf, len);//自己加了一个回传用来测试，可以删除
}



