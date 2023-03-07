#include <stdio.h>
#include "NuMicro.h"

volatile uint32_t g_u32AdcIntFlag;

void SYS_Init(void)
{
	SYS_UnlockReg();

	CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
	CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);
	CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));
	CLK->PCLKDIV = (CLK_PCLKDIV_APB0DIV_DIV2 | CLK_PCLKDIV_APB1DIV_DIV2);

	CLK_EnableModuleClock(ADC_MODULE);
	CLK_EnableModuleClock(TMR0_MODULE);
	CLK_EnableModuleClock(UART0_MODULE);

	CLK_SetModuleClock(ADC_MODULE, CLK_CLKSEL2_ADCSEL_PCLK1, CLK_CLKDIV0_ADC(1));
	CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, MODULE_NoMsk);
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

	SystemCoreClockUpdate();

	SYS->GPB_MFPL = SYS_GPB_MFPL_PB0MFP_ADC0_CH0;
	SYS->GPB_MFPL = SYS_GPB_MFPL_PB1MFP_ADC0_CH1;
	SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk)) |(SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);
	SYS->GPF_MFPL = SYS_GPF_MFPL_PF1MFP_ICE_CLK | SYS_GPF_MFPL_PF0MFP_ICE_DAT;

	SYS_LockReg();
}

void ADC_FunctionTest()
{
	int32_t  i32ConversionData,i32ConversionData2;
	ADC_POWER_ON(ADC);
 while(1)
	{
		ADC_Open(ADC, ADC_ADCR_DIFFEN_SINGLE_END, ADC_ADCR_ADMD_SINGLE_CYCLE, BIT0);
   ADC_Open(ADC, ADC_ADCR_DIFFEN_SINGLE_END, ADC_ADCR_ADMD_SINGLE_CYCLE, BIT1);
		ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);

		ADC_ENABLE_INT(ADC, ADC_ADF_INT);
		NVIC_EnableIRQ(ADC_IRQn);

		g_u32AdcIntFlag = 0;
		ADC_START_CONV(ADC);

		while(g_u32AdcIntFlag == 0);

		ADC_DISABLE_INT(ADC, ADC_ADF_INT);

		i32ConversionData = ADC_GET_CONVERSION_DATA(ADC, 0);
	  i32ConversionData2 = ADC_GET_CONVERSION_DATA(ADC, 1);
		printf("Conversion result of channel %d: 0x%X (%d)\n", 0, i32ConversionData, i32ConversionData);
		printf("Conversion result of channel %d: 0x%X (%d)\n", 1, i32ConversionData2, i32ConversionData2);
		TIMER_Delay(TIMER0, 1000000);
	}
}

void ADC_IRQHandler(void)
{
	g_u32AdcIntFlag = 1;
	ADC_CLR_INT_FLAG(ADC, ADC_ADF_INT);
}

void UART0_Init(void)
{
	//SYS_ResetModule(UART0_RST);
	UART_Open(UART0, 115200);
}

int32_t main(void)
{
	SYS_Init();
//GPIO_SetMode(PB, BIT0, GPIO_MODE_INPUT);
	UART0_Init();
	ADC_FunctionTest();

	while(1);
}
