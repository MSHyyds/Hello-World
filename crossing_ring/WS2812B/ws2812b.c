#include "ws2812b.h"
#include "string.h"
#include "tim.h"
#include "math.h"

// G:R:B 
const COLOR_T GREEN    = {255,0,0};     // 绿色
const COLOR_T RED      = {0,255,0};     // 红色
const COLOR_T BLUE     = {0,0,255};     // 蓝色

const COLOR_T YELLOW   = {255,255,0};   // 黄色
const COLOR_T CYAN     = {255,0,255};   // 青色
const COLOR_T MAGENTA  = {0,255,255};   // 品红

const COLOR_T BLACK    = {0,0,0};       // 黑色
const COLOR_T WHITE    = {255,255,255}; // 白色

const COLOR_T SILVER   = {128,128,128}; // 银色
const COLOR_T SKY      = {191,0,255};   // 天蓝色
const COLOR_T MEDIUM   = {224,64,208};  // 松石绿色
const COLOR_T PURPLE   = {0,128,128};   // 紫色
const COLOR_T JACINTH  = {69,255,0};    // 橘红色
const COLOR_T INDIGO   = {0,75,130};    // 靛蓝色
const COLOR_T GOLD     = {215,255,0};   // 金色

COLOR_T COLOR_FUL[14]= {{255,0,0}, {0,255,0}, {0,0,255}, 
                        {255,255,0}, {255,0,255}, {0,255,255}, 
                        {128,128,128}, {191,0,255}, {224,64,208},
                        {0,128,128}, {69,255,0}, {0,75,130}, {215,255,0}, {255,255,255}};

COLOR_T RED_BLUE[2]= {{0,255,0}, {0,0,255}};

uint32_t color_raw[BUFF_LEN];


void ws_start_run(void)
{
    HAL_TIM_PWM_Start_DMA(&WS_TIM, WS_TIM_CH1, color_raw, BUFF_LEN);
    HAL_TIM_PWM_Start_DMA(&WS_TIM, WS_TIM_CH2, color_raw, BUFF_LEN);
}

void ws_reset(void)
{
    uint16_t i;
   
    for(i=0;i<LED_TOTAL_CNT;i++)
    {
        get_sigle_color(&(color_raw[i*24]), BLACK);
    }
    
}

/**
 * @brief Get the sigle color object, and assign to the buf
 * 
 * @param buf 
 * @param color 
 */
void get_sigle_color(uint32_t* buf, COLOR_T color)
{
    uint16_t i;

    for(i=0;i<8;i++)
    {
        buf[i] = ((color.G & (1<<(7-i))) ? CODE_1 : CODE_0);
    }
    for(i=8;i<16;i++)
    {
        buf[i] = ((color.R & (1<<(15-i))) ? CODE_1 : CODE_0);
    }
    for(i=16;i<24;i++)
    {
        buf[i] = ((color.B & (1<<(23-i))) ? CODE_1 : CODE_0);
    }

}

/**
 * @brief 全部亮一种颜色
 * 
 * @param buf 数据缓存区
 * @param cnt 灯数量，需 <= LED_TOTAL_CNT
 * @param color 要显示的颜色
 */
void ws_light_shine(uint32_t* buf, uint16_t cnt, COLOR_T color)
{
    uint16_t i;

    if(cnt > LED_TOTAL_CNT)
    {
        return;
    }
    if(buf==NULL)buf=color_raw;
    ws_reset();
    
    for(i=0;i<cnt;i++)
    {
        get_sigle_color(&(buf[i*24]), color);
    }

    for(i=0;i<24;i++)
    {
        color_raw[BUFF_LEN-24+i]=0;
    }
    ws_start_run(); 
} 

/**
 * @brief 从begin开始的cnt个灯以color颜色发光
 * 
 * @param buf   数据缓存区
 * @param begin 开始序号
 * @param cnt   发光数量
 * @param color 发光颜色
 * @param dir   递进方向， 1 正向（通信方向）; 0 逆向
 */
