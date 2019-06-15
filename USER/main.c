#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "stm32f4xx_dma.h"

/*
0：高电平大概占据625ns， 
1：高电平大概占据1250ns， 
一个bit周期大概为1.67us。 
（有一些误差是可以接受的）*/

#define ESC_CMD_BUFFER_LEN 18 
#define ESC_BIT_0     26
#define ESC_BIT_1     52


void Clock_Config(void){
     ErrorStatus        State;
     uint32_t           PLL_M;      
     uint32_t           PLL_N;
     uint32_t           PLL_P;
     uint32_t           PLL_Q;
 

    /*配置前将所有RCC重置为初始值*/

     RCC_DeInit();


     /*这里选择 外部晶振（HSE）作为 时钟源，因此首先打开外部晶振*/

     RCC_HSEConfig(RCC_HSE_ON);

     /*等待外部晶振进入稳定状态*/

     while( RCC_WaitForHSEStartUp() != SUCCESS );

     /*

     **我们要选择PLL时钟作为系统时钟，因此这里先要对PLL时钟进行配置

     */

     /*选择外部晶振作为PLL的时钟源*/


     /* 到这一步为止，已有 HSE_VALUE = 8 MHz.

        PLL_VCO input clock = (HSE_VALUE or HSI_VALUE / PLL_M)，

        根据文档，这个值被建议在 1~2MHz，因此我们令 PLL_M = 8，

        即 PLL_VCO input clock = 1MHz */

     PLL_M         =    8;  

     /* 到这一步为止，已有 PLL_VCO input clock = 1 MHz.

        PLL_VCO output clock = (PLL_VCO input clock) * PLL_N,

        这个值要用来计算系统时钟，我们 令 PLL_N = 336,

        即 PLL_VCO output clock = 336 MHz.*/       

     PLL_N        =    336;


     /* 到这一步为止，已有 PLL_VCO output clock = 336 MHz.

        System Clock = (PLL_VCO output clock)/PLL_P ,

        因为我们要 SystemClock = 168 Mhz，因此令 PLL_P = 2.

        */

     PLL_P         =    2;


     /*这个系数用来配置SD卡读写，USB等功能，暂时不用，根据文档，暂时先设为7*/

     PLL_Q         =    7;


     /* 配置PLL并将其使能，获得 168Mhz 的 System Clock 时钟*/

     RCC_PLLConfig(RCC_PLLSource_HSE, PLL_M, PLL_N, PLL_P, PLL_Q);

     RCC_PLLCmd(ENABLE);


     /*到了这一步，我们已经配置好了PLL时钟。下面我们配置Syetem Clock*/

     /*选择PLL时钟作为系统时钟源*/

     RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

     /*到了这一步，我们已经配置好了系统时钟，频率为 168MHz. 下面我们可以对 AHB，APB，外设等的 时钟进行配置*/

     /*时钟的结构请参考用户手册*/

 

     /*首先配置 AHB时钟（HCLK）. 为了获得较高的频率，我们对 SYSCLK 1分频，得到HCLK*/

     RCC_HCLKConfig(RCC_HCLK_Div1);

 

     /*APBx时钟（PCLK）由AHB时钟（HCLK）分频得到，下面我们配置 PCLK*/

 

     /*APB1时钟配置. 4分频，即 PCLK1 = 42 MHz*/

     RCC_PCLK1Config(RCC_HCLK_Div2);

     /*APB2时钟配置. 2分频，即 PCLK2 = 84 MHz*/

     RCC_PCLK2Config(RCC_HCLK_Div2);
}
void MYDMA_Config(DMA_Stream_TypeDef *DMA_Streamx,u32 chx,u32 par,u32 mar,u16 ndtr)
{ 
	DMA_InitTypeDef  DMA_InitStructure;
	if((u32)DMA_Streamx>(u32)DMA2) 
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE); 
					
	}else 
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE); 
	}
	DMA_DeInit(DMA_Streamx);

	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){} 

  DMA_InitStructure.DMA_Channel = chx;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = par; 
  DMA_InitStructure.DMA_Memory0BaseAddr = mar; 
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral; 
  DMA_InitStructure.DMA_BufferSize = ndtr; 
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; 
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; 
  DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Word; 
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; 
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; 
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single; 
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; 
  DMA_Init(DMA_Streamx, &DMA_InitStructure); 
	
	TIM_DMACmd(TIM2, TIM_DMA_CC1, ENABLE); 
	DMA_Cmd(DMA_Streamx, DISABLE); 
} 


