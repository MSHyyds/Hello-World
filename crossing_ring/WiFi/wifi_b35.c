#include "wifi_b35.h"
#include "string.h"
#include "stdio.h"
#include "main.h"
#include "debug_log.h"  // 调试日志


uint8_t ins_buf[64];    // 指令缓存区
uint8_t ins_rx[64];     // 指令接收buffer



/**
 * @brief 接收wifi模块回复的数据
 * 
 * @param recv_buf  接收数据存放的缓存数组
 * @return uint16_t 返回接收的数据长度
 */
static uint16_t recv_reply(uint8_t* recv_buf)
{
    uint16_t cnt,cnt2;

    // if(rx_complt)
    {
      cnt = get_used_len(&ringbuffer);
      cnt2 = get_free_len(&ringbuffer);
      if(cnt!=0)
      {
        read_bytes(&ringbuffer, recv_buf, cnt);
        return cnt;
      }
      
    //   emlog(LOG_DEBUG, "recv len =%d, remain len: %d, data is:[%s]",cnt, cnt2, recv_buf);  // 测试接收情况
    //   memset(recv_buf, 0, cnt);
    //   rx_complt = 0;
    // emlog(LOG_DEBUG, "%s",recv_buf);
     
    
    }
    return 0;
}


/**
 * @brief 过滤掉一组数据中的\r\n
 * 
 * @param src 源数据
 * @param res 操作后的数据
 * @param len 源数据长度
 */
void cond_msg(uint8_t* src, uint8_t* res, uint16_t len)
{
    uint16_t i=0,j=0,k=0;

    for(k=0;k<len;k++)
    {
        if(src[i]==0x0d && src[i+1]==0x0a)
        {
            i+=2;
            continue;
        }
        else
        {
            res[j]=src[i];
            j++;
            i++;
        }
    }
}

/**
 * @brief 发送信息给wifi模块
 * 
 * @param buffer    要发送的信息
 * @param len       要发送信息的长度
 */
void wifi_send_msg(uint8_t* buffer, uint16_t len)
{
  HAL_UART_Transmit(&UART_H, (const uint8_t*)buffer, len, 0xffff);
}

/**
 * @brief 发送一个指令，并对比期望指令
 * 
 * @param cmd   要发送的指令
 * @param exp   期望收到的指令
 * @return int8_t 返回0，成功；其他，失败
 */
int8_t send_instructions(char* cmd, char* exp)
{
    uint16_t len;

    memset(ins_rx, 0, sizeof(ins_rx));
    
    wifi_send_msg((uint8_t*)cmd, strlen(cmd));
    
    HAL_Delay(500);
    
    len = recv_reply(ins_rx);
    
    if(exp != NULL)
    {
       
        if(strstr((const char*)ins_rx, (const char*)exp) != NULL)
        {
            emlog(LOG_DEBUG, "jump: %s", ins_rx);
            return 0;
        }
        else 
        {
            emlog(LOG_WARN, "no exp: %s", ins_rx);
            return 1;
        }
    }
    else
    {
        if(len!=0)
        {
            emlog(LOG_DEBUG, "recv: %s", ins_rx);
            return 0;
        }
        else
        {
            emlog(LOG_DEBUG, "no recv\r\n");
            return 1;
        } 
        
    }
}


/**
 * @brief 查询配置信息并打印输出
 * 
 */
