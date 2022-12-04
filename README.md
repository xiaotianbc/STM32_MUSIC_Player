# STM32_MUSIC_Player STM32播放音乐的例程

设备：

* `STM32F407ZGT6`单片机
* 单身道音频功放+喇叭

本程序同样也可以作为`STM32F407ZGT6` 的`CMake`/`MDK`的模板程序

## 1-TIM+DAC+DMA播放音乐

原理

1. 使用`Adobe Audition` 从MP3文件中生成16000Hz采样率，8bit的wav音频.
2. 使用Winhex将wav文件转换为c语言数组
3. 使用`TIM6`生成等于采样率的周期，生成TRGO输出事件
4. 初始化DMA+DAC，设置`DAC_Trigger = DAC_Trigger_T6_TRGO`由`TIM6`触发DAC转换，DMA把数据从数组搬运到DAC的`x位`/`xx对齐`
   数据保持寄存器里