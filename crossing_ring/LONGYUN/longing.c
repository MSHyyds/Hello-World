#include "longing.h"

#include "string.h"
#include "stdio.h"
#include "debug_log.h"

/**
 * @brief 校验和值计算
 * 
 * @param data 待校验数据流
 * @param len  待校验数据长
 * @return uint8_t 返回校验和计算值
 */
static uint8_t calculate_checksum(uint8_t *data, uint16_t len)
{
    uint8_t checksum = 0;
    uint16_t i;

    for(i = 0;i<len;i++)
    {
        checksum ^= data[i];
    }
    return checksum;
}



/**
 * @brief 协议解析
 * 
 * @param buffer 接收到的待解析的数据
 * @param length 待解析数据长度
 * @param frame  解析后获得的协议参数和数据
 * @return uint8_t 返回0成功， 返回1失败
 */
uint8_t protocol_process(uint8_t* buffer, uint16_t length, longing_protocol_t* frame)
{
    if(length < 15)
    {
        // emlog(LOG_ERROR, "total length error.\r\n");
        return 1;
    }

    // 检查帧头
    if (buffer[0] != FRAME_HEADER_1 || buffer[1] != FRAME_HEADER_2 ||
        buffer[2] != FRAME_HEADER_3 || buffer[3] != FRAME_HEADER_4){
        emlog(LOG_ERROR, "frame head error.\r\n");
        return 2;  // 帧头错误
    }

    if(buffer[length-2] != FRAME_END_1 || buffer[length-1] != FRAME_END_2)
    {
        emlog(LOG_ERROR, "frame end error.\r\n");
        return 2;
    }
    // 检查帧类型 (发送 0x75 / 响应 0x7A)
    if (buffer[4] == FRAME_SEND) {
        frame->type = FRAME_SEND;
    } else if (buffer[4] == FRAME_RESPONSE) {
        frame->type = FRAME_RESPONSE;
    } else {
        emlog(LOG_ERROR, "frame type error.\r\n");
        return 3; // 类型错误
    }

    // 解析数据
    frame->target_addr = (buffer[5] << 8) | buffer[6]; // 目标地址
    frame->local_addr = (buffer[7] << 8) | buffer[8];  // 本机地址
    frame->cmd_seq = buffer[9];   // 命令序列号
    frame->cmd = buffer[10];      // 命令字
    frame->data_len = (buffer[11] << 8) | buffer[12]; // 数据块长度

    // 校验数据长度合法性
    if (frame->data_len > FRAME_MAX_DATA || (15 + frame->data_len) > length) {
        emlog(LOG_ERROR, "data length error: %d.\r\n", frame->data_len);
        emlog(LOG_ERROR, "into total len: %d\r\n", length);
        
        return 4; // 数据长度异常
    }

    // 复制数据块
    memcpy(frame->data, &buffer[13], frame->data_len);

    // 获取校验字
    frame->checksum = buffer[13 + frame->data_len];

    // 计算校验值
    uint8_t computed_checksum = calculate_checksum(buffer, 13 + frame->data_len);
    if (computed_checksum != frame->checksum) {
        emlog(LOG_ERROR, "checksum val error.\r\n");
        return 5;  // 校验失败
    }

    return 0;  // 解析成功
}


/**
 * @brief 协议帧打包
 * 
 * @param frame  协议参数和数据
 * @param buffer 打包缓存数组
 * @return uint16_t 返回帧长度
 */
uint16_t pack_frame(longing_protocol_t *frame, uint8_t *buffer) 
{
    uint16_t index = 0;

    // 帧头
    buffer[index++] = FRAME_HEADER_1;
    buffer[index++] = FRAME_HEADER_2;
    buffer[index++] = FRAME_HEADER_3;
    buffer[index++] = FRAME_HEADER_4;

    // 帧类型 (发送帧 0x75 / 响应帧 0x7A)
    buffer[index++] = frame->type;

    // 目标地址
    buffer[index++] = (frame->target_addr >> 8) & 0xFF;
    buffer[index++] = frame->target_addr & 0xFF;

    // 本机地址
    buffer[index++] = (frame->local_addr >> 8) & 0xFF;
    buffer[index++] = frame->local_addr & 0xFF;

    // 命令序列号
    buffer[index++] = frame->cmd_seq;

    // 命令字
    buffer[index++] = frame->cmd;

    // 数据块长度
    buffer[index++] = (frame->data_len >> 8) & 0xFF;
    buffer[index++] = frame->data_len & 0xFF;

    // 复制数据块
    memcpy(&buffer[index], frame->data, frame->data_len);
    index += frame->data_len;

    // 计算校验码
    buffer[index++] = calculate_checksum(buffer, index);

    // 帧尾
    buffer[index++] = FRAME_END_1;
    buffer[index++] = FRAME_END_2;

    // 返回帧长度
    return index;
}

/**
 * @brief       将浮点型的数据转存为字符型数组
 * 
 * @param val       数据源
 * @param f_len     数据源长度
 * @param bytes     转后数据
 * @return uint8_t  返回转后数据长度
 */
uint8_t float_to_uint8_t(float* val, uint8_t f_len, uint8_t* bytes)
{
    uint8_t i;
    uint8_t* p;
    
    for(i=0;i<f_len;i++)
    {
        // bytes[i*4+0] = (uint8_t*)&val[i];
        // bytes[i*4+1] = (uint8_t*)(&val[i]+1);
        // bytes[i*4+2] = (uint8_t*)(&val[i]+2);
        // bytes[i*4+3] = (uint8_t*)(&val[i]+3);

        p = (uint8_t*)&val[i];
        bytes[i*4+0] = p[0];
        bytes[i*4+1] = p[1];
        bytes[i*4+2] = p[2];
        bytes[i*4+3] = p[3];
    }
    return (f_len * 4);
}

/**
 * @brief 填充帧中数据段
 * 
 * @param src           要传递的数据
 * @param group_cnt     数据组数量，即传递几组的数据，与序号相关
 * @param len           每个组中的数据量
 * @param data          要填充地数据段
 * @return uint8_t      返回填充后的数据段长度
 */
uint8_t filler_frame_data(uint8_t* src, uint8_t group_cnt, uint16_t len, uint8_t* data)
{
    uint16_t i,j;

    for(i=0;i<group_cnt;i++)
    {
        for(j=0;j<len+1;j++)
        {
            if((i*len+j)%(len+1) == 0)
            {
                data[i*len+j] = i % group_cnt;
            }
            else
            {
                data[i*len+j] = src[i*len+j];
            }
            
        }
    }
    return (group_cnt * (len + 1));
}


