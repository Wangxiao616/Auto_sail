#ifndef INFRARED_H
#define INFRARED_H

#include <stdint.h>

/* 红外传感器数量 */
#define IR_SENSOR_NUM 9

/* 红外传感器权重配置
   IRM_1~IRM_9 呈圆环排列，5在正前方，2在正右方，8在正左方 */

static const int8_t ir_weights[IR_SENSOR_NUM] = {
    0,   // IRM_1 → 后方，不参与控制，否则船会识别到后面的红外信号
    4,   // IRM_2 → 最右侧（船偏左 → 最大力度右转）
    3,   // IRM_3 → 右侧（船偏左 → 大力度右转）
    2,   // IRM_4 → 右前（船偏左 → 中力度右转）
    0,   // IRM_5 → 正前方（直行，无修正）
   -2,   // IRM_6 → 左前（船偏右 → 中力度左转）
   -3,   // IRM_7 → 左侧（船偏右 → 大力度左转）
   -4,   // IRM_8 → 最左侧（船偏右 → 最大力度左转）
    0    // IRM_9 → 后方，不参与控制
};

/* 遮蔽模式：用于反射抗性，在S型航线中禁用错误方向的传感器 */
typedef enum {
    IR_MASK_NONE  = 0,  /* 不遮蔽，全部传感器生效 */
    IR_MASK_LEFT  = 1,  /* 遮蔽左侧传感器（IRM_6/7/8），用于向右转向的阶段 */
    IR_MASK_RIGHT = 2   /* 遮蔽右侧传感器（IRM_2/3/4），用于向左转向的阶段 */
} IR_MaskSide;

/* 传感器触发状态（0：未触发，1：触发） */
extern volatile uint8_t ir_trigger_status[IR_SENSOR_NUM];

/* 初始化红外传感器 */
void Infrared_Init(void);

/* 计算红外传感器加权和（受遮蔽影响的传感器不参与计算） */
int8_t Infrared_CalcWeightSum(void);

/* 重置所有传感器触发状态 */
void Infrared_ResetTriggerStatus(void);

/* 设置单个传感器触发状态（受遮蔽影响的传感器将被忽略） */
void Infrared_SetTriggerStatus(uint8_t sensor_idx, uint8_t status);

/* 设置遮蔽模式 */
void Infrared_SetMaskSide(IR_MaskSide side);

/* 获取当前遮蔽模式 */
IR_MaskSide Infrared_GetMaskSide(void);

/* 判断指定传感器是否当前被遮蔽 */
uint8_t Infrared_IsSensorMasked(uint8_t idx);

#endif