void query_config_info(void)
{   
    uint8_t tmp=1;
    while(check_is_cmd_mode());
    while(send_instructions(QUERY_RES, NULL)); 
    while(send_instructions(QUERY_VER, NULL)); 
    while(send_instructions(QUERY_UART, NULL)); 
    while(send_instructions(QUERY_NETMODE, NULL)); 
    while(send_instructions(QUERY_MODE, NULL)); 
    while(send_instructions(QUERY_DHCP, NULL)); 
    while(send_instructions(QUERY_MAC, NULL)); 
    while(send_instructions(QUERY_BLE_NAME, NULL));

    while(send_instructions(QUERY_NET_MODE, NULL)); 
    while(send_instructions(QUERY_WIFI_CONF, NULL)); 

    // do{
    //     tmp = connect_state();
    //     mylog("connecting...\r\n");
    //     HAL_Delay(200);
    // }while(tmp!=0);
    // mylog("connected!\r\n");
    while(tmp)
    {
        char* ptr = (char*)ins_buf;
        memset(ptr, 0x00, sizeof(ins_buf));
        strcpy(ptr, QUERY_WIFI_STA_CONNCET);
        // mylog("%s\r\n", ptr);
        tmp = send_instructions(ptr, "Connected");
    }
    

    while(send_instructions(QUERY_WIFI_STA_CONNCET, NULL)); 
    while(send_instructions(QUERY_NET_INFO, NULL)); 
    while(send_instructions(QUERY_TCP_MODE, NULL)); 
    while(send_instructions(QUERY_SCOKET_MODE, NULL)); 

    while(send_instructions(QUERY_LOCAL_PORT, NULL));
    while(send_instructions(QUERY_REMOTE_IP, NULL));
    while(send_instructions(QUERY_REMOTE_PORT, NULL));

    while(send_instructions(QUSERY_TCP_S_CONN_N, NULL));
    while(send_instructions(QUSERY_TCP_S_DISC_N, NULL));
    
    tmp=1;
    while(tmp)
    {
        char* ptr = (char*)ins_buf;
        memset(ptr, 0x00, sizeof(ins_buf));
        strcpy(ptr, SET_RESTART);
        // mylog("%s\r\n", ptr);
        tmp = send_instructions(ptr, "ok");
    }
}


/**
 * @brief 配置WiFi模块串口参数
 * 
 */
uint8_t config_wifi_uart()
{
    uint8_t tmp;
    char* ptr = (char*)ins_buf;
    memset(ins_buf, 0x00, sizeof(ins_buf));
    strcpy(ptr, SET_UART);
    strcat(ptr, BAUDRATE);
    strcat(ptr, ",");
    strcat(ptr, DATA_BIT);
    strcat(ptr, ",");
    strcat(ptr, CHECK_TP);
    strcat(ptr, ",");
    strcat(ptr, STOP_BIT);
    strcat(ptr, "\r\n");
    emlog(LOG_INFO, "%s\r\n", ptr);
    tmp = send_instructions(ptr, SET_UART); 
    return tmp;
}

/**
 * @brief  配置WiFi模块为STA模式
 * 
 * @return uint8_t 返回0成功，返回1失败
 */
uint8_t config_wifi_sta()
{
    uint8_t tmp = 0;
    char* ptr = (char*)ins_buf;
    memset(ins_buf, 0x00, sizeof(ins_buf));
    ptr += sprintf(ptr, SET_NET_MODE_STA);
    mylog("%s\r\n", ptr);
    tmp = send_instructions(ptr, "ok");
    if(tmp)
    {
        mylog("wifi sta-mode config error\r\n");
    }
    else
    {
        mylog("wifi as sta-mode\r\n");
    }
    return tmp;
}



/**
 * @brief 配置WiFi模块模式
 * 
 * @param get_ip_mode   获取IP的方式，静态配置或动态获取
 * @param socket_mode   socket模式，TCP 或 UDP
 * @param termianl_mode 终端模式，client 或 server
 * @return uint16_t     返回 0成功，1失败
 */
