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
						     printf("\n new msg content = %s \n\r",str);
							  
							   cJSON *json,*json_value;
							   json = cJSON_Parse(str);
							  
						     if (json != NULL)
							   {  
									   char* trade_no ;
                     printf("\n resole json \n\r");
  									 json_value = cJSON_GetObjectItem(json ,"trade_no");  
									   if( json_value->type == cJSON_String )  
									   {  
									  	    printf("trade_no:%s\r\n",json_value->valuestring); 
											    trade_no = json_value->valuestring;
									   }
									   json_value = cJSON_GetObjectItem(json ,"size"); 
										 if( json_value->type == cJSON_Number )  
									   {  
										     printf("size:%d\r\n",json_value->valueint);  
									   }
										 json_value = cJSON_GetObjectItem(json ,"per");  
										 if( json_value->type == cJSON_Number )  
									   {  
										     printf("per:%d\r\n",json_value->valueint);  
									   }
									   // 释放内存空间 
										 //cJSON_Delete(json_value);
									   cJSON_Delete(json);
										 
										 //********************begin****************************
										 //执行出纸操作
										 
										 //********************begin****************************
										 
										 GSM_http_init();//http初始化
										 
//										 char *redata;
//										 redata = GSM_http_get("http://api.paper-go.com/test3/123456");
//										 printf("\n服务器返回的参数：%s\n",redata);
//										 GSM_CLEAN_RX();//清除接收缓存
									   
										 // 创建JSON Object  
										 cJSON *notifyJson = cJSON_CreateObject();  
										 // 加入节点（键值对），节点名称为value，节点值为123.4  
										 cJSON_AddStringToObject(notifyJson,"trade_no",trade_no);
										 cJSON_AddStringToObject(notifyJson,"result","1");  
										 char *notify = cJSON_Print(notifyJson);  
										 char *redata2;
										 redata2 = GSM_http_post("http://api.paper-go.com/paper_result",notify);
										 printf("\npost服务器返回的参数：%s\n",redata2);
										 // 释放内存  
										 cJSON_Delete(notifyJson);  
										 free(notify);
										 GSM_CLEAN_RX();//清除接收缓存
										 GSM_http_end();
										 
										 
							   }
						 }
						 GSM_tx_printf("AT+CMGD=%d\r",newmessadd);         // 删除短信
						 GSM_DELAY(100); 
						 newmessadd=0;
			   }
     }

    
//	  char *redata;
//	  redata = GSM_http_get("http://api.paper-go.com/test3/123456");
//	  printf("\n服务器返回的参数：%s\n",redata);
//	  GSM_CLEAN_RX();//清除接收缓存
//	
//	
//	  char *redata2;
//	  redata2 = GSM_http_post("http://api.paper-go.com/test_post2","{\"code\":654321}");
//	  printf("\npost服务器返回的参数：%s\n",redata2);
//	  GSM_CLEAN_RX();//清除接收缓存
//	  GSM_http_end();
//	
//	
//    cJSON * pJson = cJSON_Parse(redata);
//	  if(pJson != NULL){
//	      cJSON * pSub = cJSON_GetObjectItem(pJson, "result");
//		    if(pSub != NULL){
//			      printf("\nJSON读取数据: result == %d\n",pSub->valueint);
//		    }
//		    pSub = cJSON_GetObjectItem(pJson, "status");
//		    if(pSub != NULL){
//			      pJson = cJSON_GetObjectItem(pSub, "code");
//			      printf("\nJSON读取数据: code == %d\n",pJson->valueint);
//			      pJson = cJSON_GetObjectItem(pSub, "description");
//			      printf("\nJSON读取数据: des == %s\n",pJson->valuestring);
//					
//			      //cJSON_Delete(pSubSub);
//		    }
//		    cJSON_Delete(pSub);
//    }
//	  cJSON_Delete(pJson);
//	
//	  GSM_CLEAN_RX();//清除接收缓存
//	
//		/* 无限循环 */
//    while(1)
//	 {
//		GSM_DELAY(1000);
//		newmessadd=IsReceiveMS();	
////		if(newmessadd != 0)
////		    printf("\n new msg address ==== %c\n\r",newmessadd);
//		if(newmessadd)
//		{    
//		    GSM_tx_printf("AT+CSCS=\"8859-1\"\r");     //"GSM"字符集
//		    GSM_DELAY(500);
//			IsRead=readmessage(newmessadd,namenum,str);			
////			printf("newmessadd=%d,IsRead:%d\n",newmessadd,IsRead);
//			if(IsRead)
//			{
//				//hexuni2gbk(namenum,namegbk);	
//				//hexuni2gbk(str,gbkstr);						
//				//printf("\n新短信:\n发件人:%s\n内容:%s\n\r",namegbk,gbkstr);
//				printf("\n new msg content = %s \n\r",str);
//			}
//		    GSM_tx_printf("AT+CMGD=%d\r",newmessadd);         // 删除短信
//		    GSM_DELAY(100); 
//		    newmessadd=0;
//		}
//    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