void TIM2_PWM_Init(u32 arr,u32 psc)
{                                                          
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);           
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);          
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_TIM2);  
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;            
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;         
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;         
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;       
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;         
	GPIO_Init(GPIOA,&GPIO_InitStructure);               
		
	TIM_TimeBaseStructure.TIM_Prescaler=psc;   
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;  
	TIM_TimeBaseStructure.TIM_Period=arr;    
	TIM_TimeBaseStructure.TIM_ClockDivision=0; 
	
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure); 
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;  
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;  
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;  
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OC1Init(TIM2, &TIM_OCInitStructure);   

	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);   

	TIM_ARRPreloadConfig(TIM2,ENABLE); 
	
	TIM_Cmd(TIM2, ENABLE);   
} 





uint32_t dd[ESC_CMD_BUFFER_LEN]={0}; 


u16 add_checksum_and_telemetry(u16 packet) {
    u16 packet_telemetry = (packet << 1) | 0;
    u8 i;
    int csum = 0;
    int csum_data = packet_telemetry;

    for (i = 0; i < 3; i++) {
        csum ^=  csum_data;   // xor data by nibbles
        csum_data >>= 4;
    }
    csum &= 0xf;
		packet_telemetry = (packet_telemetry << 4) | csum;
		
    return packet_telemetry;    //append checksum
}
void pwmWriteDigital(uint32_t *esc_cmd, u16 value)
{
    value = ( (value > 2047) ? 2047 : value );
    value = add_checksum_and_telemetry(value);
    esc_cmd[0]  = (value & 0x8000) ? ESC_BIT_1 : ESC_BIT_0;
    esc_cmd[1]  = (value & 0x4000) ? ESC_BIT_1 : ESC_BIT_0;
    esc_cmd[2]  = (value & 0x2000) ? ESC_BIT_1 : ESC_BIT_0;
    esc_cmd[3]  = (value & 0x1000) ? ESC_BIT_1 : ESC_BIT_0;
    esc_cmd[4]  = (value & 0x0800) ? ESC_BIT_1 : ESC_BIT_0;
    esc_cmd[5]  = (value & 0x0400) ? ESC_BIT_1 : ESC_BIT_0;
    esc_cmd[6]  = (value & 0x0200) ? ESC_BIT_1 : ESC_BIT_0;
    esc_cmd[7]  = (value & 0x0100) ? ESC_BIT_1 : ESC_BIT_0;
    esc_cmd[8]  = (value & 0x0080) ? ESC_BIT_1 : ESC_BIT_0;
    esc_cmd[9]  = (value & 0x0040) ? ESC_BIT_1 : ESC_BIT_0;
    esc_cmd[10] = (value & 0x0020) ? ESC_BIT_1 : ESC_BIT_0;
    esc_cmd[11] = (value & 0x0010) ? ESC_BIT_1 : ESC_BIT_0;     
    esc_cmd[12] = (value & 0x8) ? ESC_BIT_1 : ESC_BIT_0;
    esc_cmd[13] = (value & 0x4) ? ESC_BIT_1 : ESC_BIT_0;
    esc_cmd[14] = (value & 0x2) ? ESC_BIT_1 : ESC_BIT_0;
    esc_cmd[15] = (value & 0x1) ? ESC_BIT_1 : ESC_BIT_0;
}

int main(void)
{ 
	u16 moto_value =0;
	u8 b = 1;
	Clock_Config();
	delay_init(168);                                   
	TIM2_PWM_Init(70-1,4-1);   
	//pwmWriteDigital(dd, 2000);
	MYDMA_Config(DMA1_Stream5,DMA_Channel_3,(uint32_t)&(TIM2->CCR1),(uint32_t)dd,ESC_CMD_BUFFER_LEN);
	delay_ms(1000);//解锁电调
	delay_ms(1000);//解锁电调
	while(1)
	{         
		 while (DMA_GetCmdStatus(DMA1_Stream5) != DISABLE){}
		 DMA_SetCurrDataCounter(DMA1_Stream5,ESC_CMD_BUFFER_LEN);      
		 DMA_Cmd(DMA1_Stream5, ENABLE);                        
		 TIM_Cmd(TIM2, ENABLE);                      
		 while(1)
		 {
			if(DMA_GetFlagStatus(DMA1_Stream5,DMA_FLAG_TCIF5)!=RESET)
			{ 
				TIM_Cmd(TIM2, DISABLE);        
				DMA_Cmd(DMA1_Stream5, DISABLE);                                                        

				DMA_ClearFlag(DMA1_Stream5,DMA_FLAG_TCIF5);
				break; 
			}  
		 }
		delay_ms(10);
		pwmWriteDigital(dd, moto_value);
		if(b == 1)
		{
			moto_value += 1;
		}
		else
		{
			moto_value -= 1;
		}
		
		if(moto_value > 2047) 
		{
			b = 0;
		}
		if(moto_value == 0) 
		{
			b = 1;
		}
			
	}
}