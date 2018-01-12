/**
  ******************************************************************************
  * �ļ�����: main.c 
  * ��    ��: ӲʯǶ��ʽ�����Ŷ�
  * ��    ��: V1.0
  * ��д����: 2015-10-04
  * ��    ��: GSMģ�鹦��ʵ�֣���Ӣ�Ķ��Ź���
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

/* ˽�����Ͷ��� --------------------------------------------------------------*/

/* ˽�к궨�� ----------------------------------------------------------------*/
static  char *  PAPER_RESULT_PREFIX = "http://api.paper-go.com/paper_result";

/* ��չ���� ------------------------------------------------------------------*/
/* ˽�к���ԭ�� --------------------------------------------------------------*/
/* ������ --------------------------------------------------------------------*/
/**
  * ��������: ϵͳʱ������
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;  // �ⲿ����8MHz
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;  // 9��Ƶ���õ�72MHz��ʱ��
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;       // ϵͳʱ�ӣ�72MHz
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;              // AHBʱ�ӣ�72MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;               // APB1ʱ�ӣ�36MHz
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;               // APB2ʱ�ӣ�72MHz
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

 	// HAL_RCC_GetHCLKFreq()/1000    1ms�ж�һ��
	// HAL_RCC_GetHCLKFreq()/100000	 10us�ж�һ��
	// HAL_RCC_GetHCLKFreq()/1000000 1us�ж�һ��
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);  // ���ò�����ϵͳ�δ�ʱ��
  /* ϵͳ�δ�ʱ��ʱ��Դ */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
  /* ϵͳ�δ�ʱ���ж����ȼ����� */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/**
  * ��������: ��ʼ��GSM
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */

void gsm_init(void){
// GPRS��ʼ������ 
	  GSM_gprs_init();
	  GSM_ip_init();
}


/**
  * ��������: ��ʼ������ģ��
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
void msgInit(void){
	  GSM_tx_printf("AT+CNMI=2,1\r");         // ��������Ϣָʾ
    GSM_DELAY(100);  
    GSM_tx_printf("AT+CMGF=1\r");           //�ı�ģʽ
    GSM_DELAY(100);
    GSM_tx_printf("AT+CMGDA=\"DEL ALL\"\r");  //ɾ��SIM���ڵĶ���
    GSM_DELAY(100);  
    GSM_CLEAN_RX();//������ջ���
}


/**
  * ��������: ������.
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
int main(void)
{
    /* ��λ�������裬��ʼ��Flash�ӿں�ϵͳ�δ�ʱ�� */
    HAL_Init();
    /* ����ϵͳʱ�� */
    SystemClock_Config();
    /* ����LED��ʼ�� */
    LED_GPIO_Init();

    /* ���Ӱ�����ʼ�� */
    KEY_GPIO_Init();

    /* ��ʼ�����ڲ����ô����ж����ȼ� */
    MX_DEBUG_USART_Init();
  
    /* ��ʼ������flash */
    MX_SPIFlash_Init();
  
    /* ���ģ����Ӧ�Ƿ����� */
    while(GSM_init()!= GSM_TRUE)
    {
        printf("\nģ����Ӧ���Բ���������\n");
        printf("��ģ����Ӧ����һֱ������������ģ������ӻ��Ƿ��ѿ�����Դ����\n");
        GSM_DELAY(1000);
    }  
    // printf("\nͨ����ģ����Ӧ���ԣ�1���ʼ����GPRSͨ�Ż���...\n");  
    // ��ʱ100�����ٷ������ģ�� 
    GSM_DELAY(100);
	
    msgInit();//��ʼ������
		gsm_init();//��ʼ��http
		printf("\nEverything is ready!\n");
		
		 //����
    uint8_t newmessadd=0,IsRead=0;
		char namenum[20*4]={0},str[512]={0};
		
		/* ����ѭ�� */
    while(1)
    {
		    GSM_DELAY(1000);
		    newmessadd=IsReceiveMS();
		    if(newmessadd)
		    {    
            GSM_tx_printf("AT+CSCS=\"8859-1\"\r");     //"GSM"�ַ���
		        GSM_DELAY(500);
			      IsRead=readmessage(newmessadd,namenum,str);
						if(IsRead)
						{
	               GSM_CLEAN_RX();//�建��						
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
										 //ִ�г�ֽ����
										 
										 //********************begin****************************
										 
										 GSM_http_init();//http��ʼ��
										 printf("\nurl ==== %s\n",url);
										 char *redata;
										 redata = GSM_http_get(url);
										 printf("\n���������صĲ�����%s\n",redata);
										 GSM_CLEAN_RX();//������ջ���
										 GSM_http_end();
										 
										 // �ͷ��ڴ�ռ� 
										 //cJSON_Delete(json_value);
									   cJSON_Delete(json);
										 //free(json_value);
										 //free(json);
							   }
						 }
						 GSM_tx_printf("AT+CMGD=%d\r",newmessadd);         // ɾ������
						 GSM_DELAY(100); 
						 newmessadd=0;
			   }
     }

}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