void ws_light_place(uint32_t *buf, uint16_t begin, uint16_t cnt, COLOR_T color, uint8_t dir)
{
    uint16_t i;

    if(cnt > LED_TOTAL_CNT)
    {
        mylog("error: exceed light count\r\n");
        return;
    }
    
    if(buf==NULL)buf=color_raw;
    // ws_reset(); 
    
    for(i=0;i<=LED_TOTAL_CNT;i++)  // 这里 i <= (begin+cnt) 的目的是保证最后有 BLACK 填充，稳定数据帧的颜色编码
    {
        if(dir)
        {
            if(i<begin)
            {
                get_sigle_color(&(buf[i*24]), BLACK);
            }
            else if(i<begin+cnt)
            {
                get_sigle_color(&(buf[i*24]), color);
            }
            else 
            {
                if(cnt==0)get_sigle_color(&(buf[i*24]), BLACK);
            }
        }
        else
        {
            if(i<(begin+1-cnt))
            {
                get_sigle_color(&(buf[i*24]), BLACK);
            }
            else if(i<=begin)
            {
                get_sigle_color(&(buf[i*24]), color);
            }
            else 
            {
                if(cnt==0)
                get_sigle_color(&(buf[i*24]), BLACK);
            }
        }
        
    }

    for(i=0;i<24;i++)
    {
        color_raw[BUFF_LEN-24+i]=0;
    }
    ws_start_run(); 
}

/**
 * @brief 从begin开始逐个点亮cnt个color颜色的LED，递增时间跟调用该函数的时间间隔有关
 * 
 * @param buf   数据缓存区
 * @param begin 开始序号
 * @param cnt   点亮数量
 * @param color 发光颜色
 * @param dir   递进方向， 1 正向（通信方向）; 0 逆向
 */
void ws_light_inc(uint32_t *buf, uint16_t begin, uint16_t cnt, COLOR_T color, uint8_t dir)
{
    static uint16_t add_cnt = 0;
    
    if(dir == 0 && (begin + 1 < cnt))
    {
        return;
    }

    if(dir==0 && begin + 1  < cnt)
    {
        return;
    }

    if(add_cnt<=cnt)
    {
        ws_light_place(buf, begin, add_cnt++, color, dir);
    }
    else
    {
        ws_light_place(buf, begin, cnt, BLACK, dir);
        add_cnt = 0;
    }
    
    
}

/**
 * @brief 从begin开始逐个熄灭cnt个color颜色的LED，递减时间跟调用该函数的时间间隔有关
 * 
 * @param buf   数据缓存区
 * @param begin 开始序号
 * @param cnt   点亮数量
 * @param color 发光颜色
 * @param dir   递进方向， 1 正向（通信方向）; 0 逆向
 */
void ws_light_dec(uint32_t *buf, uint16_t begin, uint16_t cnt, COLOR_T color, uint8_t dir)
{
    static uint16_t sub_cnt = 0;

   
    
    if(dir==0)
    {
        if(sub_cnt<=cnt)
        {
            ws_light_place(buf, begin, cnt-sub_cnt, color, 1);
            sub_cnt++;
        }
        else
        {
            ws_light_place(buf, begin, cnt, BLACK, 1);
            sub_cnt = 0;
        }
    }
    else
    {
        
        if(sub_cnt < cnt)
        {
            ws_light_place(buf, begin, cnt-sub_cnt, color, 0);
            sub_cnt++;
        }
        else
        {
            ws_light_place(buf, begin, cnt, BLACK, 0);
            sub_cnt = 0;
        }
    }
    
}




// 辅助函数：计算最小值
static float min(float a, float b, float c) {
    float m = a < b ? a : b;
    return (m < c ? m : c);
}

// 辅助函数：计算最大值
static float max(float a, float b, float c) {
    float m = a > b ? a : b;
    return (m > c ? m : c);
}

