#include "tm1628a.h"
#include "debug_log.h"

const uint8_t digit_map[14] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F, // 9
    0x80, // .
    0x79, // E
    0x48, // =
    0xFF, // all
};

void delay_700ns(void)
{
    for (uint8_t i = 0; i < 50; i++)
    {
        __NOP(); // 每个 __NOP() 大约 13.9ns
    }
}

int get_decimal_position(double num) {
    if (num < 0) num = -num;  // 处理负数

    int int_part = (int)num;  // 提取整数部分
    int count = 0;

    // 计算整数部分的位数
    do {
        int_part /= 10;
        count++;
    } while (int_part > 0);

    return count + 1;  // 小数点位置 = 整数位数 + 1
}

/**
 * @brief 写入 tm1628a 一个字节数据
 * 
 * @param data 要写入的数据
 */
void tm1628a_write_byte(uint8_t data)
{
    uint8_t i; 
    
    for(i=0;i<8;i++)
    {
        
        if(data & 0x01)
        {
            TM1628A_DIO_HIGH();
        }
        else
        {
            TM1628A_DIO_LOW();
        }
        TM1628A_CLK_LOW();
        // HAL_Delay(1);
        // delay_700ns();
        TM1628A_CLK_HIGH();
        // HAL_Delay(1);
        // delay_700ns();
        data >>= 1;

    }
   
}

/**
 * @brief 向 tm1628a 发送命令
 * 
 * @param cmd   要发送的命令
 * @return * void 
 */
void tm1628a_send_cmd(uint8_t cmd)
{
    TM1628A_STB_LOW();
    tm1628a_write_byte(cmd);
    TM1628A_STB_HIGH();
}

/**
 * @brief 向 tm1628a 的一个地址写入一个数据
 * 
 * @param addr 要写入数据的地址
 * @param data 要写入的数据
 */
void tm1628a_write_data(uint8_t addr, uint8_t data)
{
    TM1628A_STB_LOW();
    tm1628a_write_byte(ADDR_FIXED);
    TM1628A_STB_HIGH();

    TM1628A_STB_LOW();
    tm1628a_write_byte(addr);
    tm1628a_write_byte(data);
    TM1628A_STB_HIGH();

}

/**
 * @brief 数码管初始化
 * 
 * @param mode 1，测试显示；0，只清零不测试显示
 */
void tm1628a_init(uint8_t mode)
{
    uint8_t i,j;

    TM1628A_STB_HIGH();
    TM1628A_CLK_HIGH();
    TM1628A_DIO_HIGH();

    tm1628a_send_cmd(BIT7_10P);     // 7位10段
    tm1628a_send_cmd(ADDR_FIXED);   // 固定地址模式
    
    if(mode)
    {
        for(j=0;j<14;j++)
        {
            for(i=0;i<14;i++)
            {
                TM1628A_STB_LOW();
                tm1628a_write_byte(CXH(i));
                if(i%2==0)tm1628a_write_byte(digit_map[j]);
                else tm1628a_write_byte(CLK_PT);
                // else tm1628a_write_byte(NONE);
                TM1628A_STB_HIGH();
            }
            HAL_Delay(500);
        }
    }
   
   // clear
    for(i=0;i<14;i++)
    {
        TM1628A_STB_LOW();
        tm1628a_write_byte(CXH(i));
        tm1628a_write_byte(NONE);
        TM1628A_STB_HIGH();
    }
    TM1628A_STB_HIGH();
    tm1628a_send_cmd(SHOW_ON | PULSE_WIDTH_14_16);
}

/**
 * @brief 数码管清零操作
 * 
 */
void tm1628a_clear_op()
{
    uint8_t i;

    for(i=0;i<14;i++)
    {
        TM1628A_STB_LOW();
        tm1628a_write_byte(CXH(i));
        tm1628a_write_byte(NONE);
        TM1628A_STB_HIGH();
    }
    TM1628A_STB_HIGH();
}


/**
 * @brief 数码管显示0~9999数字,采用固定地址模式
 * 
 * @param num       要显示的数字，0~9999；其他不显示
 * @param dp_pos    小数点的位置，1~4；其他不显示
 * @param delay_ms  闪烁延时
 * @param mode      模式选择，1，常显；0，频闪
 */