uint16_t config_device_mode(get_ip_mode_t get_ip_mode, socket_mode_t socket_mode, termianl_mode_t termianl_mode)
{
    uint16_t tmp=1;
    uint16_t cnt=0;
    /* 确定进入指令模式 */
    while(tmp)
    {   cnt++;
        tmp = check_is_cmd_mode();
        if(tmp)emlog(LOG_ERROR, "into cmd mode error. cnt=%d.\r\n",cnt);
        else emlog(LOG_INFO, "into cmd mode succ. cnt=%d.\r\n",cnt);
    }
    tmp=1;
    /* 配置串口波特率 */
    while(tmp)
    {
        tmp = config_wifi_uart();
        if(tmp)emlog(LOG_ERROR, "uart set error.\r\n");
        else emlog(LOG_INFO, "uart set succ.\r\n");
    }
    char* ptr = (char*)ins_buf;
    tmp = 1;
    /* 配置为sta模式 */
    while(tmp)
    {
        memset(ptr, 0x00, sizeof(ins_buf));
        strcpy(ptr, SET_NET_MODE_STA);
        // emlog(LOG_DEBUG, "%s\r\n", ptr);
        tmp=send_instructions(ptr, "ok");
        if(tmp)emlog(LOG_ERROR, "sta mode set error.\r\n");
        else emlog(LOG_INFO, "sta set succ.\r\n");
    }
    
    tmp = 1;
    /* 设置连接路由的名称、加密方式和密码 */
    while(tmp)
    {
        ptr = (char*)ins_buf;
        memset(ptr, 0x0, sizeof(ins_buf));
        ptr += sprintf(ptr, SET_WIFI_CONF);
        strcpy(ptr, SET_WIFI_CONF);
        strcat(ptr, WIFI_SSID);
        strcat(ptr, ",");
        strcat(ptr, ENCR_TYPE);
        strcat(ptr, ",");
        strcat(ptr, PASSWORD);
        strcat(ptr, "\r\n");
        // emlog(LOG_DEBUG, "%s\r\n",ptr);
        tmp=send_instructions(ptr, "ok");
        if(tmp)emlog(LOG_ERROR, "connect route set error.\r\n");
        else emlog(LOG_INFO, "connect route info set succ.\r\n");
    }
    
    tmp = 1;
    /* 配置为IP获取方式 */
    while(tmp)
    {
        ptr = (char*)ins_buf;
        memset(ptr, 0x0, sizeof(ins_buf));
        if(get_ip_mode == STATIC_MODE)
        {
            strcpy(ptr, SET_STATIC_IP);
        }
        else if(get_ip_mode == DHCP_MODE)
        {
            strcpy(ptr, SET_DYNAMIC_IP);
        }
        // emlog(LOG_DEBUG, "%s\r\n", ptr);
        tmp=send_instructions(ptr, "ok");
        if(tmp)emlog(LOG_ERROR, "ip set type error.\r\n");
        emlog(LOG_INFO, "ip get type set succ.\r\n");
    }
    
    if(get_ip_mode == STATIC_MODE)
    {
        /* 设置模块的IP等信息 */
        tmp = 1;
        while(tmp)
        {
            ptr = (char*)ins_buf;
            memset(ptr, 0x0, sizeof(ins_buf));
            strcpy(ptr, SET_NET_IP);
            strcat(ptr, DEVICE_IP);
            strcat(ptr, ",");
            strcat(ptr, DEVICE_SUB);
            strcat(ptr, ",");
            strcat(ptr, DEVICE_GET);
            strcat(ptr, "\r\n");
            // emlog(LOG_DEBUG, "%s\r\n",ptr);
            tmp=send_instructions(ptr, "ok");
            if(tmp)emlog(LOG_ERROR, "static ip set error.\r\n");
            else emlog(LOG_INFO, "set ip info succ.\r\n");
        }
        
    }
    
    /* 设置远端IP */
    tmp = 1;
    while(tmp)
    {
        ptr = (char*)ins_buf;
        memset(ptr, 0x0, sizeof(ins_buf));
        strcpy(ptr, SET_REMOTE_IP);
        strcat(ptr, REMOTE_IP);
        strcat(ptr, "\r\n");
        // emlog(LOG_DEBUG, "%s\r\n",ptr);
        tmp=send_instructions(ptr, "ok");
        if(tmp)emlog(LOG_ERROR, "remote ip set error.\r\n");
        else emlog(LOG_INFO, "set remote ip succ.\r\n");
    }
    

    /* 设置远程port */
    tmp = 1;
    while(tmp)
    {
        ptr = (char*)ins_buf;
        memset(ptr, 0x0, sizeof(ins_buf));
        strcpy(ptr, SET_REMOTE_PORT);
        strcat(ptr, REMOTE_PORT);
        strcat(ptr, "\r\n");
        // emlog(LOG_DEBUG, "%s\r\n",ptr);
        tmp=send_instructions(ptr, "ok");
        if(tmp)emlog(LOG_ERROR, "remote port set error.\r\n");
        else emlog(LOG_INFO, "set remote port succ.\r\n");
    }
    

    /* 设置TCP方式传输 */
    tmp = 1;
    while(tmp)
    {
        ptr = (char*)ins_buf;
        memset(ptr, 0x0, sizeof(ins_buf));
        if(socket_mode == TCP_MODE)
        {
            strcpy(ptr, SET_SCOKET_TCP);
        }
        else if(socket_mode == UDP_MODE)
        {
            strcpy(ptr, SET_SCOKET_UDP);
        }
        // emlog(LOG_DEBUG, "%s\r\n",ptr);
        tmp=send_instructions(ptr, "ok");
        if(tmp)emlog(LOG_ERROR, "scoekt tcp set error.\r\n");
        else emlog(LOG_INFO, "set tcp translat succ.\r\n");
    }
    

    /* 设置模块作为客户端 */
    tmp = 1;
    while(tmp)
    {
        ptr = (char*)ins_buf;
        memset(ptr, 0x0, sizeof(ins_buf));
        if(termianl_mode == CLIENT_MODE)
        {
            strcpy(ptr, SET_TCP_CLIENT);
        }
        else
        {
            strcpy(ptr, SET_TCP_SERVER);
        }
        // emlog(LOG_DEBUG, "%s\r\n",ptr);
        tmp=send_instructions(ptr, "ok");
        if(tmp)emlog(LOG_ERROR, "client mode set error.\r\n");
        else emlog(LOG_INFO, "set client mode succ.\r\n");

    }
    

    if(termianl_mode == CLIENT_MODE)
    {
        /* 设置本地端口 */
        tmp = 1;
        while(tmp)
        {
            ptr = (char*)ins_buf;
            memset(ptr, 0x0, sizeof(ins_buf));
            strcpy(ptr, SET_LOCAL_PORT);
            strcat(ptr, LOCAL_PORT);
            strcat(ptr, "\r\n");
            // emlog(LOG_DEBUG, "%s\r\n",ptr);
            tmp=send_instructions(ptr, "ok");
            if(tmp)emlog(LOG_ERROR, "local port set error.\r\n");
            else emlog(LOG_INFO, "set local port succ.\r\n");
        }
        
        if(socket_mode == TCP_MODE)
        {
            tmp = 1;
            while(tmp)
            {
                ptr = (char*)ins_buf;
                memset(ptr, 0x0, sizeof(ins_buf));
                strcpy(ptr, SET_TCP_S_CONN_N);
                strcat(ptr, TCP_S_CONN_NODE);
                strcat(ptr, "\r\n");
                // emlog(LOG_DEBUG, "%s\r\n",ptr);
                tmp = send_instructions(ptr, "ok");
                if(tmp)emlog(LOG_ERROR, "tcp server connect node set error.\r\n");
                else emlog(LOG_INFO, "tcp server connect node set succ.\r\n");
            }
            
            tmp = 1;
            while(tmp)
            {
                ptr = (char*)ins_buf;
                memset(ptr, 0x0, sizeof(ins_buf));
                strcpy(ptr, SET_TCP_S_DISC_N);
                strcat(ptr, TCP_S_DISC_NODE);
                strcat(ptr, "\r\n");
                // emlog(LOG_DEBUG, "%s\r\n",ptr);
                tmp = send_instructions(ptr, "ok");
                if(tmp)emlog(LOG_ERROR, "tcp server disconnect node set error.\r\n");
                else emlog(LOG_INFO, "tcp server disconnect node set succ.\r\n");
            }
        }
    }

    /* 提交参数 */
    tmp = 1;
    while(tmp)
    {
        ptr = (char*)ins_buf;
        memset(ptr, 0x0, sizeof(ins_buf));
        strcpy(ptr, SET_NET_COMMIT);
        // emlog(LOG_DEBUG, "%s\r\n",ptr);
        tmp=send_instructions(ptr, "ok");
        if(tmp)emlog(LOG_ERROR, "commit error.\r\n");
        else emlog(LOG_INFO, "commit succ.\r\n");
    }
    

    /* 重启模块 */
    tmp = 1;
    while (tmp)
    {
        ptr = (char*)ins_buf;
        memset(ptr, 0x0, sizeof(ins_buf));
        strcpy(ptr, SET_RESTART);
        // emlog(LOG_DEBUG, "%s\r\n",ptr);
        tmp=send_instructions(ptr, "ok");
        if(tmp)emlog(LOG_ERROR, "reboot error.\r\n");
        else emlog(LOG_INFO, "reconn succ.\r\n");
    }

    return tmp;
}

