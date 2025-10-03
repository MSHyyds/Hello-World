#include "ringbuffer.h"
#include "string.h" 

uint8_t rx_complt =0;
uint8_t rx_data;                // 串口接收变量
uint8_t rx_buff[200];           // 串口接收环形缓存区的实体数组
RB_INFO_T ringbuffer;           // 定义对应环形缓存区

/**
 * @brief 得到缓存剩余空闲长度
 * 
 * @param rb 缓存句柄
 * @return uint16_t 
 */
uint16_t get_free_len(RB_INFO_T *rb)
{
    return (rb->buff_size - get_used_len(rb));
}

/**
 * @brief 得到缓存已用长度
 * 
 * @param rb 缓存句柄
 * @return uint16_t 
 */
uint16_t get_used_len(RB_INFO_T *rb)
{
    if (rb->write_index >= rb->read_index)
        return rb->write_index - rb->read_index;
    else
        return rb->buff_size - rb->read_index + rb->write_index;
}

/**
 * @brief 初始化缓存区，将写指针和读指针都置零
 * 
 * @param rb    缓存区句柄
 * @param pool  缓存区指向的数组
 * @param size  缓存区大小
 */
void ringbuffer_init(RB_INFO_T *rb, uint8_t* pool, uint16_t size)
{
    assert_param(rb != NULL);
    assert_param(size > 0);

    rb->write_index = 0;
    rb->read_index  = 0;

    rb->buff_data = pool;
    rb->buff_size = size;
}

/**
 * @brief 向缓存区写入一个字节
 * 
 * @param rb    缓存区句柄
 * @param dat   要写入的字节
 */
void write_1byte(RB_INFO_T *rb, uint8_t dat)
{
    uint16_t next_write_index = (rb->write_index + 1) % rb->buff_size;
    if(next_write_index == rb->read_index)
    {
        mylog("buffer is full.\r\n");
        return;
    }
    else
    {
        rb->buff_data[rb->write_index] = dat;
        rb->write_index = next_write_index;
    }
    
}

/**
 * @brief 向缓存区读取一个字节
 * 
 * @param rb    缓存区句柄
 * @param dat   读取出来的字节
 * @return uint8_t 
 */
uint8_t read_1byte(RB_INFO_T *rb, uint8_t* dat)
{
    if(rb->write_index == rb->read_index)
    {
        mylog("buffer is empty.\r\n");
        return 0;
    }
    else
    {
        *dat = rb->buff_data[rb->read_index];
        rb->read_index = (rb->read_index + 1) % rb->buff_size;
    }
    return *dat;
}

/**
 * @brief 向缓存区写入长度为LEN的字符串
 * 
 * @param rb    缓存区句柄
 * @param buf   字符串
 * @param len   字符串长度
 * @return uint16_t 剩余空间长度
 */
uint16_t write_bytes(RB_INFO_T *rb, uint8_t *buf, uint16_t len)
{
    uint16_t i;
    uint16_t free_len = get_free_len(rb);

    if(free_len < len)
    {
        mylog("space is not enough.\r\n");
        return free_len;
    }
    else
    {
        for(i=0;i<len;i++)
        {
            write_1byte(rb, buf[i]);
        }
    }
    return get_free_len(rb);
}

/**
 * @brief   向缓存区读出长度为LEN的字符串
 * 
 * @param rb  缓存区句柄
 * @param buf 字符串
 * @param len 字符串长度
 * @return uint16_t 剩余空间长度
 */
uint16_t read_bytes(RB_INFO_T *rb, uint8_t *buf, uint16_t len)
{
    uint16_t i,temp;
    temp = get_used_len(rb);
    if(temp < len)
    {
        for(i=0;i<temp;i++)
        {
            read_1byte(rb, &buf[i]);
        }
    }
    else
    {
         for(i=0;i<len;i++)
        {
            read_1byte(rb, &buf[i]);
        }
    }
    return get_free_len(rb);
}

void clear_buffer(RB_INFO_T *rb)
{
    rb->write_index = 0;
    rb->read_index  = 0;
    memset(rb->buff_data, 0, rb->buff_size); 
}


