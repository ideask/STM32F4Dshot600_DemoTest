#include "pwm.h"
#include "led.h"
#include "usart.h"
 

u8 ESC_CMD[ESC_CMD_BUFFER_LEN]={0};

void TIM1_PWM_Init(void)
{		 					 
	
	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);  	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 	
	
	
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1); 
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_TIM1);
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_11;       
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       
	GPIO_Init(GPIOA,&GPIO_InitStructure);             
	
	TIM_TimeBaseStructure.TIM_Prescaler=3;  
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; 
	TIM_TimeBaseStructure.TIM_Period=34;   
	TIM_TimeBaseStructure.TIM_ClockDivision=0; 
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);
 
 
 
 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 
 
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
	TIM_OCInitStructure.TIM_Pulse = 0; 
	TIM_OC1Init(TIM1, &TIM_OCInitStructure); 
	
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);
	
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  
	TIM_ARRPreloadConfig(TIM1,ENABLE);
	TIM_DMACmd( TIM1, TIM_DMA_CC1, ENABLE );
	TIM_Cmd(TIM1, ENABLE); 
				  
}  


void tim1_dma_CH1(void)
{
	DMA_InitTypeDef    DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;        
	
	/**************DMAÅäÖÃ*****************/
	DMA_DeInit(DMA2_Stream1);
	while (DMA_GetCmdStatus(DMA2_Stream1) != DISABLE){}
	DMA_InitStructure.DMA_Channel=DMA_Channel_6;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&TIM1->CCR1;
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32)&ESC_CMD;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize =ESC_CMD_BUFFER_LEN ;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; 
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority =DMA_Priority_High; 
		
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream1, &DMA_InitStructure);
     
  DMA_DoubleBufferModeConfig(DMA2_Stream1,(uint32_t)&ESC_CMD[0],DMA_Memory_0);
  DMA_DoubleBufferModeCmd(DMA2_Stream1,ENABLE);                      
}
