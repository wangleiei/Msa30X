#include "Msa30x.h"

                             
#define MSA_REG_SPI_I2C                 0x00
#define MSA_REG_WHO_AM_I                0x01

#define MSA_REG_ACC_X_LSB               0x02
#define MSA_REG_ACC_X_MSB               0x03
#define MSA_REG_ACC_Y_LSB               0x04
#define MSA_REG_ACC_Y_MSB               0x05
#define MSA_REG_ACC_Z_LSB               0x06
#define MSA_REG_ACC_Z_MSB               0x07
#define MSA_REG_ORIENTA_STA             0x0C

#define MSA_REG_MOTION_FLAG             0x09
#define MSA_REG_G_RANGE                 0x0f
#define MSA_REG_ODR_AXIS_DISABLE        0x10
#define MSA_REG_POWERMODE_BW            0x11
#define MSA_REG_SWAP_POLARITY           0x12
#define MSA_REG_FIFO_CTRL               0x14
#define MSA_REG_INTERRUPT_SETTINGS1     0x16
#define MSA_REG_INTERRUPT_SETTINGS2     0x17
#define MSA_REG_INTERRUPT_MAPPING1      0x19
#define MSA_REG_INTERRUPT_MAPPING2      0x1a
#define MSA_REG_INTERRUPT_MAPPING3      0x1b
#define MSA_REG_INT_PIN_CONFIG          0x20
#define MSA_REG_INT_LATCH               0x21
#define MSA_REG_ACTIVE_DURATION         0x27
#define MSA_REG_ACTIVE_THRESHOLD        0x28
#define MSA_REG_TAP_DURATION            0x2A
#define MSA_REG_TAP_THRESHOLD           0x2B
#define MSA_REG_CUSTOM_OFFSET_X         0x38
#define MSA_REG_CUSTOM_OFFSET_Y         0x39
#define MSA_REG_CUSTOM_OFFSET_Z         0x3a
#define MSA_REG_ENGINEERING_MODE        0x7f
#define MSA_REG_SENSITIVITY_TRIM_X      0x80
#define MSA_REG_SENSITIVITY_TRIM_Y      0x81
#define MSA_REG_SENSITIVITY_TRIM_Z      0x82
#define MSA_REG_COARSE_OFFSET_TRIM_X    0x83
#define MSA_REG_COARSE_OFFSET_TRIM_Y    0x84
#define MSA_REG_COARSE_OFFSET_TRIM_Z    0x85
#define MSA_REG_FINE_OFFSET_TRIM_X      0x86
#define MSA_REG_FINE_OFFSET_TRIM_Y      0x87
#define MSA_REG_FINE_OFFSET_TRIM_Z      0x88
#define MSA_REG_SENS_COMP               0x8c
#define MSA_REG_SENS_COARSE_TRIM        0xd1
static void Msa30x_Getdirection(MSA30X_BASE*base,uint8_t *uplook,uint8_t *dir);

