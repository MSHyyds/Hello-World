#include "ir.h"
#include "string.h"


extern int mylog(char *format, ...);

/* user log print */
// int mylog(char *format, ...)
// {
// 	char buffer[MAX_LOGBUFF_LEN];
// 	unsigned int len = 0;
// 	va_list args;
// 	va_start(args, format);
// 	len = vsnprintf(buffer, MAX_LOGBUFF_LEN, format, args);
// 	va_end(args);
	
	
// 	user_uart_send(buffer, len);
	
// 	return len;
// }

IR_INFO_T ir_info[IR_HEAD_NUM];


/* infrared init */
void ir_init(void)
{
	HAL_TIM_Base_Start_IT(&IR_TIM);               //
	HAL_TIM_IC_Start_IT(&IR_TIM,TIM_CHANNEL_1);   //
	HAL_TIM_IC_Start_IT(&IR_TIM,TIM_CHANNEL_2);
	HAL_TIM_IC_Start_IT(&IR_TIM,TIM_CHANNEL_3);
	HAL_TIM_IC_Start_IT(&IR_TIM,TIM_CHANNEL_4);
}

/* infrared timer over interrupt service request */
void ir_tim_over_isr(TIM_HandleTypeDef *tim)
{
	if(tim->Instance == IR_TIM.Instance)
	{
		switch(tim->Channel)
		{
			case HAL_TIM_ACTIVE_CHANNEL_1:
				ir_info[0].over_cnt++;
				break;
			
			case HAL_TIM_ACTIVE_CHANNEL_2:
				ir_info[1].over_cnt++;
				break;

			case HAL_TIM_ACTIVE_CHANNEL_3:
				ir_info[2].over_cnt++;
				break;

			case HAL_TIM_ACTIVE_CHANNEL_4:
				ir_info[3].over_cnt++;
				break;
		}
	}	
}





/* infrared timer input capture interrupt service request */
void ir_tim_ic_isr(TIM_HandleTypeDef *tim)
{
	uint8_t index;
	if(tim->Instance == IR_TIM.Instance)
	{
		switch(tim->Channel)
		{
			case HAL_TIM_ACTIVE_CHANNEL_1:
				index = 0;
				ir_info[index].IR_TIM_CHANNEL = TIM_CHANNEL_1;
				break;
			
			case HAL_TIM_ACTIVE_CHANNEL_2:
				index = 1;
				ir_info[index].IR_TIM_CHANNEL = TIM_CHANNEL_2;
				break;

			case HAL_TIM_ACTIVE_CHANNEL_3:
				index = 2;
				ir_info[index].IR_TIM_CHANNEL = TIM_CHANNEL_3;
				break;

			case HAL_TIM_ACTIVE_CHANNEL_4:
				index = 3;
				ir_info[index].IR_TIM_CHANNEL = TIM_CHANNEL_4;
				break;
		}
		
		if(ir_info[index].level_flag) // 上升沿捕获
		{
			ir_info[index].up_val = HAL_TIM_ReadCapturedValue(tim ,ir_info[index].IR_TIM_CHANNEL);
			
			__HAL_TIM_SET_CAPTUREPOLARITY(tim,ir_info[index].IR_TIM_CHANNEL,TIM_INPUTCHANNELPOLARITY_FALLING);
			ir_info[index].over_cnt = 0;
			ir_info[index].level_flag=0;
		}
		else  // 下降沿捕获
		{
			ir_info[index].down_val = HAL_TIM_ReadCapturedValue(tim ,ir_info[index].IR_TIM_CHANNEL);
			__HAL_TIM_SET_CAPTUREPOLARITY(tim,ir_info[index].IR_TIM_CHANNEL,TIM_INPUTCHANNELPOLARITY_RISING);
			ir_info[index].level_flag = 1;
			ir_info[index].pluse_width = ir_info[index].down_val + ir_info[index].over_cnt * tim->Init.Period - ir_info[index].up_val;
			// mylog("[%d]\r\n",ir_info[index].pluse_width);
			if((ir_info[index].pluse_width >= (IR_us_REPEAT_START - IR_US_DEVIATION) && ir_info[index].pluse_width <= (IR_us_REPEAT_START + IR_US_DEVIATION)) && ir_info[index].repet_flag == 1)
			{
				ir_info[index].repet_flag = 2;
			}
			if(ir_info[index].pluse_width >= (IR_us_REPEAT_END - IR_US_DEVIATION) && ir_info[index].pluse_width <= (IR_us_REPEAT_START + IR_us_REPEAT_END) && ir_info[index].repet_flag == 2)
			{
				ir_info[index].repet_flag = 1;
				ir_info[index].rcv_flag = 2;
			}
			if(ir_info[index].pluse_width >= (IR_US_BOOT_1 - IR_US_DEVIATION) && ir_info[index].pluse_width <= (IR_US_BOOT_1 + IR_US_DEVIATION))
			{
				ir_info[index].bit_cnt=0;
				ir_info[index].bit_buff[ir_info[index].bit_cnt++]=ir_info[index].pluse_width;
			}
			else if(ir_info[index].bit_cnt>0)
			{
				ir_info[index].bit_buff[ir_info[index].bit_cnt++]=ir_info[index].pluse_width;
				if(ir_info[index].bit_cnt>32)
				{
					ir_info[index].rcv_flag=1;
					ir_info[index].bit_cnt = 0;
					ir_info[index].repet_flag = 1;
				}
			}
		}
		
	}
}