// RGB转HSV函数
void RGBtoHSV(COLOR_T rgb, HSV_T *hsv) 
{
    float red = rgb.R / 255.0f;
    float green = rgb.G / 255.0f;
    float blue = rgb.B / 255.0f;

    float cmax = max(red, green, blue);
    float cmin = min(red, green, blue);
    float delta = cmax - cmin;

    // 计算色调
    if (delta == 0) {
        hsv->h = 0;
    } else if (cmax == red) {
        hsv->h = 60 * ((green - blue) / delta);
    } else if (cmax == green) {
        hsv->h = 60 * ((blue - red) / delta + 2);
    } else {
        hsv->h = 60 * ((red - green) / delta + 4);
    }

    if (hsv->h < 0) {
        hsv->h += 360;
    }

    // 计算饱和度
    if (cmax == 0) {
        hsv->s = 0;
    } else {
        hsv->s = delta / cmax;
    }

    // 计算明度
    hsv->v = cmax;
}


// HSV转RGB函数
void HSVtoRGB(HSV_T hsv, COLOR_T *rgb) 
{
    float c = hsv.v * hsv.s;
    float x = c * (1 - fabs(fmod(hsv.h / 60.0, 2) - 1));
    float m = hsv.v - c;

    float rp, gp, bp;

    if (hsv.h < 60) {
        rp = c;
        gp = x;
        bp = 0;
    } else if (hsv.h < 120) {
        rp = x;
        gp = c;
        bp = 0;
    } else if (hsv.h < 180) {
        rp = 0;
        gp = c;
        bp = x;
    } else if (hsv.h < 240) {
        rp = 0;
        gp = x;
        bp = c;
    } else if (hsv.h < 300) {
        rp = x;
        gp = 0;
        bp = c;
    } else {
        rp = c;
        gp = 0;
        bp = x;
    }

    rgb->R = (rp + m) * 255;
    rgb->G = (gp + m) * 255;
    rgb->B = (bp + m) * 255;
}

 

/**
 * @brief 单色流水灯效果
 * 
 * @param color 灯光颜色
 * @param begin 起始位置，0,1,2...
 * @param end   终止位置
 * @param cnt   流动灯数量
 * @param ms    流动速度，单位ms
 * @param dir   流动方向，1，通信通向；0，通信逆向
 */
void waterfall_light(COLOR_T color, uint16_t begin, uint16_t end, uint16_t cnt, uint16_t ms, uint8_t dir)
{
    uint16_t i =0;

    if((end + 1 - begin) <= cnt)
    {
        return;
    }

    if(dir)
    {
        do{
            ws_light_place(color_raw, begin, i, color, dir);
            HAL_Delay(ms);
        }while(i++ < cnt);

        for(i=begin;i<=end;i++)
        {
            
            if(end+1-i >= cnt)
            {
                ws_light_place(color_raw, i, cnt, color, dir);
            }
            else 
            {
                ws_light_place(color_raw, i, end+1-i, color, dir);
            }
            HAL_Delay(ms);
        }  
    }
    else
    {
        do{
            ws_light_place(color_raw, end, i, color, dir);
            HAL_Delay(ms);
        }while(i++ < cnt);

        for(i=end;i>=begin;i--)
        {
            if(i+1-begin >= cnt)
            {
                ws_light_place(color_raw, i, cnt, color, dir);
            }
            else 
            {
                ws_light_place(color_raw, i, i+1-begin, color, dir);
                // ws_light_place(color_raw, cnt+begin-1, cnt+begin-i, BLACK, dir);
            }
            HAL_Delay(ms);
        }  
    }  
}


/**
 * @brief 颜色数组移位
 * 
 * @param arr 颜色数组
 * @param len 颜色数组长度
 * @param bit 移位位数
 * @param dir 移位方向
 */
