#include "main.h"

// 初始化独立看门狗：预分频64，重装载值4095（约1秒超时）
int32_t iwdg_init(void) {
    // 启动独立看门狗（内部LSI时钟自动开启，无需手动配置）
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  // 允许写入IWDG寄存器（密钥操作）
    IWDG_SetPrescaler(IWDG_Prescaler_4);          // 预分频4（40kHz/4 ≈ 10kHz）
    IWDG_SetReload(1000);                          // 重装载值=4095（计数器从4095递减到0）
    IWDG_ReloadCounter();                          // 立即重载计数器（避免初始化后立刻复位）
    IWDG_Enable();                                 // 启动IWDG
	
	return 0;
}

void iwdg_reload(void){
	
	IWDG_ReloadCounter();
}

int main(void) {
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    iwdg_init();
    
    for(;;)
    {
        iwdg_reload();
    }
}

