/**
  ******************************************************************************
  * 文件名程: main.c 
  * 作    者: 硬石嵌入式开发团队
  * 版    本: V1.0
  * 编写日期: 2015-10-04
  * 功    能: GSM模块功能实现：中英文短信功能
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
#include "stm32f1xx_hal.h"
#include "stm32f1xx_it.h"
#include "usart/bsp_debug_usart.h"
#include "led/bsp_led.h"
#include "key/bsp_key.h"
#include "usart/bsp_usartx.h"
#include "gsm/bsp_gsm.h"
#include <string.h>
#include "spiflash/bsp_spiflash.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>

/* 私有类型定义 --------------------------------------------------------------*/

/* 私有宏定义 ----------------------------------------------------------------*/
static  char *  PAPER_RESULT_PREFIX = "http://api.paper-go.com/paper_result";

/* 扩展变量 ------------------------------------------------------------------*/
/* 私有函数原形 --------------------------------------------------------------*/
/* 函数体 --------------------------------------------------------------------*/
/**
  * 函数功能: 系统时钟配置
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;  // 外部晶振，8MHz
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;  // 9倍频，得到72MHz主时钟
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;       // 系统时钟：72MHz
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;              // AHB时钟：72MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;               // APB1时钟：36MHz
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;               // APB2时钟：72MHz
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

 	// HAL_RCC_GetHCLKFreq()/1000    1ms中断一次
	// HAL_RCC_GetHCLKFreq()/100000	 10us中断一次
	// HAL_RCC_GetHCLKFreq()/1000000 1us中断一次
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);  // 配置并启动系统滴答定时器
  /* 系统滴答定时器时钟源 */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
  /* 系统滴答定时器中断优先级配置 */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/**
  * 函数功能: 初始化GSM
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */

void gsm_init(void){
// GPRS初始化环境 
	  GSM_gprs_init();
	  GSM_ip_init();
}


/**
  * 函数功能: 初始化短信模块
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
void msgInit(void){
	  GSM_tx_printf("AT+CNMI=2,1\r");         // 设置新消息指示
    GSM_DELAY(100);  
    GSM_tx_printf("AT+CMGF=1\r");           //文本模式
    GSM_DELAY(100);
    GSM_tx_printf("AT+CMGDA=\"DEL ALL\"\r");  //删除SIM卡内的短信
    GSM_DELAY(100);  
    GSM_CLEAN_RX();//清除接收缓存
}


/**
  * 函数功能: 主函数.
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
int main(void)
{
    /* 复位所有外设，初始化Flash接口和系统滴答定时器 */
    HAL_Init();
    /* 配置系统时钟 */
    SystemClock_Config();
    /* 板载LED初始化 */
    LED_GPIO_Init();

    /* 板子按键初始化 */
    KEY_GPIO_Init();

    /* 初始化串口并配置串口中断优先级 */
    MX_DEBUG_USART_Init();
  
    /* 初始化串行flash */
    MX_SPIFlash_Init();
  
    /* 检测模块响应是否正常 */
    while(GSM_init()!= GSM_TRUE)
    {
        printf("\n模块响应测试不正常！！\n");
        printf("若模块响应测试一直不正常，请检查模块的连接或是否已开启电源开关\n");
        GSM_DELAY(1000);
    }  
    // printf("\n通过了模块响应测试，1秒后开始配置GPRS通信环境...\n");  
    // 延时100豪秒再发送命令到模块 
    GSM_DELAY(100);
	
    msgInit();//初始化短信
		gsm_init();//初始化http
		printf("\nEverything is ready!\n");
		
		 //短信
    uint8_t newmessadd=0,IsRead=0;
		char namenum[20*4]={0},str[512]={0};
		
		/* 无限循环 */
    while(1)
    {
		    GSM_DELAY(1000);
		    newmessadd=IsReceiveMS();
		    if(newmessadd)
		    {    
            GSM_tx_printf("AT+CSCS=\"8859-1\"\r");     //"GSM"字符集
		        GSM_DELAY(500);
			      IsRead=readmessage(newmessadd,namenum,str);
						if(IsRead)
						{
	               GSM_CLEAN_RX();//清缓存						
						     printf("\n new msg content = %s \n\r",str);
							  
							   cJSON *json,*json_value;
							   json = cJSON_Parse(str);
							  
						     if (json != NULL)
							   {  
									   char* trade_no,size,per ;
                     printf("\n resole json \n\r");
  									 json_value = cJSON_GetObjectItem(json ,"trade_no");  
									   if( json_value->type == cJSON_String )  
									   {  
											    trade_no = json_value->valuestring;
									  	    printf("trade_no:%s\r\n",trade_no); 
											    
									   }
									   json_value = cJSON_GetObjectItem(json ,"size"); 
										 if( json_value->type == cJSON_Number )  
									   {  
											    size = json_value->valueint;
										      printf("size:%d\r\n",size);  
											    
									   }
										 json_value = cJSON_GetObjectItem(json ,"per");  
										 if( json_value->type == cJSON_Number )  
									   {  
											    per = json_value->valueint;			
										      printf("per:%d\r\n",per);
                          							 
									   }
										 char url[100];
										 strcpy (url,PAPER_RESULT_PREFIX);
										 strcat(url,"?trade_no=");
										 strcat (url,trade_no);
										 strcat (url,"&result=1");
										 
									   
										 //********************begin****************************
										 //执行出纸操作
										 
										 //********************begin****************************
										 
										 GSM_http_init();//http初始化
										 printf("\nurl ==== %s\n",url);
										 char *redata;
										 redata = GSM_http_get(url);
										 printf("\n服务器返回的参数：%s\n",redata);
										 GSM_CLEAN_RX();//清除接收缓存
										 GSM_http_end();
										 
										 // 释放内存空间 
										 //cJSON_Delete(json_value);
									   cJSON_Delete(json);
										 //free(json_value);
										 //free(json);
							   }
						 }
						 GSM_tx_printf("AT+CMGD=%d\r",newmessadd);         // 删除短信
						 GSM_DELAY(100); 
						 newmessadd=0;
			   }
     }

}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
