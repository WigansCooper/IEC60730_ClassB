#include "IEC60730_FlashTest.h"


/* 运行期全局变量定义 */
volatile uint32_t FlashStartAddr = 0;
volatile uint32_t FlashTestResult = 0;

volatile const uint32_t *ROM_Length_Ptr = (volatile const uint32_t *)MCU_CHECK_LENGTH_STORE_ADDRESS;
volatile const uint32_t *ROM_CRC_Ptr = (volatile const uint32_t *)MCU_CHECK_CRC_STORE_ADDRESS;

/* 
 * 适配 Keil MDK 绝对地址放置变量方法（存放于 Flash 最末尾） 
 * 占位用，实际发布固件时，需要通过后处理工具（如 HexView）重新复写这两个位置的值
 */
// volatile const uint32_t uROM_Length __attribute__((section(".ARM.__AT_0x0801FFF8"))) __attribute__((used)) = 0x0001FFF8u; 
// volatile const uint32_t usROM_CRC __attribute__((section(".ARM.__AT_0x0801FFFC"))) __attribute__((used)) = 0x814DCFBCu;   


/**
 * @brief  STM32F103 硬件CRC外设自检代码 (满足 IEC 60730 自身故障检测要求)
 * @return 0: 硬件正常; 1: 硬件故障
 */
static uint8_t IEC60730_CRC_Hardware_SelfTest(void)
{
    // STM32F1 固定的 0x04C11DB7 多项式下，顺序喂入这两个 32-bit 数据产生的标准硬件校验和精确为 0x7D24A31Bu;
    const uint32_t test_data_1 = 0x12345678u;
    const uint32_t test_data_2 = 0x9ABCDEF0u;
    const uint32_t expected_crc = 0x7D24A31Bu;
    uint32_t computed_crc = 0;

    // 1. 标准库 API: 复位硬件 CRC 发生器 (清空 Data Register 并初始化为 0xFFFFFFFF)
    CRC_ResetDR();

    // 2. 标准库 API: 连续喂入 32 位黄金特征码数据
    CRC_CalcCRC(test_data_1);              // 喂入第一个字
    computed_crc = CRC_CalcCRC(test_data_2); // 喂入第二个字并返回当前结果

    // 3. 再次复位，清除自检现场残余数据，为接下来的真实 Flash 测试腾出空间
    CRC_ResetDR();

    // 4. 判断硬件运算、复位和读写逻辑是否完全正常
    if (computed_crc != expected_crc)
    {
        return IEC60730_TEST_FAIL; // 硬件自身损坏，向主系统报警
    }

    return IEC60730_TEST_NORMAL;
}


/**
 * @brief  专用于 STM32F103C8T6 主循环调用的 Flash 分时（Time-sliced）安全扫描测试
 * @return 0: 测试正常或处于分时扫描中; 1: 发生安全故障（CRC错/外设坏）
 */
uint8_t IEC60730_FLASH_Test(void)
{
    uint32_t Index = 0;
    uint32_t *Data_Ptr;
    uint32_t primask;
    
    // 【边界前置拦截】每次进入函数，第一步无条件检查长度配置是否完好！
    // 这样哪怕长度在运行期、或者上电时就坏了，程序能立刻感知，绝不冒险去读位置未知的指针
    if ((*ROM_Length_Ptr) != MCU_CHECK_LENGTH)
    {
        return IEC60730_TEST_FAIL; 
    }

    // 1. 地址 4 字节对齐，单周期原子级汇编位与对齐
    FlashStartAddr &= ~0x03u;
    Data_Ptr = (uint32_t *)(MCU_FLASH_BASE + FlashStartAddr);

    while (Index < MCU_ROM_CHECK_STEP_SIZE)
    {
        // 2. 临界区开始：仅包裹单次自检或单次喂数逻辑
        primask = __get_PRIMASK();
        __disable_irq();

        // 周期性功能自检：当每一次重新开始整片 Flash 的首包扫描时（即从 0 地址开始时）
        if (FlashStartAddr == 0)
        {
            if (IEC60730_CRC_Hardware_SelfTest() != IEC60730_TEST_NORMAL)
            {
                __set_PRIMASK(primask); // 开启中断，安全闭锁
                return IEC60730_TEST_FAIL;
            }
        }

        // 执行单次 32-bit 喂数（单次耗时约 0.27微秒）
        FlashTestResult = CRC_CalcCRC(*(Data_Ptr+Index)); 
        Index++;
        FlashStartAddr += 4;

        // 3. 触底边界判断：扫描完全部定义的有效 ROM 空间（对齐 64KB 上限 0x0801FFF8）
        if (FlashStartAddr >= *ROM_Length_Ptr || FlashStartAddr > MCU_CHECK_LENGTH) 
        {
            FlashStartAddr = 0;
            FlashTestResult = CRC_GetCRC(); 
            CRC_ResetDR(); 

            if (FlashTestResult != *ROM_CRC_Ptr) 
            {
                __set_PRIMASK(primask);
                return IEC60730_TEST_FAIL; // 检测到 Flash 数据损坏
            }
        }

        // 4. 临界区结束：立刻开启中断
        __set_PRIMASK(primask); 
    }

    return IEC60730_TEST_NORMAL;
}

















