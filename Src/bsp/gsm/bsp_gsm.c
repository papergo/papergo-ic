/**
  ******************************************************************************
  * �ļ�����: GSM.c 
  * ��    ��: ӲʯǶ��ʽ�����Ŷ�
  * ��    ��: V1.0
  * ��д����: 2015-10-04
  * ��    ��: GSMģ��ײ�����ʵ��
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
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "usart/bsp_debug_usart.h"
#include "usart/bsp_usartx.h"
#include "GSM/bsp_GSM.h"
/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
/* ˽�б��� ------------------------------------------------------------------*/
static uint8_t MaxMessAdd=50;
uint8_t aRxBuffer;
static uint8_t timecount=1,timestop=0;

/* ��չ���� ------------------------------------------------------------------*/
extern uint16_t ff_convert(uint16_t	src,uint32_t	dir);

/* ˽�к���ԭ�� --------------------------------------------------------------*/
/* ������ --------------------------------------------------------------------*/

/**
  * ��������: ��GSM����һ���������֤����ֵ�Ƿ���ȷ
  * �������: cmd������
  *           replay���ڴ�����ֵ
  *           waittime���ȴ�ʱ��
  * �� �� ֵ: GSM_TRUE���ɹ�
  *           GSM_FALSE��ʧ��
  * ˵    ������
  */
uint8_t GSM_cmd(char *cmd, char *reply,uint32_t waittime )
{    
  GSM_CLEAN_RX();                 //����˽��ջ���������
  GSM_TX(cmd);                    //��������
  if(reply == 0)                      //����Ҫ��������
  {
    return GSM_TRUE;
  }
  GSM_DELAY(waittime);                 //��ʱ  
  return GSM_cmd_check(reply);    //�Խ������ݽ��д���
}

/**
  * ��������: ��֤����ķ���ֵ�Ƿ���ȷ
  * �������: replay���ڴ�����ֵ
  * �� �� ֵ: GSM_TRUE���ɹ�
  *           GSM_FALSE��ʧ��
  * ˵    ������
  */
uint8_t GSM_cmd_check(char *reply)
{
  uint8_t len;
  uint8_t n;
  uint8_t off;
  char *redata;
  
  redata = GSM_RX(len); //��������
  n = 0;
  off = 0;
  while((n + off)<len)
  {
    if(reply[n] == 0) //����Ϊ�ջ��߱Ƚ����
    {
      return GSM_TRUE;
    }
    if(redata[ n + off]== reply[n])
    {
      n++; //�ƶ�����һ����������
    }
    else
    {
      off++; //������һ��ƥ��
      n=0; //����
    }
    //n++;
  }
  if(reply[n]==0) //�պ�ƥ�����
  {
     return GSM_TRUE;
  }  
  return GSM_FALSE; //����ѭ����ʾ�Ƚ���Ϻ�û����ͬ�����ݣ��������
}

/**
  * ��������: �ȴ�GSM������Ӧ��
  * �������: ��
  * �� �� ֵ: Ӧ�����ݴ�
  * ˵    ������
  */
char * GSM_waitask(uint8_t waitask_hook(void))
{
  uint8_t len=0;
  char *redata;
  do
  {    
    redata = GSM_RX(len);   //��������
    if(waitask_hook!=0)
    {
      if(waitask_hook()==GSM_TRUE) //���� GSM_TRUE ��ʾ��⵽�¼�����Ҫ�˳�
      {
        redata = 0;
        return redata;               
      }
    }
  }while(len==0);                 //��������Ϊ0ʱһֱ�ȴ�  
  GSM_DELAY(20);              //��ʱ��ȷ���ܽ��յ�ȫ�����ݣ�115200�������£�ÿms�ܽ���11.52���ֽڣ�
	printf("PostUDP:%s\n",redata);
  return redata;
}

/**
  * ��������: ��ȡ��������
  * �������: num����ź���Ļ�����
  * �� �� ֵ: GSM_TRUE���ɹ�
  *           GSM_FALSE��ʧ��
  * ˵    ������
  */
