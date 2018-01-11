#ifndef __BTL_USARTX_H__
#define	__BTL_USARTX_H__

/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* ���Ͷ��� ------------------------------------------------------------------*/
/* �궨�� --------------------------------------------------------------------*/
#define GSM_USARTx                                 USART3
#define GSM_USARTx_BAUDRATE                        115200
#define GSM_USART_RCC_CLK_ENABLE()                 __HAL_RCC_USART3_CLK_ENABLE()
#define GSM_USART_RCC_CLK_DISABLE()                __HAL_RCC_USART3_CLK_DISABLE()

#define GSM_USARTx_GPIO_ClK_ENABLE()               __HAL_RCC_GPIOB_CLK_ENABLE()
#define GSM_USARTx_PORT                            GPIOB
#define GSM_USARTx_Tx_PIN                          GPIO_PIN_10
#define GSM_USARTx_Rx_PIN                          GPIO_PIN_11


/* ��չ���� ------------------------------------------------------------------*/
extern UART_HandleTypeDef husartx_GSM;

/* �������� ------------------------------------------------------------------*/
void GSM_USARTx_Init(void);
char *get_rebuff(uint8_t *len); 
void clean_rebuff(void);
void Usart_SendByte(uint8_t ch );
void GSM_USART_printf(USART_TypeDef* USARTx, char *Data,...);

#endif /* __BTL_USARTX_H__ */

/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/