void array_shift(COLOR_T arr[], uint16_t len, uint8_t bit, uint8_t dir)
{
    uint16_t i = 0;
    COLOR_T tmp;

    if(len == 0)return;

    if(len <= 2)
    {
        tmp = arr[0];
        arr[0] = arr[1];
        arr[1] = tmp;
        return;
    }
    if(dir==0)
    {
        tmp = arr[0];
        for(i=0;i<len-1;i++)
        {
            arr[i] = arr[i+1];
        }
        arr[i] = tmp;
    }
    else
    {
        tmp = arr[len-1];
        for(i=len-1;i>0;i--)
        {
            arr[i] = arr[i-1];
        }
        arr[i] = tmp;
    }
}

/**
 * @brief 以多种颜色炫彩流动
 * 
 * @param color_arr 颜色数组，可自定义，注意为可修改的类型，不可为const
 * @param arr_len   颜色数组长度
 * @param begin     流动起始位置
 * @param end       流动终止位置
 * @param cnt       单色的数目
 * @param ms        流动的速度 
 * @param dir       流动的方向，1，通信通向；0，通信逆向
 */
void colorful_flow(COLOR_T color_arr[], uint16_t arr_len, uint16_t begin, uint16_t end, uint16_t cnt, uint16_t ms, uint8_t dir)
{
    uint16_t i = 0;
    uint16_t j = 0;

    if(cnt > LED_TOTAL_CNT)
    {
        mylog("error: exceed light count\r\n");
        return;
    }
    
    ws_reset();
    
    for(i=0;i<=LED_TOTAL_CNT;i++) 
    {
        if(i<begin)
        {
            get_sigle_color(&(color_raw[i*24]), BLACK);
        }
        else if(i>=begin && i<=end)
        {
            get_sigle_color(&(color_raw[i*24]), color_arr[j]);
            if((i+1)%cnt == 0)
            {
                j++;
                if(j>=arr_len)
                {
                    j=0;
                }
            } 
        }
        else 
        {
            get_sigle_color(&(color_raw[i*24]), BLACK);
        }
    }

    for(i=0;i<24;i++)
    {
        color_raw[BUFF_LEN-24+i]=0;
    }
    ws_start_run(); 
    array_shift(color_arr, arr_len, 1, dir);
    HAL_Delay(ms);
}


/**
 * @brief 颜色GRB数据转HSV格式，并降低亮度
 * 
 * @param color 颜色数据
 * @param type  1：增加亮度， 0：降低亮度
 * @param val   增加或降低的值大小
 * @return COLOR_T 
 */


COLOR_T bright_var(COLOR_T color, uint8_t type, float val)
{
    HSV_T hsv;
    COLOR_T clr;
    
    if(val<=0 || val>=1)
    {
        return BLACK;
    }

    RGBtoHSV(color, &hsv);
    mylog("first: g:r:b=%d:%d:%d\r\n", color.G, color.R, color.B);

    if(type)
    {
        hsv.v += val;
        if(hsv.v>0.99)
        {
            hsv.v=1;
        }
        
    }
    else
    {
        hsv.v -= val;
        if(hsv.v<=0.01)
        {
            hsv.v=0;
        }
        
    }

    HSVtoRGB(hsv, &clr);
    mylog("second: g:r:b=%d:%d:%d\r\n", clr.G, clr.R, clr.B);
    return clr;
}


/**
 * @brief 存在一定问题，2025.2.13
 * 
 * @param color 
 * @param begin 
 * @param end 
 * @param cnt 
 * @param ms 
 * @param dir 
 */
