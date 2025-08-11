#ifndef _DEBUG_LOG_H
#define _DEBUG_LOG_H
#include "stdarg.h"
#include "stm32f1xx_hal.h"

#define MAX_LOGBUFF_LEN 	256
#define LOG_LEVEL		LOG_DEBUG 
#define OPEN_LOG



enum log_level_list{
	LOG_DEBUG = 0,			// 调试输出debug信息
	LOG_INFO,				// 输出进程信息
	LOG_WARN,				// 输出警告信息
	LOG_ERROR,				// 输出错误信息
	
};



void user_uart_send(char *buff, unsigned int len);
void send_to_wifi(uint8_t*buff, unsigned int len);
int mylog(char *format, ...);
char* log_level_get(const int level);
void em_log_t(const int level, const char* fun, const int line, const char* file, const char* format, ...);



#define emlog(level, format, ...) em_log_t(level, __FUNCTION__, __LINE__,__FILE__, format, ##__VA_ARGS__)

#endif



