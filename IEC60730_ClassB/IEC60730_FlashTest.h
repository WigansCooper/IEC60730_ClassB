#ifndef IEC60730_FLASHTEST_H
#define IEC60730_FLASHTEST_H

#include "IEC60730_Test.h"

/* 每次检测的步长，以字为单位 */
#define MCU_ROM_CHECK_STEP_SIZE         1

/* STM32F103C8T6 (64KB) 的最末尾 8 字节存储位置 */
#define MCU_FLASH_BASE                      (0x08000000u)      // STM32F103 Flash 起始地址
#define MCU_CHECK_LENGTH_STORE_ADDRESS      (0x0801FFF8u)        // 长度存储物理地址
#define MCU_CHECK_CRC_STORE_ADDRESS         (0x0801FFFCu)        // 期望的 CRC 校验码存储物理地址

#define MCU_CHECK_LENGTH                    (MCU_CHECK_LENGTH_STORE_ADDRESS -  MCU_FLASH_BASE)       // 检测长度


/* 外部函数声明 */
extern uint8_t IEC60730_FLASH_Test(void);

#endif


