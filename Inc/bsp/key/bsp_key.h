#ifndef __BSP_KEY_H__
#define __BSP_KEY_H__

/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* ���Ͷ��� --------------------------------------------------------------*/
typedef enum
{
  KEY_UP   = 0,
  KEY_DOWN = 1,
}KEYState_TypeDef;

/* �궨�� --------------------------------------------------------------------*/
#define KEY1_RCC_CLK_ENABLE           __HAL_RCC_GPIOC_CLK_ENABLE
#define KEY1_GPIO_PIN                 GPIO_PIN_13
#define KEY1_GPIO                     GPIOC
#define KEY1_DOWN_LEVEL               1  /* ����ԭ��ͼ��ƣ�KEY1����ʱ����Ϊ�ߵ�ƽ��������������Ϊ1 */

#define KEY2_RCC_CLK_ENABLE           __HAL_RCC_GPIOC_CLK_ENABLE
#define KEY2_GPIO_PIN                 GPIO_PIN_14
#define KEY2_GPIO                     GPIOC
#define KEY2_DOWN_LEVEL               1  /* ����ԭ��ͼ��ƣ�KEY1����ʱ����Ϊ�͵�ƽ��������������Ϊ1 */

#define KEY3_RCC_CLK_ENABLE           __HAL_RCC_GPIOC_CLK_ENABLE
#define KEY3_GPIO_PIN                 GPIO_PIN_15
#define KEY3_GPIO                     GPIOC
#define KEY3_DOWN_LEVEL               1  /* ����ԭ��ͼ��ƣ�KEY1����ʱ����Ϊ�͵�ƽ��������������Ϊ1 */

/* ��չ���� ------------------------------------------------------------------*/
/* �������� ------------------------------------------------------------------*/
void KEY_GPIO_Init(void);
KEYState_TypeDef KEY1_StateRead(void);
KEYState_TypeDef KEY2_StateRead(void);
KEYState_TypeDef KEY3_StateRead(void);
#endif  // __BSP_KEY_H__

/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/
