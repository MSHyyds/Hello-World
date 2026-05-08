#ifndef FRAME_H
#define FRAME_H
#include <Arduino.h>   
#include <stdint.h> 

#define FRAME_HEAD_1 0x55
#define FRAME_HEAD_2 0xAA
#define FRAME_END    0x0D

#define FRAME_DATA_MAX_LEN 64

typedef struct {
    uint8_t cmd;
    uint8_t len;
    uint8_t data[FRAME_DATA_MAX_LEN];
} frame_packet_t;


extern volatile bool opendoorFlag;
extern volatile bool keepaliveFlag;
extern unsigned int duration; 

extern void send_to_wifi(uint8_t* buff, unsigned int len);
void frame_init(void);
void frame_send(uint8_t cmd, uint8_t *data, uint8_t len);
void frame_on_packet(frame_packet_t* pkt);
void frame_uart_input(uint8_t byte);

#endif