uint8_t GSM_cnum(char *num)
{
  char *redata;
  uint8_t len;
  
  if(GSM_cmd("AT+CNUM\r","OK", 100) != GSM_TRUE)
  {
    return GSM_FALSE;
  }  
  redata = GSM_RX(len);   //��������
  if(len == 0)
  {
    return GSM_FALSE;
  }  
  //��һ�����ź��������Ϊ:"��������"
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
  *num = 0;               //�ַ�����β��Ҫ��0
  return GSM_TRUE;
}

/**
  * ��������: ��ʼ�������ģ��
  * �������: ��
  * �� �� ֵ: GSM_TRUE���ɹ�
  *           GSM_FALSE��ʧ��
  * ˵    ������
  */
uint8_t GSM_init(void)
{
	char *redata;
	uint8_t len;
	
	GSM_CLEAN_RX(); //����˽��ջ���������
	GSM_config(); //��ʼ������
	
  GSM_tx_printf("ATQ0\r");
  GSM_DELAY(200);
  HAL_UART_Receive_IT(&husartx_GSM,&aRxBuffer,1);
	if(GSM_cmd("ATE0\r","OK", 200) != GSM_TRUE)
	{    
		return GSM_FALSE;
	}  
  GSM_CLEAN_RX(); //����˽��ջ���������
	if(GSM_cmd("AT+CGMM\r","OK", 200) != GSM_TRUE)
	{
	  return GSM_FALSE;
	}	
	redata = GSM_RX(len);   //��������
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

/*-------------------------   �绰����  ---------------------------------*/
/**
  * ��������: ���𲦴�绰�����ܽӲ���ͨ��
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
void GSM_call(char *num)
{
  GSM_CLEAN_RX(); //����˽��ջ���������
  GSM_tx_printf("ATD%s;\r",num);
  //������ǲ��������ݵ�
  //�����κ�Ӧ�𣬿�ͷ����"\r\n",��������Ч�������Ǵӵ�3���ֽڿ�ʼ
  //�Է��ҵ��绰��û��ͨ�������أ�BUSY
  //�Է������˵绰��+COLP: "555",129,"",0,"9648674F98DE"   OK
  //�Է������˵绰,Ȼ��ҵ���NO CARRIER
  //�Է���ʱû�ӵ绰��NO ANSWER
}

/**
  * ��������: ������绰
  * �������: num��������뻺����
  * �� �� ֵ: GSM_TRUE���ɹ�
  *           GSM_FALSE��ʧ��
  * ˵    ������
  */
uint8_t IsRing(char *num)
{
  char *redata;
  uint8_t len;
  
  if(GSM_cmd_check("RING"))
  {
    return GSM_FALSE;
  }  
  redata = GSM_RX(len);   //��������
  if(len == 0)
  {
    return GSM_FALSE;
  }  
  //��һ�����ź��������Ϊ:�����롰
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
  *num = 0;               //�ַ�����β��Ҫ��0
  GSM_CLEAN_RX();
  return GSM_TRUE;
}

/*-------------------------   ���Ź���  ---------------------------------*/
/**
  * ��������: �ж��ַ���ֻ��ASCLL��û������
  * �������: str���ַ���������
  * �� �� ֵ: GSM_TRUE���ɹ�
  *           GSM_FALSE��ʧ��
  * ˵    ������
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
  * ��������: GBK����תunicode����
  * �������: ucs2��Unicode���뻺����
  *           gbk��GBK���뻺����
  * �� �� ֵ: ��
  * ˵    ������
  */
void GSM_gbk2ucs2(char *ucs2,char *gbk)
{
  uint16_t   tmp;  
  while(*gbk)
  {
    if((*gbk&0xFF) < 0x7F)      //Ӣ��
    {        
      *ucs2++ = 0;
      *ucs2++ = *gbk++;  
    }
    else                        //����
    {
      tmp = GSM_SWAP16(*(uint16_t *)gbk);
      *(uint16_t *)ucs2 = GSM_SWAP16(ff_convert(tmp,1));
      gbk+=2;
      ucs2+=2;
    }
  }
}

/**
  * ��������: �����ַ�ת16�����ַ���
  * �������: hex��16�����ַ���������
  *           ch�������ַ�
  * �� �� ֵ: ��
  * ˵    ������
  */
void GSM_char2hex(char *hex,char ch)
{
  const char numhex[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  *hex++  = numhex[(ch & 0xF0)>>4];
  *hex    = numhex[ ch & 0x0F];
}

/**
  * ��������: GBK����תunicode����16�����ַ���
  * �������: ucs2��Unicode����16�����ַ���������
  *           gbk��GBK���뻺����
  * �� �� ֵ: ��
  * ˵    ������
  */
void GSM_gbk2ucs2hex(char * ucs2hex,char * gbk)
{
  uint16_t   tmp;
  
  while(*gbk)
  {
    if((*gbk&0xFF) < 0x7F)      //Ӣ��
    {        
      *ucs2hex++ = '0';
      *ucs2hex++ = '0';
      GSM_char2hex(ucs2hex,*gbk);
      ucs2hex+=2;
      gbk++;  
    }
    else                        //����
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
  * ��������: ���Ͷ��ţ�֧����Ӣ��,����ΪGBK�룩
  * �������: num���Է��ֻ�����
  *           smstext����������
  * �� �� ֵ: ��
  * ˵    ������
  */
void GSM_sms(char *num,char *smstext)
{
  char ucsbuff[160];
  char end[2] = {0x1A,0x00};

  GSM_CLEAN_RX();                 //����˽��ջ���������
  if(IsASSIC(smstext)==GSM_TRUE)
  {
    //Ӣ��
    GSM_tx_printf("AT+CSCS=\"GSM\"\r");     //"GSM"�ַ���
    GSM_DELAY(100);    
    GSM_tx_printf("AT+CMGF=1\r");           //�ı�ģʽ
    GSM_DELAY(100);    
    GSM_tx_printf("AT+CMGS=\"%s\"\r",num);  //�绰����
    GSM_DELAY(100);
    GSM_tx_printf("%s",smstext);            //��������        
  }
  else
  {
      //����
      GSM_tx_printf("AT+CSCS=\"UCS2\"\r");    //"UCS2"�ַ���
      GSM_DELAY(100);      
      GSM_tx_printf("AT+CMGF=1\r");           //�ı�ģʽ
      GSM_DELAY(100);      
      GSM_tx_printf("AT+CSMP=17,167,0,8\r");  //
      GSM_DELAY(100);      
      GSM_gbk2ucs2hex(ucsbuff,num);
      GSM_tx_printf("AT+CMGS=\"%s\"\r",ucsbuff);  //UCS2�ĵ绰����(��Ҫת�� ucs2��)
      GSM_DELAY(100);      
      GSM_gbk2ucs2hex(ucsbuff,smstext);
      GSM_tx_printf("%s",ucsbuff);          //UCS2���ı�����(��Ҫת�� ucs2��)
  }
  GSM_DELAY(1); 
  // GSM_tx_printf("%c",0x1A);
  GSM_tx_printf("%s",end);
	//while((USART2->SR&0X40)==0);//�ȴ���һ�����ݷ������  
	//GSM_USARTx->DR=(u32)0x1A;		//����ʮ����������0X1A,��Ϣ��������
}

/**
  * ��������: ��ѯ�Ƿ���յ��¶���
  * �������: ��
  * �� �� ֵ: 0:���¶���
  *           ��0���¶��ŵ�ַ
  * ˵    ������
  */
uint8_t IsReceiveMS(void)
{
	char address[3]={0};
	uint8_t addressnum=0;
	char *redata;
  uint8_t len;
/*------------- ��ѯ�Ƿ����¶��Ų���ȡ�洢λ�� ----------------------------*/
  if(GSM_cmd_check("+CMTI:"))
  {
    return 0;
  }   
  redata = GSM_RX(len);   //��������
	//printf("CMTI:redata:%s,len=%d\n",redata,len);
  if(len == 0)
  {
    return 0;
  }    
  //��һ�����ź��������Ϊ:�����Ŵ洢��ַ��
  while(*redata != ',')
  {
    len--;
    if(len==0)
    {
        return 0;
    }
    redata++;
  }
	redata+=1;//ȥ����','
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
  * ��������: ��ȡ��������
  * �������: messadd�����ŵ�ַ
  *        		num�����淢���˺���(unicode�����ʽ���ַ���)
  *     			str�������������(unicode�����ʽ���ַ���)
  * �� �� ֵ: 0��ʧ��
  * 	        1���ɹ���ȡ�����ţ��ö���δ�����˴��ǵ�һ�ζ����������Զ�����׼Ϊ�Ѷ���
  *      			2���ɹ���ȡ�����ţ��ö����Ѷ�
  * ˵    ������
  */
uint8_t readmessage(uint8_t messadd,char *num,char *str)
{
	char *redata,cmd[20]={0};
    uint8_t len;
	char result=0;
	GSM_CLEAN_RX();                 //����˽��ջ���������
	if(messadd>MaxMessAdd)return 0;
	
/*------------- ��ȡ�������� ----------------------------*/
	sprintf(cmd,"AT+CMGR=%d\r",messadd);	
	if(GSM_cmd(cmd,"+CMGR:",500) != GSM_TRUE)
	{
		return 0;
	}
	redata = GSM_RX(len);   //��������
	if(len == 0)
	{
		return 0;
	}
	printf("CMGR:redata:%s\nlen=%d\n",redata,len);
	if(strstr(redata,"UNREAD")==0)result=2;
	else	result=1;
	//��һ�����ź��������Ϊ:�������˺��롰
	while(*redata != ',')
	{
		len--;
		if(len==0)
		{
			return 0;
		}
		redata++;
	}
	redata+=2;//ȥ��',"'
	while(*redata != '"')
	{
		*num++ = *redata++;
		len--;
	}
	*num = 0;               //�ַ�����β��Ҫ��0
	
	while(*redata != '+')
	{
		len--;
		if(len==0)
		{
			return 0;
		}
		redata++;
	}
	redata+=6;//ȥ��'+32"\r'
	while(*redata != '\r')
	{
		*str++ = *redata++;
	}
	*str = 0;               //�ַ�����β��Ҫ��0
	printf("CMGR:result:%d\n",result);
	return result;
}

/**
  * ��������: unicode����16�����ַ���תGBK����
  * �������: hexuni��Unicode����16�����ַ���������
  *           chgbk��GBK���뻺����
  * �� �� ֵ: GSM_TRUE���ɹ�
  *           GSM_FALSE��ʧ��
  * ˵    ������
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

/*--------------------------   GPRS ����   ---------------------------------*/
/**
  * ��������: GPRS���ܻ�����ʼ��
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
void GSM_gprs_init(void)
{
  GSM_tx_printf("AT+CGCLASS=\"B\"\r");                       //�����ƶ�̨���Ϊ"B"
  GSM_DELAY(100);
  GSM_tx_printf("AT+CGDCONT=1,\"IP\",\"CMNET\"\r");          //PDP�����ı�ʶ1��������Э�飬��������ƣ�CMNET
  GSM_DELAY(100);
  GSM_tx_printf("AT+CGATT=1\r");                             //���� GPRS ����
  GSM_DELAY(100);
  GSM_tx_printf("AT+CIPCSGP=1,\"CMNET\"\r");                 //����Ϊ GPRS ����ģʽ �������ΪCMNET
	GSM_DELAY(100);
}

/**
  * ��������: GPRS���ܣ�TCPͨ��Э������
  * �������: localport�����ض˿�
  *           serverip��Զ�˷�����IP��ַ
  *           serverport��Զ�˷������˿�
  * �� �� ֵ: GSM_TRUE���ɹ�
  *           GSM_FALSE��ʧ��
  * ˵    ������
  */
uint8_t GSM_gprs_tcp_link(char *localport,char * serverip,char * serverport)
{
  char *redata;
	GSM_tx_printf("AT+CLPORT=\"TCP\",\"%s\"\r",localport);         //��ȡ���ض˿�
  GSM_DELAY(100);
  
	GSM_CLEAN_RX();
  //���÷�����IP�Ͷ˿�
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
  * ��������: GPRS���ܣ�UDPͨ��Э������
  * �������: localport�����ض˿�
  *           serverip��Զ�˷�����IP��ַ
  *           serverport��Զ�˷������˿�
  * �� �� ֵ: GSM_TRUE���ɹ�
  *           GSM_FALSE��ʧ��
  * ˵    ������
  */
uint8_t GSM_gprs_udp_link(char *localport,char * serverip,char * serverport)
{
	char *redata;
  GSM_tx_printf("AT+CLPORT=\"UDP\",\"%s\"\r",localport);              //��ȡ���ض˿�
  GSM_DELAY(100);
	
	GSM_CLEAN_RX();
  GSM_tx_printf("AT+CIPSTART=\"UDP\",\"%s\",\"%s\"\r",serverip,serverport);   //���÷�����IP�Ͷ˿�
	
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
  * ��������: GPRS���ܣ����ݷ���
  * �������: str�����ݻ�����
  * �� �� ֵ: GSM_TRUE���ɹ�
  *           GSM_FALSE��ʧ��
  * ˵    ������
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
	while((GSM_USARTx->SR&0X40)==0);//�ȴ���һ�����ݷ������  
	GSM_USARTx->DR=(uint32_t)0x1A;		//����ʮ����������0X1A,��Ϣ��������
	
	redata=GSM_waitask(0);
	if(strstr(redata,"OK")==NULL)
  {
    return GSM_FALSE;
  }
	return GSM_TRUE;
}

/**
  * ��������: GPRS���ܣ�IP���ӶϿ�
  * �������: ��
  * �� �� ֵ: GSM_TRUE���ɹ�
  *           GSM_FALSE��ʧ��
  * ˵    ������
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
  * ��������: GPRS���ܣ��رճ���
  * �������: ��
  * �� �� ֵ: GSM_TRUE���ɹ�
  *           GSM_FALSE��ʧ��
  * ˵    ������
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
  * ��������: GPRS���ܣ���������
  * �������: ��
  * �� �� ֵ: GSM_TRUE���ɹ�
  *           GSM_FALSE��ʧ��
  * ˵    ������
  */
uint8_t PostGPRS(void)
{
	char *redata;
  uint8_t len;
 
  redata = GSM_RX(len);   //�������� 
  if(len == 0)
  {
    return GSM_FALSE;
  }
	printf("PostUDP:%s\n",redata);
	GSM_CLEAN_RX();
  return GSM_TRUE;
}
/**
*��ʼ��IPӦ��
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
*��ʼ��HTTP
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
  * ��������:����http
  * �������: ��
  * �� �� ֵ: GSM_TRUE���ɹ�
  *           GSM_FALSE��ʧ��
  * ˵    ������
  */
void GSM_http_end(){
	GSM_tx_printf("AT+HTTPTERM\r");   
	GSM_DELAY(100);
}


/**
  * ��������:���շ�������Ϣ
  * �������: ��
  * �� �� ֵ: GSM_TRUE���ɹ�
  *           GSM_FALSE��ʧ��
  * ˵    ������
  */
char * Return_result(void)
{
	char *redata;
  uint8_t len;
	while(1){
		redata = GSM_RX(len); //�������� 
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
				printf("��ʱ����");
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

