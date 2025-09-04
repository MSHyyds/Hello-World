#ifndef __LONGING_H
#define __LONGING_H

#include "stm32f1xx_hal.h"
#include "usart.h"
#include "hc_sr04.h"
#include "ir.h"

/** 龙云协议数据格式 
 * 
 *  命令帧：
 *  | 命令头4B          | 起始字1B | 目标地址2B     | 本机地址2B   | 命令顺序号1B | 命令字1B     | 数据块长度2B   | 数据块              | 检验字1B | 命令尾2B |
 *  | 55H 55H AAH AAH  | 75H      | 高字节+低字节  | 高字节+低字节 | 从00H开始   | 00~FFH       | 高字节+低字节  | 命令字对应数据或参数 | 00~FFH   | 0x0d 0x0a|
 * 
 *  应答帧：
 *  | 55H 55H AAH AAH  | 7AH      | 高字节+低字节  | 高字节+低字节 | 同命令序号一样 | 同命令字一样 | 高字节+低字节  | 命令字对应数据或参数 | 00~FFH   | 0x0d 0x0a|
 * 
 * 
 * 
 */

#define FRAME_HEADER_1 0x55
#define FRAME_HEADER_2 0x55
#define FRAME_HEADER_3 0xaa
#define FRAME_HEADER_4 0xaa
#define FRAME_END_1    0x0d
#define FRAME_END_2    0x0a
#define FRAME_SEND     0x75
#define FRAME_RESPONSE 0x7a
#define FRAME_MAX_DATA 64  // 帧中数据最大长度
#define FRAME_FIX_MIN  15   // 帧固定最小长度，不包括数据块实体，即：帧头4B + 起始字1B + 目标地址2B + 本机地址2B + 命令顺序1B + 命令字1B + 数据长度2B + 《数据块实体》 + 帧尾2B =15 + 《数据块实体》

#define BROADCAST_ADDR 0xFFEE


/* 龙云私有协议 */

/* 心跳包 */
#define OTKP    0x20    // 请求
#define ORKP    0x21    // 响应

/* 下发 */
#define OSWS    0x30    // 设置灯带
#define OSADDS  0x31    // 分数增加
#define OSSUBS  0x32    // 分数减少
#define OSREZS  0x33    // 分数归零
#define OSWARN  0x35    // 违规警告

/* 上位机查询 --- 设备上传 */

#define ORAR    0x41    // 查询读取地址, 5位编码器， 最高位为模式位，即：M * * x  x x x x, eg:0b1xx1 0101
#define ORHC    0x42    // 查询读取超声波
#define ORIR    0x43    // 查询读取红外ID
#define ORBV    0x44    // 查询读取电池电压
#define ORCR    0x45    // 查询读取灯带颜色
#define ORSC    0x46    // 查询读取分数

#pragma pack(push, 1)
typedef struct _longing_protocol_t
{
    uint16_t target_addr;          // 目标地址
    uint16_t local_addr;          // 本机地址
    uint8_t cmd_seq;            // 命令序号
    uint8_t cmd;                // 命令字
    uint16_t data_len;          // 数据块长度
    uint8_t data[FRAME_MAX_DATA];           // 数据块缓存数组
    uint8_t checksum;           // 校验和
    uint8_t type;               // 0x75=发送帧，0x7a=响应帧
}longing_protocol_t;

#pragma pack(pop)

extern longing_protocol_t send_info;
extern longing_protocol_t recv_info;
extern uint8_t dev_addr;
extern float dist[HC_HEAD_NUM];
extern uint8_t re_data[10];
extern uint8_t ir_id[IR_HEAD_NUM][4];



uint8_t protocol_process(uint8_t* buffer, uint16_t length, longing_protocol_t* frame);
uint16_t pack_frame(longing_protocol_t *frame, uint8_t *buffer);
uint8_t float_to_uint8_t(float* val, uint8_t f_len, uint8_t* bytes);
uint8_t filler_frame_data(uint8_t* src, uint8_t group_cnt, uint16_t len, uint8_t* data);
uint16_t upload_device_data(longing_protocol_t* send_info, uint8_t cmd_seq, uint8_t cmd, uint8_t* re_data, uint8_t data_cnt);
uint8_t recv_cmd_precess(uint8_t* rx_buff, uint16_t len, longing_protocol_t* recv_info);

#endif


