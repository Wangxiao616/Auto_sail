#include "infrared.h"
#include "stm32f1xx_hal.h"

/* 传感器触发状态（索引0~8对应IRM_1~IRM_9） */
volatile uint8_t ir_trigger_status[IR_SENSOR_NUM] = {0};

/* 当前遮蔽模式 */
static IR_MaskSide current_mask = IR_MASK_NONE;

/*
 * 各遮蔽模式对应的传感器索引列表
 * IR_MASK_LEFT:  遮蔽 IRM_6(5), IRM_7(6), IRM_8(7) — 左侧
 * IR_MASK_RIGHT: 遮蔽 IRM_2(1), IRM_3(2), IRM_4(3) — 右侧
 */
static const uint8_t mask_left_sensors[]  = {5, 6, 7};
static const uint8_t mask_right_sensors[] = {1, 2, 3};
#define MASK_SENSOR_COUNT 3

/* 初始化红外传感器（初始化状态数组） */
void Infrared_Init(void)
{
    current_mask = IR_MASK_NONE;
    for(uint8_t i=0; i<IR_SENSOR_NUM; i++)
    {
        ir_trigger_status[i] = 0;
    }
}

/* 判断指定传感器是否当前被遮蔽 */
uint8_t Infrared_IsSensorMasked(uint8_t idx)
{
    if (current_mask == IR_MASK_NONE) return 0;

    const uint8_t *mask_list = (current_mask == IR_MASK_LEFT)
                               ? mask_left_sensors
                               : mask_right_sensors;

    for (uint8_t i = 0; i < MASK_SENSOR_COUNT; i++) {
        if (idx == mask_list[i]) return 1;
    }
    return 0;
}

/* 计算红外传感器加权和（被遮蔽的传感器不参与计算） */
int8_t Infrared_CalcWeightSum(void)
{
    int8_t sum = 0;
    for(uint8_t i=0; i<IR_SENSOR_NUM; i++)
    {
        if(ir_trigger_status[i] == 1 && !Infrared_IsSensorMasked(i))
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

/* 设置单个传感器触发状态（被遮蔽的传感器将被忽略） */
void Infrared_SetTriggerStatus(uint8_t sensor_idx, uint8_t status)
{
    if(sensor_idx < IR_SENSOR_NUM && !Infrared_IsSensorMasked(sensor_idx))
    {
        ir_trigger_status[sensor_idx] = status;
    }
}

/* 设置遮蔽模式 */
void Infrared_SetMaskSide(IR_MaskSide side)
{
    current_mask = side;
}

/* 获取当前遮蔽模式 */
IR_MaskSide Infrared_GetMaskSide(void)
{
    return current_mask;
}