/**
  ******************************************************************************
  * 文件名程: GSM.c 
  * 作    者: 硬石嵌入式开发团队
  * 版    本: V1.0
  * 编写日期: 2015-10-04
  * 功    能: GSM模块底层驱动实现
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
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "usart/bsp_debug_usart.h"
#include "usart/bsp_usartx.h"
#include "GSM/bsp_GSM.h"
/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
static uint8_t MaxMessAdd=50;
uint8_t aRxBuffer;
static uint8_t timecount=1,timestop=0;

/* 扩展变量 ------------------------------------------------------------------*/
extern uint16_t ff_convert(uint16_t	src,uint32_t	dir);

/* 私有函数原形 --------------------------------------------------------------*/
/* 函数体 --------------------------------------------------------------------*/

/**
  * 函数功能: 向GSM发送一个命令，并验证返回值是否正确
  * 输入参数: cmd：命令
  *           replay：期待返回值
  *           waittime：等待时间
  * 返 回 值: GSM_TRUE：成功
  *           GSM_FALSE：失败
  * 说    明：无
  */
uint8_t GSM_cmd(char *cmd, char *reply,uint32_t waittime )
{    
  GSM_CLEAN_RX();                 //清空了接收缓冲区数据
  GSM_TX(cmd);                    //发送命令
  if(reply == 0)                      //不需要接收数据
  {
    return GSM_TRUE;
  }
  GSM_DELAY(waittime);                 //延时  
  return GSM_cmd_check(reply);    //对接收数据进行处理
}

/**
  * 函数功能: 验证命令的返回值是否正确
  * 输入参数: replay：期待返回值
  * 返 回 值: GSM_TRUE：成功
  *           GSM_FALSE：失败
  * 说    明：无
  */
uint8_t GSM_cmd_check(char *reply)
{
  uint8_t len;
  uint8_t n;
  uint8_t off;
  char *redata;
  
  redata = GSM_RX(len); //接收数据
  n = 0;
  off = 0;
  while((n + off)<len)
  {
    if(reply[n] == 0) //数据为空或者比较完毕
    {
      return GSM_TRUE;
    }
    if(redata[ n + off]== reply[n])
    {
      n++; //移动到下一个接收数据
    }
    else
    {
      off++; //进行下一轮匹配
      n=0; //重来
    }
    //n++;
  }
  if(reply[n]==0) //刚好匹配完毕
  {
     return GSM_TRUE;
  }  
  return GSM_FALSE; //跳出循环表示比较完毕后都没有相同的数据，因此跳出
}

/**
  * 函数功能: 等待GSM有数据应答
  * 输入参数: 无
  * 返 回 值: 应答数据串
  * 说    明：无
  */
char * GSM_waitask(uint8_t waitask_hook(void))
{
  uint8_t len=0;
  char *redata;
  do
  {    
    redata = GSM_RX(len);   //接收数据
    if(waitask_hook!=0)
    {
      if(waitask_hook()==GSM_TRUE) //返回 GSM_TRUE 表示检测到事件，需要退出
      {
        redata = 0;
        return redata;               
      }
    }
  }while(len==0);                 //接收数据为0时一直等待  
  GSM_DELAY(20);              //延时，确保能接收到全部数据（115200波特率下，每ms能接收11.52个字节）
	printf("PostUDP:%s\n",redata);
  return redata;
}

/**
  * 函数功能: 获取本机号码
  * 输入参数: num：存放号码的缓冲区
  * 返 回 值: GSM_TRUE：成功
  *           GSM_FALSE：失败
  * 说    明：无
  */
uint8_t GSM_cnum(char *num)
{
  char *redata;
  uint8_t len;
  
  if(GSM_cmd("AT+CNUM\r","OK", 100) != GSM_TRUE)
  {
    return GSM_FALSE;
  }  
  redata = GSM_RX(len);   //接收数据
  if(len == 0)
  {
    return GSM_FALSE;
  }  
  //第一个逗号后面的数据为:"本机号码"
  while(*redata != ',')
  {
    len--;
    if(len==0)
    {
      return GSM_FALSE;
    }
    redata++;
  }
  redata+=2;  
  while(*redata != '"')
  {
    *num++ = *redata++;
  }
  *num = 0;               //字符串结尾需要清0
  return GSM_TRUE;
}

/**
  * 函数功能: 初始化并检测模块
  * 输入参数: 无
  * 返 回 值: GSM_TRUE：成功
  *           GSM_FALSE：失败
  * 说    明：无
  */
