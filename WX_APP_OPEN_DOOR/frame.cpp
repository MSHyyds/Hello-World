#include "frame.h"


static enum{
    S_HEAD1,
    S_HEAD2,
    S_LEN,
    S_CMD,
    S_DATA,
    S_CS,
    S_END
} state = S_HEAD1;

static uint8_t  rx_buf[FRAME_DATA_MAX_LEN + 16];
static uint16_t rx_index;
static uint8_t  packet_len;
static uint8_t  data_len;


// 先算 XOR， 再取反
static uint8_t frame_calc_cs(uint8_t *buf, uint8_t len)
{
    uint8_t cs = 0;
    for(uint8_t i = 0; i < len; i++)
        cs ^= buf[i];
    return cs;
}

void frame_init(void)
{
    state = S_HEAD1;
    rx_index = 0;
}

/* ================= 协议发送 ================= */


/** 发送帧
 *  cmd: 命令字
 *  data： 数据
 *  len: 数据长度
 */
void frame_send(uint8_t cmd, uint8_t *data, uint8_t len)
{
    uint8_t buf[FRAME_DATA_MAX_LEN + 16];
    uint16_t p = 0;

    buf[p++] = FRAME_HEAD_1;
    buf[p++] = FRAME_HEAD_2;
    buf[p++] = len + 2;   //  CMD + CS
    buf[p++] = cmd;

    for(uint8_t i = 0; i < len; i++)
        buf[p++] = data[i];

    uint8_t cs = frame_calc_cs(&buf[2], 1 + 1 + len);
    buf[p++] = cs;
    buf[p++] = FRAME_END;

    send_to_wifi(buf, p);
}

/* ================= 协议解析入口 ================= */

void frame_uart_input(uint8_t byte)
{
    static uint8_t calc_cs = 0;
    frame_packet_t pkt;

    switch (state)
    {
    case S_HEAD1:
        if(byte == FRAME_HEAD_1) state = S_HEAD2;
        break;

    case S_HEAD2:
        if(byte == FRAME_HEAD_2) state = S_LEN;
        else state = S_HEAD1;
        break;

    case S_LEN:
        packet_len = byte;
        if(packet_len < 2 || packet_len > FRAME_DATA_MAX_LEN + 2){
            frame_init();
            break;
        }
        data_len = packet_len - 2;
        rx_index = 0;
        calc_cs = packet_len;
        state = S_CMD;
        break;

    case S_CMD:
        rx_buf[rx_index++] = byte;
        calc_cs ^= byte;
        state = (data_len > 0) ? S_DATA : S_CS;
        break;

    case S_DATA:
        rx_buf[rx_index++] = byte;
        calc_cs ^= byte;
        if(rx_index >= (uint16_t)(1 + data_len))
            state = S_CS;
        break;

    case S_CS:
        if(calc_cs == byte)
            state = S_END;
        else
            frame_init();
        break;

    case S_END:
        if(byte == FRAME_END){
            pkt.cmd = rx_buf[0];
            pkt.len = data_len;
            for(uint8_t i = 0; i < data_len; i++)
                pkt.data[i] = rx_buf[i + 1];
            frame_on_packet(&pkt);
        }
        frame_init();
        break;
    }
}

// 指令类型
#define KEEPALIVE 0x20   // 心跳包
#define DOOR_CTL  0x21   // 门禁控制命令

#define GET_DURA  0x22   // 获取单次开门后的锁定时间

const char keep_alive[6] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
const char keep_alive_resply[6] = {0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa};

void frame_on_packet(frame_packet_t* pkt)
{
    Serial.printf("RX CMD=0x%02X LEN=%d\r\n", pkt->cmd, pkt->len);
    
    /* 示例：收到任何命令，原样 ACK */
    uint8_t ack = 0xAA;
    // frame_send(pkt->cmd, &ack, 1);
    
    switch(pkt->cmd){
      case KEEPALIVE:frame_send(pkt->cmd, (uint8_t*)&keep_alive_resply, sizeof(keep_alive_resply));
        if(memcmp(pkt->data, keep_alive, pkt->len)==0)keepaliveFlag=true;
        else keepaliveFlag - false;
      break;

      case DOOR_CTL:frame_send(pkt->cmd, &ack, 1);
        if(pkt->data[0] == 0)opendoorFlag=false;
        else if(pkt->data[0] == 0x01)opendoorFlag=true;
      break;

      case GET_DURA:
      uint8_t pause_time = duration;
      frame_send(pkt->cmd, &pause_time, 1);
      break;
    }
   
}


