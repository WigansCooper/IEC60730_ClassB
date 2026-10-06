#include "IEC60730_Test.h"
#include "IEC60730_FlashTest.h"

void IEC60730_Test_Init(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
}


void IEC60730_Test_Handler(void)
{
    if(IEC60730_FLASH_Test() != IEC60730_TEST_NORMAL)
    {
        NVIC_SystemReset();
    }
}