/* infrared val read */
/**
 * @brief 读取红外接收头解码后的数值，共4个字节（地址码，地址反码，用户码，用户反码）
 * 
 * @param cnt 接收头的数量
 * @return uint8_t 
 */
uint8_t ir_read(uint8_t (*arr)[4], uint8_t cnt)
{
	uint8_t  i;
	uint8_t index;
	
	for(index=0;index<4;index++)
	{
		if(ir_info[index].rcv_flag ==  1)
		{
			
			for(i=0;i<32;i++)
			{
				ir_info[index].change_data[i/8] >>= 1;
				if(ir_info[index].bit_buff[i+1] >1000)
				{
					ir_info[index].change_data[i/8] |= 0x80;
				}
			}

			for(i=0;i<cnt;i++)
			{
				ir_info[index].save_data[i] = ir_info[index].change_data[i];
				arr[index][i] = ir_info[index].save_data[i];
			}


			/* 判断帧格式 */
			// if(ir_info[index].change_data[0] == (uint8_t)~ir_info[index].change_data[1] && ir_info[index].change_data[2] == (uint8_t)~ir_info[index].change_data[3])
			// {
			// 	mylog("perfect:  %02x %02x %02x %02x\r\n",ir_info[index].change_data[0],ir_info[index].change_data[1],ir_info[index].change_data[2],ir_info[index].change_data[3]);
			// 	ir_info[index].rcv_flag = 0;
			// 	for(i=0;i<4;i++)
			// 	{
			// 		ir_info[index].save_data[i] = ir_info[index].change_data[i];
			// 	}
			// 	memset(ir_info[index].change_data, 0, 4);
			// 	return 0;
			// }
			// if(ir_info[index].change_data[0] == (uint8_t)~ir_info[index].change_data[1] ||  ir_info[index].change_data[2] == (uint8_t)~ir_info[index].change_data[3])
			// {
			//     mylog("half perfect:  %02x %02x %02x %02x\r\n",ir_info[index].change_data[0],ir_info[index].change_data[1],ir_info[index].change_data[2],ir_info[index].change_data[3]);
			// 	ir_info[index].rcv_flag = 0;
			// 	for(i=0;i<4;i++)
			// 	{
			// 		ir_info[index].save_data[i] = ir_info[index].change_data[i];
			// 	}
			// 	memset(ir_info[index].change_data, 0, 4);
			// 	return 0;
			// }
			
			// mylog("\r\n");
			// memset(ir_info[index].save_data, 0, 4);
			
		}
		if(ir_info[index].rcv_flag == 2)
		{
			// mylog("repeat:  %02x %02x %02x %02x\r\n",ir_info[index].save_data[0],ir_info[index].save_data[1],ir_info[index].save_data[2],ir_info[index].save_data[3]);
			for(i=0;i<cnt;i++)
			{
				arr[index][i] = ir_info[index].save_data[i];
			}
		}

		memset(ir_info[index].change_data, 0, 4);
		memset(ir_info[index].bit_buff, 0, 128);
		ir_info[index].rcv_flag = 0;
	}

	return 1;
}




