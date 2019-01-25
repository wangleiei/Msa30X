#ifndef __MSA30X
#define __MSA30X 

// typedef enum {}HZ;
#include "stm8l15x.h"

// 支持I2C接口
typedef enum {POSNEG_2G,POSNEG_4G,POSNEG_8G,POSNEG_16G}MEASURING_RANGE;//测量范围
typedef enum {NORMAL_MODE,SUSPEND_MODE,SLEEP_MODE}POWER_MODE;//工作模式
typedef enum {BIT_8,BIT_10,BIT_12,BIT_14}OUTDATA_FORMATE;//输出数据格式

typedef enum {PUSH_PULL,OPEN_DRAIN}INTERPIN_OUTMODE;//表示输出模式是推挽输出，还是开漏输出
typedef enum {LOW, HIGH}INTERPIN_ACTIVE_LEVEL;//中断时，管脚输出电平
typedef enum {LATCH_1S,LATCH_8S,LATCHED,NON_LATCHED,LATCH_25MS,LATCH_100MS}INTERBIT_LATCHTIME;//中断时，中断bit保持时间

typedef struct 
{
    MEASURING_RANGE measuring_range;   
    uint32_t Bandwidth_HZ;//带宽，数据输出速率一半,单位HZ
    POWER_MODE power_mode;
    OUTDATA_FORMATE outdata_formate;
/**********************************************************************************************************
*    函 数 名: int (*read)(uint8_t* buf,uint8_t len);
*    功能说明: 可以从设备中读取数据的函数指针
*    传    参: @addr 寄存器地址
               @dataptr 数据指针
               @len 数据长度
*    返 回 值: 1：失败，0：成功
*********************************************************************************************************/
    int (*read)(uint8_t addr,uint8_t* dataptr,uint8_t len);
/**********************************************************************************************************
*    函 数 名: int (*write)(uint8_t addr,uint8_t data);
*    功能说明: 可以往设备中写入数据的函数指针
*    传    参: @addr 寄存器地址
               @data 数据
*    返 回 值: 1：失败，0：成功
*********************************************************************************************************/    
    int (*write)(uint8_t addr,uint8_t data);
}MSA30X_BASE;

void MSA30x_Init(MSA30X_BASE*base,
            MEASURING_RANGE measuring_range,
            uint32_t Bandwidth_HZ,
            POWER_MODE power_mode,
            OUTDATA_FORMATE outdata_formate,
            int (*read)(uint8_t addr,uint8_t* dataptr,uint8_t len),
            int (*write)(uint8_t addr,uint8_t data));    

void MSA30x_EnterSleepMode(MSA30X_BASE*base,uint32_t Bandwidth_HZ);

void MSA30x_InteruptCfig(MSA30X_BASE*base,
                        INTERPIN_OUTMODE mode,
                        INTERPIN_ACTIVE_LEVEL level,
                        INTERBIT_LATCHTIME times);

void MSA30X_ReadXYZ(MSA30X_BASE *base,uint16_t* X_axis,uint16_t* Y_axis,uint16_t* Z_axis);

uint8_t MSA30x_Test(MSA30X_BASE*base);

#endif
       