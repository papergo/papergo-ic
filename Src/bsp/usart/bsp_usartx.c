/**
  ******************************************************************************
  * �ļ�����: bsp_usartx.c 
  * ��    ��: ӲʯǶ��ʽ�����Ŷ�
  * ��    ��: V1.0
  * ��д����: 2015-10-04
  * ��    ��: GSM���ڵײ���������
  ******************************************************************************
  * ˵����
  * ����������ӲʯYS-F1Mini��ʹ�á�
  * 
  * �Ա���
  * ��̳��http://www.ing10bbs.com
  * ��Ȩ��ӲʯǶ��ʽ�����Ŷ����У��������á�
  ******************************************************************************
  */

/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "usart/bsp_usartx.h"
#include <stdarg.h>

/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
/* ˽�б��� ------------------------------------------------------------------*/
UART_HandleTypeDef husartx_GSM;
static __IO uint32_t TimingDelay=0;

/* ��չ���� ------------------------------------------------------------------*/
extern uint8_t aRxBuffer;
/* ˽�к���ԭ�� --------------------------------------------------------------*/
/* ������ --------------------------------------------------------------------*/
/**
  * ��������: GSMͨ�Ź�������GPIO��ʼ��
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
void GSM_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* ��������ʱ��ʹ�� */
  GSM_USARTx_GPIO_ClK_ENABLE();

  /* �������蹦��GPIO���� */
  GPIO_InitStruct.Pin = GSM_USARTx_Tx_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GSM_USARTx_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GSM_USARTx_Rx_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GSM_USARTx_PORT, &GPIO_InitStruct);
}

/**
  * ��������: ���ڲ�������.
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
void GSM_USARTx_Init(void)
{ 
  /* GSMͨ�Ź�������GPIO��ʼ�� */
  GSM_GPIO_Init();
  __HAL_RCC_USART3_CLK_ENABLE();
  husartx_GSM.Instance = GSM_USARTx;
  husartx_GSM.Init.BaudRate = GSM_USARTx_BAUDRATE;
  husartx_GSM.Init.WordLength = UART_WORDLENGTH_8B;
  husartx_GSM.Init.StopBits = UART_STOPBITS_1;
  husartx_GSM.Init.Parity = UART_PARITY_NONE;
  husartx_GSM.Init.Mode = UART_MODE_TX_RX;
  husartx_GSM.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  husartx_GSM.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&husartx_GSM);
  HAL_NVIC_SetPriority(USART3_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(USART3_IRQn);
}

/**
  * ��������: ����������ת�����ַ���
  * �������: radix =10 ��ʾ10���ƣ��������Ϊ0
  *           value Ҫת����������
  *           buf ת������ַ���
  *           radix = 10
  * �� �� ֵ: ��
  * ˵    ������
  */
static char *itoa(int value, char *string, int radix)
{
  int     i, d;
  int     flag = 0;
  char    *ptr = string;

  /* This implementation only works for decimal numbers. */
  if (radix != 10)
  {
    *ptr = 0;
    return string;
  }
  if (!value)
  {
    *ptr++ = 0x30;
    *ptr = 0;
    return string;
  }
  /* if this is a negative value insert the minus sign. */
  if (value < 0)
  {
    *ptr++ = '-';
    /* Make the value positive. */
    value *= -1;
  }
  for (i = 10000; i > 0; i /= 10)
  {
    d = value / i;
    if (d || flag)
    {
      *ptr++ = (char)(d + 0x30);
      value -= (d * i);
      flag = 1;
    }
  }
  /* Null terminate the string. */
  *ptr = 0;
  return string;
} /* NCL_Itoa */

/**
  * ��������: ���ڷ���һ���ֽ����� 
  * �������: ch���������ַ�
  * �� �� ֵ: ��
  * ˵    ������
  */
void Usart_SendByte(uint8_t ch )
{
  	while(__HAL_UART_GET_FLAG(&husartx_GSM,UART_FLAG_TXE)==0); //ѭ������,ֱ���������
	/* ����һ���ֽ����ݵ�USART2 */
	HAL_UART_Transmit(&husartx_GSM,(uint8_t *)&ch,1,0xffff);
		
}

//�жϻ��洮������
#define UART_BUFF_SIZE      255
__IO  uint16_t uart_p = 0;
uint8_t   uart_buff[UART_BUFF_SIZE];

/**
  * ��������: �����жϻص�����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  
  if(uart_p<UART_BUFF_SIZE)
  {
    uart_buff[uart_p] =aRxBuffer; 
    uart_p++; 
    HAL_UART_Receive_IT(&husartx_GSM,&aRxBuffer,1);
  }
  else
  {
    clean_rebuff();       
  }
}

/**
  * ��������: ��ȡ���յ������ݺͳ��� 
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
char *get_rebuff(uint8_t *len) 
{
    *len = uart_p;
    return (char *)&uart_buff;
}

/**
  * ��������: ��ջ�����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
void clean_rebuff(void)
{
  uint16_t i=256;
  
  uart_p = 0;
	while(i)
		uart_buff[--i]=0;
}

/**
  * ��������: ��ʽ�������������C���е�printf��������û���õ�C��
  * �������: USARTx ����ͨ���������õ��˴���2����USART2
  *		        Data   Ҫ���͵����ڵ����ݵ�ָ��
  *			      ...    ��������
  * �� �� ֵ: ��
  * ˵    ��������Ӧ��
  *           GSM_USART_printf( USART2, "\r\n this is a demo \r\n" );
  *           GSM_USART_printf( USART2, "\r\n %d \r\n", i );
  *           GSM_USART_printf( USART2, "\r\n %s \r\n", j );
  */
void GSM_USART_printf(USART_TypeDef* USARTx, char *Data,...)
{
	const char *s;
  int d;   
  char buf[16];

  va_list ap;
  va_start(ap, Data);

	while ( *Data != 0)     // �ж��Ƿ񵽴��ַ���������
	{				                          
		if ( *Data == 0x5c )  //'\'
		{									  
			switch ( *++Data )
			{
				case 'r':							          //�س���
					Usart_SendByte(0x0d);
					Data ++;
					break;
				case 'n':							          //���з�
					Usart_SendByte( 0x0a);	
					Data ++;
					break;				
				default:
					Data ++;
				    break;
			}			 
		}
		else if ( *Data == '%')
		{									  //
			switch ( *++Data )
			{				
				case 's':										  //�ַ���
					s = va_arg(ap, const char *);
          for ( ; *s; s++) 
					{
						Usart_SendByte(*s);
						while( __HAL_UART_GET_FLAG(&husartx_GSM, UART_FLAG_TXE) == RESET );
          }
					Data++;
          break;
        case 'd':										//ʮ����
          d = va_arg(ap, int);
          itoa(d, buf, 10);
          for (s = buf; *s; s++) 
					{
						Usart_SendByte(*s);
						while( __HAL_UART_GET_FLAG(&husartx_GSM, UART_FLAG_TXE) == RESET );
          }
					Data++;
          break;
				 default:
						Data++;
				    break;
			}		 
		} /* end of else if */
		else Usart_SendByte(*Data++);
		while( __HAL_UART_GET_FLAG(&husartx_GSM, UART_FLAG_TXE) == RESET );
	}
}

/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/
