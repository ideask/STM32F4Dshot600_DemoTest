#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"


#define ESC_CMD_BUFFER_LEN 18 
void TIM1_PWM_Init(void);
void tim1_dma_CH1(void);
extern u8 ESC_CMD[ESC_CMD_BUFFER_LEN];
#endif
