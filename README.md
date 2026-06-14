# AUTO_SAIL
## 文件结构
```
    AUTO_SAIL
    |——Core 片上外设代码（由CubeMX生成）
    |——Drivers 芯片核心驱动代码
    |——MDK-ARM
    |   |——AUTO_SAIL.uvprojx 项目工程文件，可用keil打开
    |   |——AUTO_SAIL/AUTO_SAIL.hex 编译后生成此二进制文件，用于烧录
    |——User
    |   |——DRV8833 电机驱动
    |   |——SG90 舵机驱动
    |——AUTO_SAIL.ioc CubeMX工程文件
```
## 环境配置
    STM32CubeMX 6.13.0
    MDK-ARM 5.41.0
    STM32 ST-Link Utility 4.6.0.0

## 工具
    ST-Link Debugger
    CH340串口模块

## 设计思路

当IRM接收到红外信号时触发中断，调用对应中断的中断回调函数`HAL_GPIO_EXIT_Callback`，可在其中根据赛道情况进行对应处理
舵机驱动函数和电机驱动函数均已封装好，对应接口位于***User/DRV8833***和***User/SG90***中

## 烧录

烧录使用 **STM32 ST-Link Utility**软件，将编译好的***MDK-ARM/AUTO_SAIL/AUTO_SAIL.hex***打开并烧录到核心板上

## 调试

在底板上有预留出***UART2***针脚，供调试时通过串口输出信息

## 测试

***TEST***文件夹为红外板测试用例，如果遇到问题可以先测试下是否接收到红外信号时串口回传对应的红外管号码。其中***IRM_IT***为中断实现的测试用例，也即本工程所采用的实现方式，***IRM_SCAN***为未被采用的使用持续扫描法进行实现。如果中断法发现有无法解决的bug，也可能采用这种方法进行重构
