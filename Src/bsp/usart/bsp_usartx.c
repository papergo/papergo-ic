/**
  ******************************************************************************
  * 文件名程: bsp_usartx.c 
  * 作    者: 硬石嵌入式开发团队
  * 版    本: V1.0
  * 编写日期: 2015-10-04
  * 功    能: GSM串口底层驱动程序
  ******************************************************************************
  * 说明：
  * 本例程配套硬石YS-F1Mini板使用。
  * 
  * 淘宝：
  * 论坛：http://www.ing10bbs.com
  * 版权归硬石嵌入式开发团队所有，请勿商用。
  ******************************************************************************
  */

/* 包含头文件 ----------------------------------------------------------------*/
#include "usart/bsp_usartx.h"
#include <stdarg.h>

/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
UART_HandleTypeDef husartx_GSM;
static __IO uint32_t TimingDelay=0;

/* 扩展变量 ------------------------------------------------------------------*/
extern uint8_t aRxBuffer;
/* 私有函数原形 --------------------------------------------------------------*/
/* 函数体 --------------------------------------------------------------------*/
/**
  * 函数功能: GSM通信功能引脚GPIO初始化
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
void GSM_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* 串口外设时钟使能 */
  GSM_USARTx_GPIO_ClK_ENABLE();

  /* 串口外设功能GPIO配置 */
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
  * 函数功能: 串口参数配置.
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
void GSM_USARTx_Init(void)
{ 
  /* GSM通信功能引脚GPIO初始化 */
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
  * 函数功能: 将整形数据转换成字符串
  * 输入参数: radix =10 表示10进制，其他结果为0
  *           value 要转换的整形数
  *           buf 转换后的字符串
  *           radix = 10
  * 返 回 值: 无
  * 说    明：无
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
  * 函数功能: 串口发送一个字节数据 
  * 输入参数: ch：待发送字符
  * 返 回 值: 无
  * 说    明：无
  */
void Usart_SendByte(uint8_t ch )
{
  	while(__HAL_UART_GET_FLAG(&husartx_GSM,UART_FLAG_TXE)==0); //循环发送,直到发送完毕
	/* 发送一个字节数据到USART2 */
	HAL_UART_Transmit(&husartx_GSM,(uint8_t *)&ch,1,0xffff);
		
}

//中断缓存串口数据
#define UART_BUFF_SIZE      255
__IO  uint16_t uart_p = 0;
uint8_t   uart_buff[UART_BUFF_SIZE];

/**
  * 函数功能: 接收中断回调函数
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
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
  * 函数功能: 获取接收到的数据和长度 
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
char *get_rebuff(uint8_t *len) 
{
    *len = uart_p;
    return (char *)&uart_buff;
}

/**
  * 函数功能: 清空缓冲区
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
void clean_rebuff(void)
{
  uint16_t i=256;
  
  uart_p = 0;
	while(i)
		uart_buff[--i]=0;
}

/**
  * 函数功能: 格式化输出，类似于C库中的printf，但这里没有用到C库
  * 输入参数: USARTx 串口通道，这里用到了串口2，即USART2
  *		        Data   要发送到串口的内容的指针
  *			      ...    其他参数
  * 返 回 值: 无
  * 说    明：典型应用
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

	while ( *Data != 0)     // 判断是否到达字符串结束符
	{				                          
		if ( *Data == 0x5c )  //'\'
		{									  
			switch ( *++Data )
			{
				case 'r':							          //回车符
					Usart_SendByte(0x0d);
					Data ++;
					break;
				case 'n':							          //换行符
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
				case 's':										  //字符串
					s = va_arg(ap, const char *);
          for ( ; *s; s++) 
					{
						Usart_SendByte(*s);
						while( __HAL_UART_GET_FLAG(&husartx_GSM, UART_FLAG_TXE) == RESET );
          }
					Data++;
          break;
        case 'd':										//十进制
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

/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/
