#include "debug_log.h"

#include "stdio.h"
//#include "stdarg.h"
//#include "main.h"

/* use HAL lib */
#include "stm32f1xx_hal.h"

/* user defined */
__weak void user_uart_send(char *buff, unsigned int len)
{
	//send data.
}

__weak void send_to_wifi(uint8_t*buff, unsigned int len)
{
	// send data to wifi.
}

/* user log print */
int mylog(char *format, ...)
{
	char buffer[MAX_LOGBUFF_LEN];
	unsigned int len = 0;
	va_list args;
	va_start(args, format);
	len = vsnprintf(buffer, MAX_LOGBUFF_LEN, format, args);
	va_end(args);
	
	
	user_uart_send(buffer, len);
	
	return len;
}

void em_log_t(const int level, const char* fun, const int line, const char* file, const char* format, ...)
{
#ifdef OPEN_LOG
	if(level >= LOG_LEVEL)
	{
		char log_buffer[MAX_LOGBUFF_LEN];
		va_list args;
		va_start(args, format);

		vsnprintf(log_buffer, MAX_LOGBUFF_LEN, format, args);
		va_end(args);
		mylog("[%-7s] [%-20s%4d%20s] %s \r\n", log_level_get(level), fun, line, file, log_buffer);
	}
#endif
	
#ifndef	OPEN_LOG	
	mylog("\r\n --- May not open the log? ---\r\n");
#endif
}

char* log_level_get(const int level)
{
	switch(level)
	{
		case LOG_DEBUG:
			return "Debug";
		break;
		
		case LOG_INFO:
			return "Info";
		break;
		
		case LOG_WARN:
			return "Warning";
		break;
		
		case LOG_ERROR:
			return "Error";
		break;
		
		default: 
			return "Unknow";
	
	}
}

