#ifndef _WUJIQUE_LOG_H_
#define _WUJIQUE_LOG_H_

typedef enum
{
	LOG_DISABLE = 0,
	LOG_ERR,	//错误
	LOG_FUN,	//功能（用LOG输出算一个功能）
	LOG_INFO,	//信息，例如设备初始化等信息
	LOG_DEBUG,	//调试，正式程序通常屏蔽
}LOG_L;

extern void wjq_log(LOG_L l, s8 *fmt,...);
extern void PrintFormat(u8 *wbuf, s32 wlen);

#endif
