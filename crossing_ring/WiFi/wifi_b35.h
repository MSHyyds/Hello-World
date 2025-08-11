#ifndef __WIFI_B35_H
#define __WIFI_B35_H

#include "stm32f1xx_hal.h"
#include "usart.h"
#include "ringbuffer.h"
#include "main.h"


#define UART_H          huart2
// #define WIFI_ES0_PORT   WIFI_ES0_GPIO_Port
// #define WIFI_ES0_PIN    WIFI_ES0_Pin
// #define WIFI_RST_PORT   WIFI_RST_GPIO_Port
// #define WIFI_RST_PIN    WIFI_RST_Pin

// STA模式下为连接路由/WiFi的ssid、加密方式、pwd
// AP模式下为设置建立WiFi热点的ssid，加密方式 和 pwd
#define WIFI_SSID       "longing"           // 长度最大不超过32个字节
#define ENCR_TYPE       "WPA/WPA2-PSK"              // WPA/WPA2-PSK
#define PASSWORD        "longing2021"         // 长度最大不超过32个字节



typedef enum _get_ip_mode_t{
    STATIC_MODE=0,
    DHCP_MODE,
}get_ip_mode_t;

typedef enum _socket_mode_t{
    TCP_MODE = 0,
    UDP_MODE = 1,
}socket_mode_t;

typedef enum _termianl_mode_t{
    CLIENT_MODE = 0,
    SERVER_MODE,
}termianl_mode_t;

typedef struct _wifi_connect_route_info_t
{
    uint8_t route_name[32];
    uint8_t encr_type[16];
    uint8_t password[32];
}wifi_connect_route_info_t;

typedef struct _wifi_device_net_info_t
{
    uint8_t dev_ip[4];
    uint8_t dev_sub[4];
    uint8_t dev_gat[4];
    uint8_t remote_ip[4];
    uint16_t remote_port;
}wifi_device_net_info_t;

// wifi模块的串口配置信息
#define BAUDRATE        "115200"     // 原厂测试底板最高支持921600，受pcb布局影响
#define DATA_BIT        "8"          // 数据位
#define CHECK_TP        "n"          // 偶校验：add, 奇校验：even, 无校验：n
#define STOP_BIT        "1"          // 1,  1.5,  2

   

#define QUERY_RES       "at\r\n"
#define QUERY_VER       "at+ver=?\r\n"
#define QUERY_UART      "at+uart=?\r\n"
#define QUERY_NETMODE   "at+netmode=?\r\n"
#define QUERY_MODE      "at+mode=?\r\n"
#define QUERY_DHCP      "at+dhcpc=?\r\n"

#define QUERY_MAC       "at+Get_MAC=?\r\n"
#define SET_UART        "at+uart="
#define SET_DEF         "at+default=1\r\n"      // 恢复出厂设置
#define SET_NET_COMMIT  "at+net_commit=1\r\n"   // 提交配置信息
#define SET_RESTART     "at+reconn=1\r\n"      // 执行过提交操作进入重启，否则进入透传

// sta or ap
#define SET_NET_MODE_STA        "at+netmode=2\r\n"
#define SET_NET_MODE_AP         "at+netmode=3\r\n"
#define QUERY_NET_MODE          "at+netmode=?\r\n"
#define SET_WIFI_CONF           "at+wifi_conf="
#define QUERY_WIFI_CONF         "at+wifi_conf=?\r\n"
#define QUERY_WIFI_STA_CONNCET  "at+wifi_ConState=?\r\n"
#define QUERY_WIFI_STA_SCAN     "at+scanap=1\r\n"

// net info
#define SET_STATIC_IP   "at+dhcpc=0\r\n"      // =0,设置为静态IP， =1，设置为动态IP
#define SET_DYNAMIC_IP  "at+dhcpc=1\r\n"
#define SET_NET_IP      "at+net_ip="         // 有效前提：at+dhcpc=0 已经执行, AP 模式下，模块的默认地址为192.168.169.1，且不可设置

#define DEVICE_IP       "192.168.1.2"
#define DEVICE_SUB      "225.225.224.0"
#define DEVICE_GET      "192.168.1.1"
#define REMOTE_IP       "192.168.0.2"       // 对外连接服务器的IP
#define REMOTE_PORT     "9000"              // 对外连接服务器的端口



#define QUERY_NET_INFO  "at+net_ip=?\r\n"
#define SET_TCP_CLIENT  "at+mode=client\r\n"
#define SET_TCP_SERVER  "at+mode=server\r\n"
#define QUERY_TCP_MODE  "at+mode=?\r\n"
#define SET_SCOKET_TCP  "at+remotepro=tcp\r\n"
#define SET_SCOKET_UDP      "at+remotepro=udp\r\n"
#define QUERY_SCOKET_MODE   "at+remotepro=?\r\n"

// client and server
#define SET_LOCAL_PORT      "at+CLport="            // 有效前提：at+mode=client 已被执行
#define LOCAL_PORT          "8000"
#define QUERY_LOCAL_PORT    "at+CLport=?\r\n"
#define SET_REMOTE_IP       "at+remoteip="
#define QUERY_REMOTE_IP     "at+remoteip=?\r\n"
#define SET_REMOTE_PORT     "at+remoteport="         // 为客户端时此为连接的远端端口，为服务器时此为本地端口
#define QUERY_REMOTE_PORT   "at+remoteport=?\r\n"

// bletooth ref
#define QUERY_BLE_NAME      "at+ble_name=?\r\n"     // 查询蓝牙名字
#define SET_BLE_NAME        "at+ble_name="



extern uint8_t rx_complt;       // 在main.c中定义
extern uint8_t rx_complt_bit_cnt;


extern int mylog(char *format, ...);


void wifi_send_msg(uint8_t* buffer, uint16_t len);
int8_t send_instructions(char* cmd, char* exp);
void cond_msg(uint8_t* src,uint8_t* res, uint16_t len);
void query_config_info(void);
uint8_t config_wifi_uart(void);
uint8_t check_is_cmd_mode(void);

uint8_t connect_state(void);
uint16_t config_device_mode(get_ip_mode_t get_ip_mode, socket_mode_t socket_mode, termianl_mode_t termianl_mode);

#endif
