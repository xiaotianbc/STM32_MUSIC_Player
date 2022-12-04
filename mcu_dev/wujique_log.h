#ifndef _WUJIQUE_LOG_H_
#define _WUJIQUE_LOG_H_

typedef enum
{
	LOG_DISABLE = 0,
	LOG_ERR,	//����
	LOG_FUN,	//���ܣ���LOG�����һ�����ܣ�
	LOG_INFO,	//��Ϣ�������豸��ʼ������Ϣ
	LOG_DEBUG,	//���ԣ���ʽ����ͨ������
}LOG_L;

extern void wjq_log(LOG_L l, s8 *fmt,...);
extern void PrintFormat(u8 *wbuf, s32 wlen);

#endif