void MSA30x_Init(MSA30X_BASE*base,
            MEASURING_RANGE measuring_range,
            uint32_t Bandwidth_HZ,
            POWER_MODE power_mode,
            OUTDATA_FORMATE outdata_formate,
            int (*read)(uint8_t addr,uint8_t* dataptr,uint8_t len),
            int (*write)(uint8_t addr,uint8_t data)){    
    uint8_t temp = 0;

    base->measuring_range = measuring_range;
    base->Bandwidth_HZ = Bandwidth_HZ;
    base->power_mode = power_mode;
    base->outdata_formate = outdata_formate;

    base->read = read;
    base->write = write;

    base->X_axis = 0;
    base->Y_axis = 0;
    base->Z_axis = 0;

    switch(outdata_formate){
        case BIT_14:
            temp |= 0x00;
        break;
        case BIT_12:
            temp |= 0x04;
        break;
        case BIT_10:
            temp |= 0x08;
        break;
        case BIT_8:
            temp |= 0x0c;
        break;
    }




    switch(measuring_range){
        case POSNEG_2G:
            temp |= 0x00;
            base->Resolution = 0.000244140625;
        break;
        case POSNEG_4G:
            temp |= 0x01;
            base->Resolution = 0.00048828125;
        break;
        case POSNEG_8G:
            temp |= 0x02;
            base->Resolution = 0.0009765625;
        break;
        case POSNEG_16G:
            temp |= 0x03;
            base->Resolution = 0.001953125;
        break;
    }
    base->write(MSA_REG_G_RANGE,temp);//控制采集精度和测量范围

    temp = 0;
    if(Bandwidth_HZ <= 10){
        temp |= 0x03;//7.81hz
    }else if(Bandwidth_HZ <= 120){
        temp |= 0x05;//31.25hz
    }else if(Bandwidth_HZ <= 300){
        temp |= 0x08;//250hz
    }else if(Bandwidth_HZ <= 500){
        temp |= 0x09;//500hz
    }else if(Bandwidth_HZ <= 1000){
        temp |= 0x0F;//1000hz 在此频率下，只能工作在正常模式
    }   
    base->write(MSA_REG_ODR_AXIS_DISABLE,temp);//正常模式下的带宽 

    temp = 0;
    switch(power_mode){
        case NORMAL_MODE:
            temp = 0;
        break;
        case SUSPEND_MODE:
            temp |= 0x80;
        break;
        case SLEEP_MODE:
            temp |= 0x40;
        break;
    }
    if(Bandwidth_HZ <= 10){
        temp |= 0x08;//7.81hz
    }else if(Bandwidth_HZ <= 120){
        temp |= 0x0c;//31.25hz
    }else if(Bandwidth_HZ <= 300){
        temp |= 0x12;//250hz
    }else if(Bandwidth_HZ <= 500){
        temp |= 0x14;//500hz
    }else{
        // 1.95hz
    }
    base->write(MSA_REG_POWERMODE_BW,temp);//工作模式和低功耗下的带宽

}
/**********************************************************************************************************
*    函 数 名: void EnterSleepMode(MSA30X_BASE*base,uint32_t Bandwidth_HZ)
*    功能说明: 进入低功耗模式，低功耗模式下数据带宽和正常功耗下带宽一样
*    传    参: @Bandwidth_HZ:小于1000HZ
*    返 回 值: 
*    说    明: 
*********************************************************************************************************/
void MSA30x_EnterSleepMode(MSA30X_BASE*base,uint32_t Bandwidth_HZ){
    // 
    uint8_t temp = 0;
    base->power_mode = SLEEP_MODE;

    temp = 0;
    if(Bandwidth_HZ <= 10){
        temp |= 0x03;//7.81hz
    }else if(Bandwidth_HZ <= 120){
        temp |= 0x05;//31.25hz
    }else if(Bandwidth_HZ <= 300){
        temp |= 0x08;//250hz
    }else {
        temp |= 0x09;//500hz
    }  
    base->write(MSA_REG_ODR_AXIS_DISABLE,temp);//正常模式下的带宽 


    temp = 0x40;//低功耗
    if(Bandwidth_HZ <= 10){
        temp |= 0x08;//7.81hz
    }else if(Bandwidth_HZ <= 120){
        temp |= 0x0c;//31.25hz
    }else if(Bandwidth_HZ <= 300){
        temp |= 0x12;//250hz
    }else {
        temp |= 0x14;//500hz
    }
    base->write(MSA_REG_POWERMODE_BW,temp);//工作模式和低功耗下的带宽
}
/**********************************************************************************************************
*    函 数 名: void MSA30x_InteruptCfig(MSA30X_BASE*base,INTERPIN_OUTMODE mode,INTERPIN_ACTIVE_LEVEL level,INTERBIT_LATCHTIME times)
*    功能说明: 配置中断管脚，输出模式，输出电平，以及寄存器中中断bit保持时间
*    传    参: 
*    返 回 值: 
*   说    明: 
*********************************************************************************************************/
void MSA30x_InteruptCfig(MSA30X_BASE*base,
                        INTERPIN_OUTMODE mode,
                        INTERPIN_ACTIVE_LEVEL level,
                        INTERBIT_LATCHTIME times){
    uint8_t temp = 0;

    if(OPEN_DRAIN == mode){
        temp |= 0x02;
    }
    if(HIGH == level){
        temp |= 0x01;
    }
    base->write(MSA_REG_INT_PIN_CONFIG,temp);//中断管脚输出模式配置，输出电平配置
    
    temp = 0;
    switch(times){
        case LATCH_1S:
            temp |= 0x03;
        break;
        case LATCH_8S:
            temp |= 0x06;
        break;
        case LATCHED:
            temp |= 0x07;
        break;
        case NON_LATCHED:
            temp |= 0x00;
        break;
        case LATCH_25MS:
            temp |= 0x0c;
        break;
        case LATCH_100MS:
            temp |= 0x0e;
        break;
    }
    base->write(MSA_REG_INT_LATCH,temp);//设置中断位保持时间
}
/**********************************************************************************************************
*    函 数 名: void MSA30X_ReadXYZ(MSA30X_BASE *base,uint16_t X_axis,uint16_t Y_axis,uint16_t Z_axis)
*    功能说明: 读取XYZ轴的加速度值
*    传    参: 
        @X_axis:X轴数据指针，单位G(m/s^2)
        @Y_axis:Y轴数据指针，单位G(m/s^2)
        @Z_axis:Z轴数据指针，单位G(m/s^2)
*    返 回 值: 
*   说    明: 
*********************************************************************************************************/
void MSA30X_ReadXYZ(MSA30X_BASE *base,float* X_axis,float* Y_axis,float* Z_axis){
    uint8_t temp[6];
    uint8_t uplook = 0;
    uint8_t right = 0;

    static uint8_t init =0;


    base->read(MSA_REG_ACC_X_LSB,temp,6);

    Msa30x_Getdirection(base,&uplook,&right);
    base->X_axis = ((temp[1]&0x7f) << 6 | temp[0]>>2);
    base->Y_axis = ((temp[3]&0x7f) << 6 | temp[2]>>2);
    base->Z_axis = ((temp[5]&0x7f) << 6 | temp[4]>>2);


    base->X_axis_temp = temp[1] *256 + temp[0];
    base->Y_axis_temp = temp[3] *256 + temp[2];
    base->Z_axis_temp = temp[5] *256 + temp[4];

    // if(0 == init){
    //     init = 1;
    //     base->write(MSA_REG_CUSTOM_OFFSET_X,base->X_axis);
    //     base->write(MSA_REG_CUSTOM_OFFSET_Y,base->Y_axis);
    //     base->write(MSA_REG_CUSTOM_OFFSET_Z,base->Z_axis);
    // }
    // base->X_axis &= base->outdata_formate;
    // base->Y_axis &= base->outdata_formate;
    // base->Z_axis &= base->outdata_formate;

    * X_axis = base->Resolution*(base->X_axis) * ((0x80&temp[1])?-1:1);
    * Y_axis = base->Resolution*(base->Y_axis) * ((0x80&temp[3])?-1:1);
    * Z_axis = base->Resolution*(base->Z_axis) * ((0x80&temp[5])?-1:1);
}
/**********************************************************************************************************
*    函 数 名: uint8_t MSA30x_Test(MSA30X_BASE*base)
*    功能说明: 测试MSA器件是否正常
*    传    参: 
*    返 回 值: 0:成功 1：失败
*   说    明: 
*********************************************************************************************************/
uint8_t MSA30x_Test(MSA30X_BASE*base){
    uint8_t temp[1];
    base->read(MSA_REG_WHO_AM_I,temp,1);
    if((0 == temp[0])||(0xff == temp[0])){
        return 1;
    }
    return 0;
}
/**********************************************************************************************************
*    函 数 名: static void Msa30x_Getdirection(MSA30X_BASE*base,uint8_t *uplook,uint8_t *dir)
*    功能说明: 得到传感器方向
*    传    参: 
        @uplook：器件朝向 0:upward looking, 1:downward looking
        @dir：器件方向
            0x00: portrait upright
            0x01: portrait upsidedown
            0x02: landscape left,
            0x03: landscape right        
*    返 回 值: 
*   说    明: 
*********************************************************************************************************/
static void Msa30x_Getdirection(MSA30X_BASE*base,uint8_t *uplook,uint8_t *dir){
    uint8_t temp[1];
    base->read(MSA_REG_ORIENTA_STA,temp,1);
    if(temp[0]){
        temp[0]++;
    }
    *uplook = (temp[0]&0x40)>>6;
    *dir = (temp[0]&0x30)>>5;
}
// void Msa30x_SetOffset(){

// }