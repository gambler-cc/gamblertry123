#include "bsp_usart.h"

// ����ÿ�����ڸ�����һ��������ɱ�־
static volatile uint8_t usart1_tx_dma_done = 1;
static volatile uint8_t usart2_tx_dma_done = 1;
// ... ���м������ھͶ��弸��

uint8_t usart1_rx_buf[256];//����1��������
uint8_t usart2_rx_buf[256];//����2��������
//...����...ͬ��

/**
 * @brief  ͨ��USART DMA���ͺ�����֧�ֶമ��
 * @param  huart  Ҫ���͵Ĵ��ھ��ָ�루�� &huart1��
 * @param  data   �������ݻ�����ָ��
 * @param  len    �������ݳ���
 * @retval 1 �����ɹ���0 ����ʧ�ܻ���һ�λ�δ�������
 */
uint8_t usart_tx_dma_send(UART_HandleTypeDef *huart, uint8_t *data, uint16_t len)
{
    volatile uint8_t *tx_done = NULL;
    if(huart->Instance == USART1)
        tx_done = &usart1_tx_dma_done;
    else if(huart->Instance == USART2)
        tx_done = &usart2_tx_dma_done;
    // ... ��������ͬ��

    if(tx_done && *tx_done)
    {
        *tx_done = 0;
        if(HAL_UART_Transmit_DMA(huart, data, len) == HAL_OK)
            return 1; // �����ɹ�
        else
        {
            *tx_done = 1; // ����ʧ�ܣ��ָ���־
            return 0;
        }
    }
    return 0; // ��һ�λ�û����
}

/**
 * @brief  ����DMA��������жϻص���������HAL���Զ����ã�
 * @param  huart  �����ص��Ĵ��ھ��ָ��
 * @note   �ú�����ÿ��DMA������ɺ��Զ���HAL����ã��������÷�����ɱ�־��
 *         �Ա�������һ��DMA���͡�֧�ֶമ�ڡ�
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
        usart1_tx_dma_done = 1;
    else if(huart->Instance == USART2)
        usart2_tx_dma_done = 1;
    // ... ��������ͬ��
}

/**
 * @brief  ��������DMA���գ��������жϣ��ʺϱ䳤���ݣ�
 * @param  huart  ���ھ��ָ�루�� &huart1��
 * @param  buf    ���ջ�����ָ��
 * @param  len    ���ջ���������
 * @retval 1 �����ɹ���0 ����ʧ��
 * @note   ʹ�� HAL_UARTEx_ReceiveToIdle_DMA ����DMA���գ�������ɻ����������ж�ʱ���Զ������ص���
 */
uint8_t usart_rx_dma_start(UART_HandleTypeDef *huart, uint8_t *buf, uint16_t len)
{
    if(HAL_UARTEx_ReceiveToIdle_DMA(huart, buf, len) == HAL_OK)
        return 1;
    else
        return 0;
}

/**
 * @brief  ����DMA������ɣ������жϣ��ص���������HAL���Զ����ã�
 * @param  huart  �����ص��Ĵ��ھ��ָ��
 * @param  Size   ʵ�ʽ��յ������ݳ���
 * @note   �ú�����ÿ��DMA�������������жϻ������ʱ�Զ���HAL����á�
 *         ��������Դ�����յ������ݣ�����������DMA���գ���֤�������ա�
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    // �û��Զ������ݴ���
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
    // ... ��������ͬ��
}

/**
 * @brief  �û��Զ���Ĵ���DMA�������ݴ������������壬�ɱ��û���д��
 * @param  huart  ��ǰ�������ݵĴ��ھ��ָ��
 * @param  buf    ���յ������ݻ�����ָ��
 * @param  len    ʵ�ʽ��յ������ݳ���
 * @note   �û����ڴ˺�����ʵ��Э����������ݿ�������Ϣ֪ͨ�ȹ��ܡ�
 *         �ú���Ϊ�����壨__weak�����û����������ļ�������ʵ���Ը���Ĭ��ʵ�֡�
 *         ����� HAL_UART_Transmit_DMA(huart, buf, len) �ǻػ������õģ�����ɾ����
 */
__weak void usart_rx_data_process(UART_HandleTypeDef *huart, uint8_t *buf, uint16_t len)
{
    // �û��Լ�ʵ�֣��������Э�顢�������ݵ�
    // ����printf("�յ�%d�ֽ�\r\n", len);
    HAL_UART_Transmit_DMA(huart, buf, len);//�Լ�����һ���ش��������ԣ�����ɾ��
}



