#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H

#include "stm32f1xx_hal.h"


#define RB_MAX_LEN  2048

typedef struct
{
    uint16_t write_index;
    uint16_t read_index;
    uint8_t *buff_data;
    uint16_t buff_size;

}RB_INFO_T;

extern uint8_t rx_complt;       // 在ringbuffer.c中定义
extern uint8_t rx_data;         // 在ringbuffer.c中定义
extern uint8_t rx_buff[200];    // 在ringbuffer.c中定义
extern RB_INFO_T ringbuffer;    // 在ringbuffer.c中定义

extern int mylog(char *format, ...);


uint16_t get_free_len(RB_INFO_T *rb);
uint16_t get_used_len(RB_INFO_T *rb);
void ringbuffer_init(RB_INFO_T *rb, uint8_t* pool, uint16_t size);
void write_1byte(RB_INFO_T *rb, uint8_t dat);
uint8_t read_1byte(RB_INFO_T *rb, uint8_t* dat);
uint16_t write_bytes(RB_INFO_T *rb, uint8_t *buf, uint16_t len);
uint16_t read_bytes(RB_INFO_T *rb, uint8_t *buf, uint16_t len);
void clear_buffer(RB_INFO_T *rb);

#endif

