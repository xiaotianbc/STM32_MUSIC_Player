#ifndef _MCU_DAC_H_
#define _MCU_DAC_H_

extern s32 mcu_dac_init(void);
/**
 *@brief:      mcu_dac_open
 *@details:    ��DAC������
 *@param[in]   void  
 *@param[out]  ��
 *@retval:     
 */
extern s32 mcu_dac_open(void);
/**
 *@brief:      mcu_dac_output
 *@details:    ����DAC���ֵ
 *@param[in]   u16 vol�� ��ѹ����λMV��0-Vref  
 *@param[out]  ��
 *@retval:     
 */
extern s32 mcu_dac_output_vol(u16 vol);
/**
 *@brief:      mcu_dac_output
 *@details:    ��һ����ֵ��ΪDACֵ���
 *@param[in]   u16 data  
 *@param[out]  ��
 *@retval:     
 */
extern s32 mcu_dac_output(u16 data);
/**
 *@brief:      mcu_dac_test
 *@details:    DAC���Գ���
 *@param[in]   void  
 *@param[out]  ��
 *@retval:     
 */
extern s32 mcu_dac_test(void);

#endif

