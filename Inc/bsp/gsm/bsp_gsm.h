#ifndef __BSP_GSM_H__
#define __BSP_GSM_H__

/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "usart/bsp_usartx.h"

/* ���Ͷ��� ------------------------------------------------------------------*/
typedef enum
{
  GSM_TRUE,
  GSM_FALSE,    
}GSM_res_e;

/* �궨�� --------------------------------------------------------------------*/
#define GSM_config() 				        GSM_USARTx_Init()
#define GSM_tx_printf(fmt, ...)     GSM_USART_printf(GSM_USARTx,fmt,##__VA_ARGS__)     //printf��ʽ������������ͣ��������κν������ݴ���

#define GSM_CLEAN_RX()              clean_rebuff()
#define GSM_ate0()                  GSM_cmd("ATE0\r","OK",100) //�رջ���
#define GSM_TX(cmd)                	GSM_tx_printf("%s",cmd)
#define GSM_IS_RX()                 (USART_GetFlagStatus(GSM_USARTx, USART_FLAG_RXNE) != RESET)
#define GSM_RX(len)                 ((char *)get_rebuff(&(len)))
#define GSM_DELAY(time)             HAL_Delay(time) //��ʱ
#define GSM_SWAP16(data)    		    __REVSH(data)

/*************************** �绰 ���� ***************************/
#define GSM_HANGON()				GSM_TX("ATA\r");								
#define GSM_HANGOFF()				GSM_TX("ATH\r");	//�Ҷϵ绰	

/* ��չ���� ------------------------------------------------------------------*/
/* �������� ------------------------------------------------------------------*/
//                  ָ��             ��������
//��������          AT+CNUM\r         +CNUM: "","13265002063",129,7,4            //�ܶ�SIM��Ĭ�϶���û���ñ�������ģ������������ http://www.multisilicon.com/blog/a21234642.html
//SIMӪ����         AT+COPS?\r        +COPS: 0,0,"CHN-UNICOM"   OK
//SIM��״̬         AT+CPIN?\r        +CPIN: READY   OK
//SIM���ź�ǿ��     AT+CSQ\r          +CSQ: 8,0   OK
uint8_t GSM_cmd(char *cmd,char *reply,uint32_t waittime);
uint8_t GSM_cmd_check(char *reply);
uint8_t GSM_init(void); //��ʼ�������ģ��


/*************************** �绰 ���� ***************************/
uint8_t GSM_cnum(char *num); //��ȡ��������
void GSM_call(char *num); //���𲦴�绰�����ܽӲ���ͨ��
uint8_t IsRing(char *num); //��ѯ�Ƿ����磬�������������

/***************************  ���Ź���  ****************************/
void GSM_sms(char *num,char *smstext); //���Ͷ��ţ�֧����Ӣ��,����ΪGBK�룩
char *GSM_waitask(uint8_t waitask_hook(void)); //�ȴ�������Ӧ�𣬷��ؽ��ջ�������ַ
void GSM_gbk2ucs2(char * ucs2,char * gbk);           
uint8_t IsReceiveMS(void);
uint8_t readmessage(uint8_t messadd,char *num,char *str);
uint8_t hexuni2gbk(char *hexuni,char *chgbk);

/*************************** GPRS ���� ***************************/
void GSM_gprs_init(void); //GPRS��ʼ������
char *GSM_http_get(char * serverip);//HTTP get ����
char *GSM_http_post(char * serverip,char * params);//HTTP post ����
void GSM_http_end(void);//HTTP end
void GSM_ip_init(void);
void GSM_http_init(void);
uint8_t GSM_gprs_tcp_link(char *localport,char * serverip,char * serverport); //TCP����
uint8_t GSM_gprs_udp_link(char *localport,char * serverip,char * serverport); //UDP����
uint8_t GSM_gprs_send(const char * str); //��������
uint8_t GSM_gprs_link_close(void); //IP���ӶϿ�
uint8_t GSM_gprs_shut_close(void); //�رճ���
uint8_t	PostGPRS(void);
char *Return_result(void); //��ȡ���������·�����
int split(char dst[][100], char* str, const char* spl);
#endif  /* __BSP_GSM_H__ */

/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/
