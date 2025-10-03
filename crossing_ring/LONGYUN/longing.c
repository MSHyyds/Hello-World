#include "longing.h"

#include "string.h"
#include "stdio.h"
#include "debug_log.h"
#include "gpio_ctrl.h"
#include "config_info.h"



extern uint8_t test_buf[200];
longing_protocol_t send_info;
longing_protocol_t recv_info;


/**
 * @brief 异或校验和值计算
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
    uint16_t i,j,k;
    k=0;

    len += 1;   // 组长度要增加1，因为要插入组序号

    for(i=0;i<group_cnt;i++)
    {
        for(j=0;j<len;j++)
        {
            if((i*len+j)%len == 0)
            {
                data[i*len+j] = i % group_cnt;
            }
            else
            {
                data[i*len+j] = src[k++];
            }
            
        }
    }
    return (group_cnt * len);
}

/**
 * @brief 上传设备的数据，传感器、数码管、灯光颜色、电量
 * 
 * @param send_info 协议结构体
 * @param cmd_seq   命令序号
 * @param cmd       命令字
 * @param data      数据数组
 * @param data_cnt  数据组数量
 * @return uint16_t 
 */
uint16_t upload_device_data(longing_protocol_t* send_info, uint8_t cmd_seq, uint8_t cmd, uint8_t* re_data, uint8_t data_cnt)
{
    uint8_t send_len;

    send_info->type = FRAME_RESPONSE;
    send_info->target_addr = 0xffee;
    send_info->local_addr = get_dip_switch_addr();
    send_info->cmd_seq = cmd_seq;
    send_info->cmd = cmd;

  
    switch(cmd)
    {
        case ORHC:
        case ORIR:
        case ORBV:
            send_info->data_len = filler_frame_data(re_data, data_cnt, 4, send_info->data);  // 每组4字节数据
            break;

        
        case ORAR:
            send_info->data_len = filler_frame_data(re_data, data_cnt, 1, send_info->data);  // 每组1字节数据
            break;

        case ORSC:
            send_info->data_len = filler_frame_data(re_data, data_cnt, 2, send_info->data);  // 每组2字节数据
            break;

        case ORCR:
            send_info->data_len = filler_frame_data(re_data, data_cnt, 3, send_info->data);  // 每组3字节数据
            break;
    }
    
    send_len = pack_frame(send_info, test_buf);
    send_to_wifi(test_buf, send_len);
    
    return send_len;
}



uint8_t dev_addr;
float dist[HC_HEAD_NUM];
uint8_t re_data[10];
uint8_t ir_id[IR_HEAD_NUM][4];

/**
 * @brief 接收数据根据协议处理
 * 
 * @param rx_buff   接收的缓存数组
 * @param len       接收的数据长度
 * @param recv_info 解析后存放的结构体
 * @return uint8_t 返回 1，2，3，4，5 失败；返回 cmd 成功
 */
uint8_t recv_cmd_precess(uint8_t* rx_buff, uint16_t len, longing_protocol_t* recv_info)
{
    uint8_t tmp;
    uint16_t change_score;
    // hc_start();


    tmp = protocol_process(rx_buff, len, recv_info);
    dev_addr = get_dip_switch_addr();

    if(tmp==0)
    {
        /* 对比自身地址 */
        if(recv_info->target_addr == dev_addr)
        {
            if(recv_info->type == FRAME_SEND)
            {
                switch(recv_info->cmd)
                {   
                    /* 下发指令查询 */
                    case OTKP:
                        upload_device_data(recv_info, recv_info->cmd_seq, ORKP, NULL, NULL);

                        break;
                    
                    case ORAR: 
                        if(get_device_mode())
                        {
                            re_data[0] = dev_addr | 0x80;
                        }
                        else
                        {
                            re_data[0] = dev_addr & 0x7f;
                        }
                        
                        upload_device_data(recv_info, recv_info->cmd_seq, recv_info->cmd, re_data, 1);

                        break;

                    case ORHC:
                        hc_read(dist, HC_HEAD_NUM);
                        float_to_uint8_t(dist, HC_HEAD_NUM, re_data);
                        upload_device_data(recv_info, recv_info->cmd_seq, recv_info->cmd, re_data, HC_HEAD_NUM);
                        break;

                    case ORIR:
                        ir_read(ir_id, IR_HEAD_NUM);
                        
                        upload_device_data(recv_info, recv_info->cmd_seq, recv_info->cmd, ir_id[0], IR_HEAD_NUM);
                        break;
                    
                    case ORBV:
                        dist[0] = adc_get_value();
                        float_to_uint8_t(dist, 1, re_data);
                        upload_device_data(recv_info, recv_info->cmd_seq, recv_info->cmd, re_data, 1);
                        break;

                    case ORSC:
                        get_current_score(config_info_1, re_data);
                        upload_device_data(recv_info, recv_info->cmd_seq, recv_info->cmd, re_data, 1);
                        break;
                    
                    case ORCR:
                        re_data[0] = config_info_1.cross_color.R;
                        re_data[1] = config_info_1.cross_color.G;
                        re_data[2] = config_info_1.cross_color.B;
                        upload_device_data(recv_info, recv_info->cmd_seq, recv_info->cmd, re_data, 1);
                        break;
                    
                    /* 下发指令控制 */
                    case OSWS:
                        break;

                    case OSADDS:
                        change_score = (recv_info->data[0] << 8) + recv_info->data[1];
                        config_info_1.curr_score = config_info_1.curr_score + change_score;
                        upload_device_data(recv_info, recv_info->cmd_seq, recv_info->cmd, NULL, NULL);
                        tmp = OSADDS;
                        break;
                    case OSSUBS:
                        change_score = (recv_info->data[0] << 8) + recv_info->data[1];
                        if(config_info_1.curr_score <= change_score)
                        {
                            config_info_1.curr_score = 0;
                        }
                        else
                        {
                            config_info_1.curr_score = config_info_1.curr_score - change_score;
                        }
                        upload_device_data(recv_info, recv_info->cmd_seq, recv_info->cmd, NULL, NULL);
                        tmp = OSSUBS;
                        break;
                    case OSREZS:
                        config_info_1.curr_score = 0;
                        upload_device_data(recv_info, recv_info->cmd_seq, recv_info->cmd, NULL, NULL);
                        break;
                    case OSWARN:
                        upload_device_data(recv_info, recv_info->cmd_seq, recv_info->cmd, NULL, NULL);
                        tmp = OSWARN;
                        break;
                }       
            }
        }
    }

    return tmp;
}


uint8_t keep_alive_test(void)
{
    uint8_t tmp=1;
    uint8_t cmd_seq=0;

    while(tmp)
    {
        tmp = connect_state();
    }
    upload_device_data(&send_info, cmd_seq, OTKP, NULL, NULL);

    return 0;
}