uint8_t GSM_init(void)
{
	char *redata;
	uint8_t len;
	
	GSM_CLEAN_RX(); //清空了接收缓冲区数据
	GSM_config(); //初始化串口
	
  GSM_tx_printf("ATQ0\r");
  GSM_DELAY(200);
  HAL_UART_Receive_IT(&husartx_GSM,&aRxBuffer,1);
	if(GSM_cmd("ATE0\r","OK", 200) != GSM_TRUE)
	{    
		return GSM_FALSE;
	}  
  GSM_CLEAN_RX(); //清空了接收缓冲区数据
	if(GSM_cmd("AT+CGMM\r","OK", 200) != GSM_TRUE)
	{
	  return GSM_FALSE;
	}	
	redata = GSM_RX(len);   //接收数据
	if(len == 0)
	{
	  return GSM_FALSE;
	}
	if (strstr(redata,"SIMCOM_SIM") != 0)
	{
    
		return GSM_TRUE;
	}
	else
		return GSM_FALSE;
}

/*-------------------------   电话功能  ---------------------------------*/
/**
  * 函数功能: 发起拨打电话（不管接不接通）
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
void GSM_call(char *num)
{
  GSM_CLEAN_RX(); //清空了接收缓冲区数据
  GSM_tx_printf("ATD%s;\r",num);
  //拨打后是不返回数据的
  //不管任何应答，开头都是"\r\n",即真正有效的数据是从第3个字节开始
  //对方挂掉电话（没接通），返回：BUSY
  //对方接听了电话：+COLP: "555",129,"",0,"9648674F98DE"   OK
  //对方接听了电话,然后挂掉：NO CARRIER
  //对方超时没接电话：NO ANSWER
}

/**
  * 函数功能: 有来电电话
  * 输入参数: num：来电号码缓冲区
  * 返 回 值: GSM_TRUE：成功
  *           GSM_FALSE：失败
  * 说    明：无
  */
uint8_t IsRing(char *num)
{
  char *redata;
  uint8_t len;
  
  if(GSM_cmd_check("RING"))
  {
    return GSM_FALSE;
  }  
  redata = GSM_RX(len);   //接收数据
  if(len == 0)
  {
    return GSM_FALSE;
  }  
  //第一个逗号后面的数据为:”号码“
  while(*redata != ':')
  {
    len--;
    if(len==0)
    {
      return GSM_FALSE;
    }
    redata++;
  }
  redata+=3;
  while(*redata != '"')
  {
    *num++ = *redata++;
  }
  *num = 0;               //字符串结尾需要清0
  GSM_CLEAN_RX();
  return GSM_TRUE;
}

/*-------------------------   短信功能  ---------------------------------*/
/**
  * 函数功能: 判断字符串只是ASCLL，没有中文
  * 输入参数: str：字符串缓冲区
  * 返 回 值: GSM_TRUE：成功
  *           GSM_FALSE：失败
  * 说    明：无
  */
uint8_t IsASSIC(char * str)
{
  while(*str)
  {
    if(*str>0x7F)
    {
      return GSM_FALSE;
    }
    str++;
  }
  return GSM_TRUE;
}

/**
  * 函数功能: GBK编码转unicode编码
  * 输入参数: ucs2：Unicode编码缓冲区
  *           gbk：GBK编码缓冲区
  * 返 回 值: 无
  * 说    明：无
  */
void GSM_gbk2ucs2(char *ucs2,char *gbk)
{
  uint16_t   tmp;  
  while(*gbk)
  {
    if((*gbk&0xFF) < 0x7F)      //英文
    {        
      *ucs2++ = 0;
      *ucs2++ = *gbk++;  
    }
    else                        //中文
    {
      tmp = GSM_SWAP16(*(uint16_t *)gbk);
      *(uint16_t *)ucs2 = GSM_SWAP16(ff_convert(tmp,1));
      gbk+=2;
      ucs2+=2;
    }
  }
}

/**
  * 函数功能: 数字字符转16进制字符串
  * 输入参数: hex：16进制字符串缓冲区
  *           ch：数字字符
  * 返 回 值: 无
  * 说    明：无
  */
