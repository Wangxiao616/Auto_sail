#include "infrared.h"
#include "stm32f1xx_hal.h"

/* 传感器触发状态（索引0~8对应IRM_1~IRM_9） */
volatile uint8_t ir_trigger_status[IR_SENSOR_NUM] = {0};

/* 初始化红外传感器（初始化状态数组） */
void Infrared_Init(void)
{
    for(uint8_t i=0; i<IR_SENSOR_NUM; i++)
    {
        ir_trigger_status[i] = 0;
    }
}

/* 计算红外传感器加权和 */
int8_t Infrared_CalcWeightSum(void)
{
    int8_t sum = 0;
    for(uint8_t i=0; i<IR_SENSOR_NUM; i++)
    {
        if(ir_trigger_status[i] == 1)
        {
            sum += ir_weights[i];
        }
    }
    return sum;
}

/* 重置所有传感器触发状态 */
void Infrared_ResetTriggerStatus(void)
{
    for(uint8_t i=0; i<IR_SENSOR_NUM; i++)
    {
        ir_trigger_status[i] = 0;
    }
}

/* 设置单个传感器触发状态 */
void Infrared_SetTriggerStatus(uint8_t sensor_idx, uint8_t status)
{
    if(sensor_idx < IR_SENSOR_NUM)
    {
        ir_trigger_status[sensor_idx] = status;
    }
}