void meteor_trail(COLOR_T color, uint16_t begin, uint16_t end, uint16_t cnt, uint16_t ms, uint8_t dir)
{
    uint16_t i =0;
    static uint16_t j=1;

    if((end + 1 - begin) <= cnt)
    {
        return;
    }

    
    for(i=0;i<=end;i++)
    {
        if(i<begin)
        {
            get_sigle_color(&color_raw[i*24], BLACK);
        }
        else if(i>= begin && i<= end)
        {
            
            if(end==i)get_sigle_color(&color_raw[i*24],bright_var(color, 0, CHANGE_VAL*10));
            else if(end-1==i)get_sigle_color(&color_raw[i*24],bright_var(color, 0, CHANGE_VAL*5));
            else if(end-2==i)get_sigle_color(&color_raw[i*24],bright_var(color, 0, CHANGE_VAL*4));
            else if(end-3==i)get_sigle_color(&color_raw[i*24],bright_var(color, 0, CHANGE_VAL*3));
            // if(end==i)get_sigle_color(&color_raw[i*24],RED);
            // else if(end-1==i)get_sigle_color(&color_raw[i*24], BLUE);
            // else if(end-2==i)get_sigle_color(&color_raw[i*24], GREEN);
            // else if(end-3==i)get_sigle_color(&color_raw[i*24], CYAN);
            else 
            get_sigle_color(&color_raw[i*24],color);
           
        }
        else
        {
            get_sigle_color(&color_raw[i*24], BLACK);
        }
        
        ws_start_run(); 

        HAL_Delay(ms);
        
    }
    // ws_light_shine(color_raw, LED_TOTAL_CNT, BLACK);
    //     HAL_Delay(ms);
   
    

    
    
    
}

#define GAMMA 2.2f  // 1.8 ~ 2.4 f
/* 伽马矫正 */
float gamma_correct(float v) {
    return powf(v, GAMMA);
}




/**
 * @brief 以一种颜色发光的呼吸灯
 * 
 * @param color 发光颜色
 * @param begin 开始位置，从1开始
 * @param cnt   发光数量
 * @param ms    渐变速度
 */
 void breatheLED(COLOR_T color, uint16_t begin, uint16_t cnt, uint16_t ms) 
 {

    HSV_T hsv,tp;
    COLOR_T temp = color;

    RGBtoHSV(color, &hsv);  // 初始值转换
    RGBtoHSV(color, &tp);   
    do{
        
        ws_light_place(color_raw, begin, cnt, temp, 1);
        hsv.v -= CHANGE_VAL;
        HSVtoRGB(hsv, &temp);
        HAL_Delay(ms);
    }while(hsv.v >= 0.05f);
    
    do{
        hsv.v += CHANGE_VAL;
        HSVtoRGB(hsv, &temp);
        ws_light_place(color_raw, begin, cnt, temp, 1);
        HAL_Delay(ms);
    }while(hsv.v < tp.v - 0.05f);
    
}



// void breatheLED(COLOR_T color, uint16_t begin, uint16_t cnt, uint16_t ms) 
// {
//     HSV_T hsv, original_hsv;
//     COLOR_T temp;
//     float min_v = 0.05f;        // 最小亮度阈值
//     float change_val = 0.01f;   // 每次改变亮度的步进值
//     float v;

//     // 颜色 → HSV（保存原始值）
//     RGBtoHSV(color, &hsv);
//     original_hsv = hsv;

//     // 🌘 暗下去
//     for (v = original_hsv.v; v >= min_v; v -= change_val)
//     {
//         HSV_T temp_hsv = hsv;
//         temp_hsv.v = gamma_correct(v); // gamma 校正
//         HSVtoRGB(temp_hsv, &temp);
//         ws_light_place(color_raw, begin, cnt, temp, 1);
//         HAL_Delay(ms);
//     }

//     // 🌔 亮上来
//     for (v = min_v; v <= original_hsv.v; v += change_val)
//     {
//         HSV_T temp_hsv = hsv;
//         temp_hsv.v = gamma_correct(v); // gamma 校正
//         HSVtoRGB(temp_hsv, &temp);
//         ws_light_place(color_raw, begin, cnt, temp, 1);
//         HAL_Delay(ms);
//     }
// }