void GSM_char2hex(char *hex,char ch)
{
  const char numhex[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  *hex++  = numhex[(ch & 0xF0)>>4];
  *hex    = numhex[ ch & 0x0F];
}

/**
  * 函数功能: GBK编码转unicode编码16进制字符串
  * 输入参数: ucs2：Unicode编码16进制字符串缓冲区
  *           gbk：GBK编码缓冲区
  * 返 回 值: 无
  * 说    明：无
  */
void GSM_gbk2ucs2hex(char * ucs2hex,char * gbk)
{
  uint16_t   tmp;
  
  while(*gbk)
  {
    if((*gbk&0xFF) < 0x7F)      //英文
    {        
      *ucs2hex++ = '0';
      *ucs2hex++ = '0';
      GSM_char2hex(ucs2hex,*gbk);
      ucs2hex+=2;
      gbk++;  
    }
    else                        //中文
    {
      tmp = GSM_SWAP16(*(uint16_t *)gbk);
      tmp = ff_convert(tmp,1);
      GSM_char2hex(ucs2hex,(char)(tmp>>8));
      ucs2hex+=2;
      GSM_char2hex(ucs2hex,(char)tmp);
      ucs2hex+=2;
      gbk+=2;
    }
  }
  *ucs2hex=0;
}

/**
  * 函数功能: 发送短信（支持中英文,中文为GBK码）
  * 输入参数: num：对方手机号码
  *           smstext：短信内容
  * 返 回 值: 无
  * 说    明：无
  */
void GSM_sms(char *num,char *smstext)
{
  char ucsbuff[160];
  char end[2] = {0x1A,0x00};

  GSM_CLEAN_RX();                 //清空了接收缓冲区数据
  if(IsASSIC(smstext)==GSM_TRUE)
  {
    //英文
    GSM_tx_printf("AT+CSCS=\"GSM\"\r");     //"GSM"字符集
    GSM_DELAY(100);    
    GSM_tx_printf("AT+CMGF=1\r");           //文本模式
    GSM_DELAY(100);    
    GSM_tx_printf("AT+CMGS=\"%s\"\r",num);  //电话号码
    GSM_DELAY(100);
    GSM_tx_printf("%s",smstext);            //短信内容        
  }
  else
  {
      //中文
      GSM_tx_printf("AT+CSCS=\"UCS2\"\r");    //"UCS2"字符集
      GSM_DELAY(100);      
      GSM_tx_printf("AT+CMGF=1\r");           //文本模式
      GSM_DELAY(100);      
      GSM_tx_printf("AT+CSMP=17,167,0,8\r");  //
      GSM_DELAY(100);      
      GSM_gbk2ucs2hex(ucsbuff,num);
      GSM_tx_printf("AT+CMGS=\"%s\"\r",ucsbuff);  //UCS2的电话号码(需要转成 ucs2码)
      GSM_DELAY(100);      
      GSM_gbk2ucs2hex(ucsbuff,smstext);
      GSM_tx_printf("%s",ucsbuff);          //UCS2的文本内容(需要转成 ucs2码)
  }
  GSM_DELAY(1); 
  // GSM_tx_printf("%c",0x1A);
  GSM_tx_printf("%s",end);
	//while((USART2->SR&0X40)==0);//等待上一次数据发送完成  
	//GSM_USARTx->DR=(u32)0x1A;		//发送十六进制数：0X1A,信息结束符号
}

/**
  * 函数功能: 查询是否接收到新短信
  * 输入参数: 无
  * 返 回 值: 0:无新短信
  *           非0：新短信地址
  * 说    明：无
  */
uint8_t IsReceiveMS(void)
{
	char address[3]={0};
	uint8_t addressnum=0;
	char *redata;
  uint8_t len;
/*------------- 查询是否有新短信并提取存储位置 ----------------------------*/
  if(GSM_cmd_check("+CMTI:"))
  {
    return 0;
  }   
  redata = GSM_RX(len);   //接收数据
	//printf("CMTI:redata:%s,len=%d\n",redata,len);
  if(len == 0)
  {
    return 0;
  }    
  //第一个逗号后面的数据为:”短信存储地址“
  while(*redata != ',')
  {
    len--;
    if(len==0)
    {
        return 0;
    }
    redata++;
  }
	redata+=1;//去掉；','
	address[0]= *redata++;
	address[1]= *redata++;
	address[2]= *redata++;
	if((address[2]>='0')&&(address[2]<='9'))
		addressnum=(address[0]-'0')*100+(address[1]-'0')*10+(address[2]-'0');
	else if((address[1]>='0')&&(address[1]<='9'))
		addressnum=(address[0]-'0')*10+(address[1]-'0');
	else
		addressnum=address[0]-'0';
	//printf("address:%c%c%c->%d\n",address[0],address[1],address[2],addressnum);	
  return addressnum;
}	

/**
  * 函数功能: 读取短信内容
  * 输入参数: messadd：短信地址
  *        		num：保存发件人号码(unicode编码格式的字符串)
  *     			str：保存短信内容(unicode编码格式的字符串)
  * 返 回 值: 0：失败
  * 	        1：成功读取到短信，该短信未读（此处是第一次读，读完后会自动被标准为已读）
  *      			2：成功读取到短信，该短信已读
  * 说    明：无
  */
uint8_t readmessage(uint8_t messadd,char *num,char *str)
{
	char *redata,cmd[20]={0};
    uint8_t len;
	char result=0;
	GSM_CLEAN_RX();                 //清空了接收缓冲区数据
	if(messadd>MaxMessAdd)return 0;
	
/*------------- 读取短信内容 ----------------------------*/
	sprintf(cmd,"AT+CMGR=%d\r",messadd);	
	if(GSM_cmd(cmd,"+CMGR:",500) != GSM_TRUE)
	{
		return 0;
	}
	redata = GSM_RX(len);   //接收数据
	if(len == 0)
	{
		return 0;
	}
	printf("CMGR:redata:%s\nlen=%d\n",redata,len);
	if(strstr(redata,"UNREAD")==0)result=2;
	else	result=1;
	//第一个逗号后面的数据为:”发件人号码“
	while(*redata != ',')
	{
		len--;
		if(len==0)
		{
			return 0;
		}
		redata++;
	}
	redata+=2;//去掉',"'
	while(*redata != '"')
	{
		*num++ = *redata++;
		len--;
	}
	*num = 0;               //字符串结尾需要清0
	
	while(*redata != '+')
	{
		len--;
		if(len==0)
		{
			return 0;
		}
		redata++;
	}
	redata+=6;//去掉'+32"\r'
	while(*redata != '\r')
	{
		*str++ = *redata++;
	}
	*str = 0;               //字符串结尾需要清0
	printf("CMGR:result:%d\n",result);
	return result;
}

/**
  * 函数功能: unicode编码16进制字符串转GBK编码
  * 输入参数: hexuni：Unicode编码16进制字符串缓冲区
  *           chgbk：GBK编码缓冲区
  * 返 回 值: GSM_TRUE：成功
  *           GSM_FALSE：失败
  * 说    明：无
  */
uint8_t hexuni2gbk(char *hexuni,char *chgbk)
{
	uint8_t len=0,i=0;
	uint16_t wgbk=0;
	uint16_t tmp=0;
	uint8_t unitmp[4]={0};
	GSM_CLEAN_RX();
	len=strlen(hexuni);
	if(!len)return GSM_FALSE;
	//printf("hexuni:%s::len:%d\n",hexuni,len);
	for(i=0;i<len/4;++i)
	{
		if(hexuni[4*i]>=0x41)	unitmp[0]=hexuni[4*i]-0x41+10;
		else	unitmp[0]=hexuni[4*i]-0x30;
		if(hexuni[4*i+1]>=0x41)unitmp[1]=hexuni[4*i+1]-0x41+10;
		else	unitmp[1]=hexuni[4*i+1]-0x30;
		if(hexuni[4*i+2]>=0x41)unitmp[2]=hexuni[4*i+2]-0x41+10;
		else	unitmp[2]=hexuni[4*i+2]-0x30;
		if(hexuni[4*i+3]>=0x41)unitmp[3]=hexuni[4*i+3]-0x41+10;
		else	unitmp[3]=hexuni[4*i+3]-0x30;
		
		tmp=unitmp[0]*0x1000+unitmp[1]*0x100+unitmp[2]*16+unitmp[3];
		wgbk=ff_convert(tmp,0);
		//printf("tmp:%X->wgbk:%X\n",tmp,wgbk);
		
		if(wgbk<0x80)
		{
			*chgbk=(char)wgbk;
			chgbk++;
		}
		else
		{
			*chgbk=(char)(wgbk>>8);
			chgbk++;
			*chgbk=(char)wgbk;
			chgbk++;
		}		
	}	
	*chgbk=0;
	return GSM_TRUE;		
}

/*--------------------------   GPRS 功能   ---------------------------------*/
/**
  * 函数功能: GPRS功能环境初始化
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
void GSM_gprs_init(void)
{
  GSM_tx_printf("AT+CGCLASS=\"B\"\r");                       //设置移动台类别为"B"
  GSM_DELAY(100);
  GSM_tx_printf("AT+CGDCONT=1,\"IP\",\"CMNET\"\r");          //PDP上下文标识1，互联网协议，接入点名称：CMNET
  GSM_DELAY(100);
  GSM_tx_printf("AT+CGATT=1\r");                             //附着 GPRS 网络
  GSM_DELAY(100);
  GSM_tx_printf("AT+CIPCSGP=1,\"CMNET\"\r");                 //设置为 GPRS 连接模式 ，接入点为CMNET
	GSM_DELAY(100);
}

/**
  * 函数功能: GPRS功能：TCP通信协议配置
  * 输入参数: localport：本地端口
  *           serverip：远端服务器IP地址
  *           serverport：远端服务器端口
  * 返 回 值: GSM_TRUE：成功
  *           GSM_FALSE：失败
  * 说    明：无
  */
uint8_t GSM_gprs_tcp_link(char *localport,char * serverip,char * serverport)
{
  char *redata;
	GSM_tx_printf("AT+CLPORT=\"TCP\",\"%s\"\r",localport);         //获取本地端口
  GSM_DELAY(100);
  
	GSM_CLEAN_RX();
  //设置服务器IP和端口
  GSM_tx_printf("AT+CIPSTART=\"TCP\",\"%s\",\"%s\"\r",serverip,serverport);
	
	redata=GSM_waitask(0);
	GSM_CLEAN_RX();
	redata=GSM_waitask(0);
	if((strstr(redata,"OK") == NULL)&&(strstr(redata,"ALREADY")==NULL))
  {
    return GSM_FALSE;
  }
	return GSM_TRUE;
}

/**
  * 函数功能: GPRS功能：UDP通信协议配置
  * 输入参数: localport：本地端口
  *           serverip：远端服务器IP地址
  *           serverport：远端服务器端口
  * 返 回 值: GSM_TRUE：成功
  *           GSM_FALSE：失败
  * 说    明：无
  */
uint8_t GSM_gprs_udp_link(char *localport,char * serverip,char * serverport)
{
	char *redata;
  GSM_tx_printf("AT+CLPORT=\"UDP\",\"%s\"\r",localport);              //获取本地端口
  GSM_DELAY(100);
	
	GSM_CLEAN_RX();
  GSM_tx_printf("AT+CIPSTART=\"UDP\",\"%s\",\"%s\"\r",serverip,serverport);   //设置服务器IP和端口
	
	redata=GSM_waitask(0);
	GSM_CLEAN_RX();
	redata=GSM_waitask(0);
	
	if(strstr(redata,"OK") == NULL)
  {
    return GSM_FALSE;
  }
	return GSM_TRUE;
}

/**
  * 函数功能: GPRS功能：数据发送
  * 输入参数: str：数据缓冲区
  * 返 回 值: GSM_TRUE：成功
  *           GSM_FALSE：失败
  * 说    明：无
  */
uint8_t GSM_gprs_send(const char * str)
{
  char *redata;
	GSM_CLEAN_RX();
	GSM_tx_printf("AT+CIPSEND\r");
  //GSM_DELAY(100);	
  redata=GSM_waitask(0);
	
	GSM_CLEAN_RX();
  GSM_tx_printf("%s",str);
	while((GSM_USARTx->SR&0X40)==0);//等待上一次数据发送完成  
	GSM_USARTx->DR=(uint32_t)0x1A;		//发送十六进制数：0X1A,信息结束符号
	
	redata=GSM_waitask(0);
	if(strstr(redata,"OK")==NULL)
  {
    return GSM_FALSE;
  }
	return GSM_TRUE;
}

/**
  * 函数功能: GPRS功能：IP链接断开
  * 输入参数: 无
  * 返 回 值: GSM_TRUE：成功
  *           GSM_FALSE：失败
  * 说    明：无
  */
uint8_t GSM_gprs_link_close(void)
{
	GSM_CLEAN_RX();
	if(GSM_cmd("AT+CIPCLOSE=1\r","OK",200) != GSM_TRUE)
  {
    return GSM_FALSE;
  }
	return GSM_TRUE;
}

/**
  * 函数功能: GPRS功能：关闭场景
  * 输入参数: 无
  * 返 回 值: GSM_TRUE：成功
  *           GSM_FALSE：失败
  * 说    明：无
  */
uint8_t GSM_gprs_shut_close(void)
{
	GSM_CLEAN_RX();
	if(GSM_cmd("AT+CIPSHUT\r","OK",200) != GSM_TRUE)
  {
    return GSM_FALSE;
  }
	return GSM_TRUE;
}

/**
  * 函数功能: GPRS功能：接收数据
  * 输入参数: 无
  * 返 回 值: GSM_TRUE：成功
  *           GSM_FALSE：失败
  * 说    明：无
  */
uint8_t PostGPRS(void)
{
	char *redata;
  uint8_t len;
 
  redata = GSM_RX(len);   //接收数据 
  if(len == 0)
  {
    return GSM_FALSE;
  }
	printf("PostUDP:%s\n",redata);
	GSM_CLEAN_RX();
  return GSM_TRUE;
}
/**
*初始化IP应用
*/
void GSM_ip_init(void)
{
  GSM_tx_printf("AT+CGATT?\r");   
	GSM_DELAY(100);	
  GSM_tx_printf("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r");          
  GSM_DELAY(100);
  GSM_tx_printf("AT+SAPBR=3,1,\"APN\",\"CMNET\"\r");                            
  GSM_DELAY(100);
  GSM_tx_printf("AT+SAPBR=1,1\r");                 
	GSM_DELAY(100);
}

void GSM_http_init(void){
	GSM_tx_printf("AT+HTTPINIT\r");   
	GSM_DELAY(100);	
}

/**
*初始化HTTP
*/
char * GSM_http_get(char * serverip)
{
	char *redata;
  GSM_tx_printf("AT+HTTPPARA=\"URL\",\"%s\"\r",serverip);                            
  GSM_DELAY(100);
	GSM_CLEAN_RX();
	GSM_tx_printf("AT+HTTPACTION=0\r");    
  redata = Return_result();
	GSM_tx_printf("AT+HTTPREAD\r");   
	GSM_DELAY(100);
	char dst[10][100];
	int cnt = split(dst, redata, "\n");
	redata = dst[2];
	return redata;
}

char * GSM_http_post(char * serverip ,char * params)
{
	char *redata;
	
	//GSM_tx_printf("AT+HTTPSSL=1\r");                         
  //GSM_DELAY(100);
	
	GSM_tx_printf("AT+HTTPPARA=\"CID\",%d\r",1);                            
  GSM_DELAY(100);
	
  GSM_tx_printf("AT+HTTPPARA=\"URL\",\"%s\"\r",serverip);                            
  GSM_DELAY(100);
	
	GSM_tx_printf("AT+HTTPPARA=\"CONTENT\",\"%s\"\r","application/json");                            
  GSM_DELAY(100);

	GSM_tx_printf("AT+HTTPDATA=40,10000\r");   
	GSM_DELAY(100);
	
	GSM_tx_printf(params);
	GSM_DELAY(10000);
	
	GSM_CLEAN_RX();
	GSM_tx_printf("AT+HTTPACTION=1\r");    
  redata = Return_result();
	GSM_tx_printf("AT+HTTPREAD\r");   
	GSM_DELAY(100);
	char dst[10][100];
	int cnt = split(dst, redata, "\n");
	redata = dst[2];
	return redata;
}

/**
  * 函数功能:结束http
  * 输入参数: 无
  * 返 回 值: GSM_TRUE：成功
  *           GSM_FALSE：失败
  * 说    明：无
  */
void GSM_http_end(){
	GSM_tx_printf("AT+HTTPTERM\r");   
	GSM_DELAY(100);
}


/**
  * 函数功能:接收服务器信息
  * 输入参数: 无
  * 返 回 值: GSM_TRUE：成功
  *           GSM_FALSE：失败
  * 说    明：无
  */
char * Return_result(void)
{
	char *redata;
  uint8_t len;
	while(1){
		redata = GSM_RX(len); //接收数据 
		if((timecount>=50)&& (timestop!=0xFF))
		{			
			if(len > 20){
				timestop=0;
				GSM_DELAY(100);
				break;
			}
			else{
				timestop++;
			}
			timecount=0;
		}
		if(timestop==20)//10s
			{
				printf("超时跳出");
				GSM_DELAY(1000);
				timestop=0xFF;
				break;
			}
		timecount++;
		GSM_DELAY(10);
	}
	GSM_CLEAN_RX();
	return redata;
}

//
int split(char dst[][100], char* str, const char* spl)
{
    int n = 0;
    char *result = NULL;
    result = strtok(str, spl);
    while( result != NULL )
    {
        strcpy(dst[n++], result);
        result = strtok(NULL, spl);
    }
    return n;
}