void tm1628a_show_count(uint16_t num, uint8_t dp_pos, uint16_t delay_ms, uint8_t mode)
{
    uint8_t i,j;
    uint8_t dt[4] = {0,};
    uint8_t break_flag=0;
    uint8_t step = 0;

    if(num < 0 || num > 9999)return;

    dt[3] = digit_map[num % 10];  // 个位
    dt[2] = digit_map[(num / 10) % 10]; // 十位
    dt[1] = digit_map[(num / 100) % 10]; // 百位
    dt[0] = digit_map[(num / 1000) % 10]; // 千位

    // for(i=0;i<4;i++)
    // emlog(LOG_DEBUG, "dt[%d]=%02x \r\n",i,dt[i]);
    // 处理小数点位置
    // dp_pos = get_decimal_position(num);
    if (dp_pos > 0 && dp_pos <= 4) {
        dt[dp_pos-1] = dt[dp_pos-1] | POINT;
    }
    // emlog(LOG_DEBUG, "dt[dp_pos-1]=%02x \r\n",dt[dp_pos-1]);
    // clear
    // tm1628a_clear_op();
    while(break_flag<=2)
    {
        switch(step)
        {
            case 0:
            for(i=0;i<14;i++)
            {
                TM1628A_STB_LOW();
                tm1628a_write_byte(CXH(i));
                if(i%2==0)                      // 数字控制
                {
                    if(dt[0] == ZERO && dt[1] == ZERO && dt[2] == ZERO)
                    {
                        if(i==6)tm1628a_write_byte(dt[3]);
                        else tm1628a_write_byte(NONE);
                    }
                    else if(dt[0] == ZERO && dt[1] == ZERO)
                    {
                        if(i==4)tm1628a_write_byte(dt[2]);
                        else if(i==6)tm1628a_write_byte(dt[3]);
                        else tm1628a_write_byte(NONE);
                    }
                    else if(dt[0] == ZERO)
                    {
                        if(i==2)tm1628a_write_byte(dt[1]);
                        else if(i==4)tm1628a_write_byte(dt[2]);
                        else if(i==6)tm1628a_write_byte(dt[3]);
                        else tm1628a_write_byte(NONE);
                    }
                    else
                    {
                        if(i==0)tm1628a_write_byte(dt[0]);
                        else if(i==2)tm1628a_write_byte(dt[1]);
                        else if(i==4)tm1628a_write_byte(dt[2]);
                        else if(i==6)tm1628a_write_byte(dt[3]);
                        else tm1628a_write_byte(NONE);
                    }
                    
                }   
                else tm1628a_write_byte(NONE);
                TM1628A_STB_HIGH();
            }
            step++;break;

            case 1:
            if(mode==0 && break_flag<2)
            {
                break_flag++;
                HAL_Delay(delay_ms);
                tm1628a_clear_op();
                HAL_Delay(delay_ms);
                step=0;
            }
            else
            {
                step++;
                break_flag++;
            }
            break;

            case 2:
            break_flag=3;
            break;
        }
    }
}

/**
 * @brief 显示错误码
 * 
 * @param code         要显示的数字0~9
 * @param delay_ms     延时时间，单位：ms
 * @param mode         模式=1,常显；=0，频闪 
 */
void tm1628a_show_code(uint8_t code, uint16_t delay_ms, uint8_t mode)
{
    uint8_t i, err_code;
    uint8_t step = 0;

    if(code >= 10)return;
    err_code = digit_map[code % 10];
   
    // emlog(LOG_DEBUG,"err_code: %02x\r\n",err_code);

    for(step = 0;step<3;step++)
    {
       
        for(i=0;i<14;i++)
        {
            TM1628A_STB_LOW();
            tm1628a_write_byte(CXH(i));

            switch(i)
            {
                case 0:tm1628a_write_byte(NONE);break;       // 第一段
                case 1:tm1628a_write_byte(NONE);break;
                case 2:tm1628a_write_byte(LOG_E);break;      // 第二段
                case 3:tm1628a_write_byte(NONE);break;
                case 4:tm1628a_write_byte(err_code);break;      // 第三段
                case 5:tm1628a_write_byte(NONE);break;
                case 6:tm1628a_write_byte(NONE);break;       // 第四段
                case 7:tm1628a_write_byte(NONE);break;
                case 8:tm1628a_write_byte(NONE);break;       // 第五段
                case 9:tm1628a_write_byte(CLK_PT);break;
                case 10:tm1628a_write_byte(NONE);break;      // 第六段
                case 11:tm1628a_write_byte(NONE);break;
                case 12:tm1628a_write_byte(NONE);break;      // 第七段
                case 13:tm1628a_write_byte(NONE);break;  
            }
            
            TM1628A_STB_HIGH();
        }
        if(mode==0 && step<2)
        {
            tm1628a_clear_op();
            HAL_Delay(delay_ms);
        }
        else if(mode)
        {
            break;
        }
            
        
    }
}