/**
 * @brief STA模式下的连接状态测试
 * 
 * @return uint8_t 返回0，成功；返回1，失败。
 */
uint8_t connect_state()
{
    uint8_t tmp = 0;
    check_is_cmd_mode();
    char* ptr = (char*)ins_buf;
    memset(ptr, 0x00, sizeof(ins_buf));
    strcpy(ptr, QUERY_WIFI_STA_CONNCET);
        // mylog("%s\r\n", ptr);
    tmp = send_instructions(ptr, "Connected");
    return tmp;
}

/**
 * @brief 设置蓝牙设备名字，优先英文字符
 * 
 * @param name 
 * @return uint8_t 成功返回0，返回其他失败
 */
uint8_t set_ble_name(char* name)
{
    uint8_t tmp = 0;
    char* ptr = (char*)ins_buf;
    memset(ptr, 0x00, sizeof(ins_buf));
    strcpy(ptr, SET_BLE_NAME);
    strcat(ptr, name);
    strcat(ptr, "\r\n");
        mylog("%s\r\n", ptr);
    tmp = send_instructions(ptr, "ok");
    return tmp;
} 


/**
 * @brief 进入指令模式
 * 
 */
static void into_cmd_mode(void)
{
    HAL_GPIO_WritePin(WIFI_RST_GPIO_Port, WIFI_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(WIFI_RST_GPIO_Port, WIFI_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(WIFI_ES0_GPIO_Port, WIFI_ES0_Pin, GPIO_PIN_RESET);
    HAL_Delay(40);
    HAL_GPIO_WritePin(WIFI_ES0_GPIO_Port, WIFI_ES0_Pin, GPIO_PIN_SET);
}

/**
 * @brief WiFi模块复位
 * 
 */
void wifi_reset(void)
{
    HAL_GPIO_WritePin(WIFI_ES0_GPIO_Port, WIFI_ES0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(WIFI_RST_GPIO_Port, WIFI_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(WIFI_RST_GPIO_Port, WIFI_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(1000);
}

/**
 * @brief 检测是否进入指令模式
 * 
 * @return uint8_t 0，成功；1，失败
 */
uint8_t check_is_cmd_mode()
{
    uint8_t tmp;
    char* ptr = NULL;
    do
    {
        // 控制进入指令模式
        // 必要重启模块
        // 复位后需立即拉低进入指令模式，否则会反复横跳，
        into_cmd_mode();
        ptr = (char*)ins_buf;
        memset(ptr, 0x0, sizeof(ins_buf));
        strcpy(ptr, QUERY_RES);
        tmp = send_instructions(ptr, "at");
    } while (tmp!=0);
    
    return tmp;
}


