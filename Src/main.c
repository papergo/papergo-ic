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
									   // �ͷ��ڴ�ռ� 
										 //cJSON_Delete(json_value);
									   cJSON_Delete(json);
										 
										 //********************begin****************************
										 //ִ�г�ֽ����
										 
										 //********************begin****************************
										 
										 GSM_http_init();//http��ʼ��
										 
//										 char *redata;
//										 redata = GSM_http_get("http://api.paper-go.com/test3/123456");
//										 printf("\n���������صĲ�����%s\n",redata);
//										 GSM_CLEAN_RX();//������ջ���
									   
										 // ����JSON Object  
										 cJSON *notifyJson = cJSON_CreateObject();  
										 // ����ڵ㣨��ֵ�ԣ����ڵ�����Ϊvalue���ڵ�ֵΪ123.4  
										 cJSON_AddStringToObject(notifyJson,"trade_no",trade_no);
										 cJSON_AddStringToObject(notifyJson,"result","1");  
										 char *notify = cJSON_Print(notifyJson);  
										 char *redata2;
										 redata2 = GSM_http_post("http://api.paper-go.com/paper_result",notify);
										 printf("\npost���������صĲ�����%s\n",redata2);
										 // �ͷ��ڴ�  
										 cJSON_Delete(notifyJson);  
										 free(notify);
										 GSM_CLEAN_RX();//������ջ���
										 GSM_http_end();
										 
										 
							   }
						 }
						 GSM_tx_printf("AT+CMGD=%d\r",newmessadd);         // ɾ������
						 GSM_DELAY(100); 
						 newmessadd=0;
			   }
     }

    
//	  char *redata;
//	  redata = GSM_http_get("http://api.paper-go.com/test3/123456");
//	  printf("\n���������صĲ�����%s\n",redata);
//	  GSM_CLEAN_RX();//������ջ���
//	
//	
//	  char *redata2;
//	  redata2 = GSM_http_post("http://api.paper-go.com/test_post2","{\"code\":654321}");
//	  printf("\npost���������صĲ�����%s\n",redata2);
//	  GSM_CLEAN_RX();//������ջ���
//	  GSM_http_end();
//	
//	
//    cJSON * pJson = cJSON_Parse(redata);
//	  if(pJson != NULL){
//	      cJSON * pSub = cJSON_GetObjectItem(pJson, "result");
//		    if(pSub != NULL){
//			      printf("\nJSON��ȡ����: result == %d\n",pSub->valueint);
//		    }
//		    pSub = cJSON_GetObjectItem(pJson, "status");
//		    if(pSub != NULL){
//			      pJson = cJSON_GetObjectItem(pSub, "code");
//			      printf("\nJSON��ȡ����: code == %d\n",pJson->valueint);
//			      pJson = cJSON_GetObjectItem(pSub, "description");
//			      printf("\nJSON��ȡ����: des == %s\n",pJson->valuestring);
//					
//			      //cJSON_Delete(pSubSub);
//		    }
//		    cJSON_Delete(pSub);
//    }
//	  cJSON_Delete(pJson);
//	
//	  GSM_CLEAN_RX();//������ջ���
//	
//		/* ����ѭ�� */
//    while(1)
//	 {
//		GSM_DELAY(1000);
//		newmessadd=IsReceiveMS();	
////		if(newmessadd != 0)
////		    printf("\n new msg address ==== %c\n\r",newmessadd);
//		if(newmessadd)
//		{    
//		    GSM_tx_printf("AT+CSCS=\"8859-1\"\r");     //"GSM"�ַ���
//		    GSM_DELAY(500);
//			IsRead=readmessage(newmessadd,namenum,str);			
////			printf("newmessadd=%d,IsRead:%d\n",newmessadd,IsRead);
//			if(IsRead)
//			{
//				//hexuni2gbk(namenum,namegbk);	
//				//hexuni2gbk(str,gbkstr);						
//				//printf("\n�¶���:\n������:%s\n����:%s\n\r",namegbk,gbkstr);
//				printf("\n new msg content = %s \n\r",str);
//			}
//		    GSM_tx_printf("AT+CMGD=%d\r",newmessadd);         // ɾ������
//		    GSM_DELAY(100); 
//		    newmessadd=0;
//		}
//    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
