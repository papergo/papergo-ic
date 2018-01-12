#ifndef __BSP_GSM_H__
#define __BSP_GSM_H__

/* 包含头文件 ----------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "usart/bsp_usartx.h"

/* 类型定义 ------------------------------------------------------------------*/
typedef enum
{
  GSM_TRUE,
  GSM_FALSE,    
}GSM_res_e;

/* 宏定义 --------------------------------------------------------------------*/
#define GSM_config() 				        GSM_USARTx_Init()
#define GSM_tx_printf(fmt, ...)     GSM_USART_printf(GSM_USARTx,fmt,##__VA_ARGS__)     //printf格式发送命令（纯发送，不进行任何接收数据处理）

#define GSM_CLEAN_RX()              clean_rebuff()
#define GSM_ate0()                  GSM_cmd("ATE0\r","OK",100) //关闭回显
#define GSM_TX(cmd)                	GSM_tx_printf("%s",cmd)
#define GSM_IS_RX()                 (USART_GetFlagStatus(GSM_USARTx, USART_FLAG_RXNE) != RESET)
#define GSM_RX(len)                 ((char *)get_rebuff(&(len)))
#define GSM_DELAY(time)             HAL_Delay(time) //延时
#define GSM_SWAP16(data)    		    __REVSH(data)

/*************************** 电话 功能 ***************************/
#define GSM_HANGON()				GSM_TX("ATA\r");								
#define GSM_HANGOFF()				GSM_TX("ATH\r");	//挂断电话	

/* 扩展变量 ------------------------------------------------------------------*/
/* 函数声明 ------------------------------------------------------------------*/
//                  指令             正常返回
//本机号码          AT+CNUM\r         +CNUM: "","13265002063",129,7,4            //很多SIM卡默认都是没设置本机号码的，解决方法如下 http://www.multisilicon.com/blog/a21234642.html
//SIM营运商         AT+COPS?\r        +COPS: 0,0,"CHN-UNICOM"   OK
//SIM卡状态         AT+CPIN?\r        +CPIN: READY   OK
//SIM卡信号强度     AT+CSQ\r          +CSQ: 8,0   OK
uint8_t GSM_cmd(char *cmd,char *reply,uint32_t waittime);
uint8_t GSM_cmd_check(char *reply);
uint8_t GSM_init(void); //初始化并检测模块


/*************************** 电话 功能 ***************************/
uint8_t GSM_cnum(char *num); //获取本机号码
void GSM_call(char *num); //发起拨打电话（不管接不接通）
uint8_t IsRing(char *num); //查询是否来电，并保存来电号码

/***************************  短信功能  ****************************/
void GSM_sms(char *num,char *smstext); //发送短信（支持中英文,中文为GBK码）
char *GSM_waitask(uint8_t waitask_hook(void)); //等待有数据应答，返回接收缓冲区地址
void GSM_gbk2ucs2(char * ucs2,char * gbk);           
uint8_t IsReceiveMS(void);
uint8_t readmessage(uint8_t messadd,char *num,char *str);
uint8_t hexuni2gbk(char *hexuni,char *chgbk);

/*************************** GPRS 功能 ***************************/
void GSM_gprs_init(void); //GPRS初始化环境
char *GSM_http_get(char * serverip);//HTTP get 请求
char *GSM_http_post(char * serverip,char * params);//HTTP post 请求
void GSM_http_end(void);//HTTP end
void GSM_ip_init(void);
void GSM_http_init(void);
uint8_t GSM_gprs_tcp_link(char *localport,char * serverip,char * serverport); //TCP连接
uint8_t GSM_gprs_udp_link(char *localport,char * serverip,char * serverport); //UDP连接
uint8_t GSM_gprs_send(const char * str); //发送数据
uint8_t GSM_gprs_link_close(void); //IP链接断开
uint8_t GSM_gprs_shut_close(void); //关闭场景
uint8_t	PostGPRS(void);
char *Return_result(void); //获取服务器下下发内容
int split(char dst[][100], char* str, const char* spl);
#endif  /* __BSP_GSM_H__ */

/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/
