#include "buzzer.h"
#include "staled.h"
#include "frame.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <Servo.h>
#include <DNSServer.h>


#define RELAY_PIN   12  // 继电器
#define STALED_PIN  16  // 状态灯
#define SENSOR_PIN  4   // 光电开关
#define ADC_PIN     0   // 电位器
#define STEN_PIN    14  // 舵机

#define DIVIDER_RATIO 6.0 // 
#define SAMPLE_COUNT 20   // ADC 获取次数

volatile bool keepaliveFlag = false;  // 保活标志
volatile bool opendoorFlag = false;   // 开门标志
volatile bool buzzPlayFlag = false;   // 开门成功后的鸣叫标志

unsigned int duration;                // 间隔时间
unsigned long opendoorTick = 0;       // 开门计时变量


unsigned int time_arr[5] = {15, 20, 30, 45, 60};  // 间隔时间表，单位秒
// 定时变量
unsigned long lastSampleTime = 0;
const unsigned long sampleInterval = 1000;  // 1秒

// WiFi 配置

String  ssid = "CMCC-503-2.4G";
String  password = "longing2021";

// 静态IP 设置（最重要部分）
IPAddress local_IP(192, 168, 1, 222);    // 你想要给ESP12的固定IP（要和路由器同网段，且没被占用）
IPAddress gateway(192, 168, 1, 1);       // 通常是路由器IP（网关）
IPAddress subnet(255, 255, 255, 0);       // 子网掩码，一般家用都是这个
IPAddress primaryDNS(192, 168, 1, 1);    // DNS 可写路由器IP，或用114.114.114.114 / 8.8.8.8
// IPAddress secondaryDNS(114, 114, 114, 114);  // 可选第二DNS

// MQTT 配置
// const char* mqtt_server = "broker.emqx.io";
// const int mqtt_port = 1883;
// const char* client_id = "esp32_client_001";
// const char* pub_topic = "test/esp32/pub";
// const char* sub_topic = "test/esp32/sub";

// mqtt
// WiFiClient espClient;
// PubSubClient client(espClient);

// void callback(char* topic, byte* payload, unsigned int length)
// {
//   Serial.print("收到消息 [");
//   Serial.print(topic);
//   Serial.print("]: ");
//   for (int i = 0; i < length; i++)
//     Serial.print((char)payload[i]);
//   Serial.println();
// }

// void reconnect()
// {
//   while (!client.connected())
//   {
//     Serial.print("连接 MQTT...");
//     if (client.connect(client_id))
//     {
//       Serial.println("成功");
//       client.subscribe(sub_topic);
//     }
//     else
//     {
//       Serial.print("失败, rc=");
//       Serial.println(client.state());
//       delay(2000);
//     }
//   }
// }

// http
ESP8266WebServer  server(80);
DNSServer dnsServer;
const  char* ap_ssid = "ESP_CONFIG";

void handleRoot()
{
  String html = "<html><body>";
  html += "<h2>WIFI Setup</h2>";
  html += "<form action='/save'>";
  html += "SSID:<input name='ssid'><br>";
  html += "PASS:<input name='pass'><br>";
  html += "<input type='submit'>";
  html += "</form>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}
void handleSave()
{
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");

    EEPROM.begin(128);

    for(int i=0;i<32;i++)
        EEPROM.write(i, ssid[i]);

    for(int i=0;i<64;i++)
        EEPROM.write(32+i, pass[i]);

    EEPROM.commit();

    server.send(200,"text/html","Saved. Rebooting...");

    delay(2000);
    ESP.restart();
}
void handleShow()
{
    String html = "<!DOCTYPE html>";
    html += "<html>";
    html += "<head>";
    html += "<meta charset='utf-8'>";
    html += "<title>ESP8266 Info</title>";
    html += "</head>";
    html += "<body>";

    html += "<h1>ESP8266 Device Info</h1>";

    html += "<p>SSID: ";
    html += WiFi.SSID();
    html += "</p>";

    html += "<p>SubnetMask: ";
    html += WiFi.gatewayIP().toString();
    html += "</p>";

    html += "<p>IP Address: ";
    html += WiFi.localIP().toString();
    html += "</p>";

    
    html += "<p>SubnetMask: ";
    html += WiFi.subnetMask().toString();
    html += "</p>";

    

    html += "</body></html>";

    server.send(200, "text/html", html);
}

void readWiFi(String &ssid,String &pass)
{
    EEPROM.begin(128);

    char s[32];
    char p[64];

    for(int i=0;i<32;i++)
        s[i]=EEPROM.read(i);

    for(int i=0;i<64;i++)
        p[i]=EEPROM.read(32+i);

    ssid = String(s);
    pass = String(p);
}
bool connectWiFi()
{
    String ssid,pass;

    readWiFi(ssid,pass);

    if(ssid.length()==0)
        return false;

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(),pass.c_str());

    int timeout=0;

    while(WiFi.status()!=WL_CONNECTED)
    {
        delay(500);
        timeout++;

        if(timeout>20)
            return false;
    }

    return true;
}
void startAP()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid);
  dnsServer.start(53, "*", WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.begin();
}

// void handleRoot() {
//   server.send(200, "text/plain", "Hello from ESP8266!");
// }

// void handleOpenDoor() {
//   // 这里加开门逻辑，例如控制继电器
//   if (!server.hasArg("duration")) {
//     server.send(400, "text/plain", "Missing duration");
//     return;
//   }
//   unsigned int dur;
//   dur = server.arg("duration").toInt();  // ms

//   if (dur <= 0 || dur > 60000) dur = 10000; // 默认10秒，最大60秒防止继电器烧毁
//   Serial.println(dur);
//   opendoorFlag = true;
//   server.send(200, "text/plain", "Door Opened!");
// }

// void handleLockDoor(){
//   opendoorFlag = false;
//   digitalWrite(RELAY_PIN, LOW);
//   digitalWrite(LED_BUILTIN, HIGH);
//   server.send(200, "text/plain", "Door Locked!");
// }

// UDP
WiFiUDP udp;
WiFiServer tcpServer(9000);
WiFiClient tcpClient;

IPAddress remoteIP;
uint16_t  remotePort;

#define UDP_PORT 9000

void send_to_wifi(uint8_t* buff, unsigned int len)
{
    /* UDP 回发 */
    udp.beginPacket(remoteIP, remotePort);
    udp.write(buff, len);
    udp.endPacket();

    /* TCP 回发（如果有连接） */
    if(tcpClient && tcpClient.connected()){
        tcpClient.write(buff, len);
    }
}

void udp_poll()
{
    int len = udp.parsePacket();
    if(len <= 0) return;

    remoteIP   = udp.remoteIP();
    remotePort = udp.remotePort();

    while(len--){
        frame_uart_input(udp.read());
    }
}

// tcp 
void tcp_poll()
{
    if(!tcpClient || !tcpClient.connected()){
        tcpClient = tcpServer.available();
        return;
    }

    while(tcpClient.available()){
        frame_uart_input(tcpClient.read());
    }
}

// get adc
float readADC()
{
  long sum = 0;

  for (int i = 0; i < SAMPLE_COUNT; i++)
  {
    sum += analogRead(ADC_PIN);
    // delay(2);
  }

  return sum / (float)SAMPLE_COUNT;
}

// exti
void IRAM_ATTR sensorISR(){
  opendoorFlag = true;
}

// WIFI reconnect
void wifi_check(){
  static unsigned long t = 0;
  if(millis() - t > 5000){
    t = millis();

    if(WiFi.status() != WL_CONNECTED){
      WiFi.reconnect();
      Serial.println("WiFi 重连中...");
    }
  }
}



// strring engine 舵机
Servo myservo;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // 初始化状态灯、继电器、光电开关、舵机
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(
    digitalPinToInterrupt(SENSOR_PIN),
    sensorISR,
    FALLING          // HIGH → LOW，物体进入
  );
  myservo.attach(STEN_PIN, 500, 2500);
  myservo.write(0);// 0°

  // 初始化蜂鸣器和状态灯
  buzzerInit();
  ledInit(STALED_PIN);
  ledSetMode(LED_BLINK_FAST);
  

  
  // 连接WiFi
  WiFi.mode(WIFI_STA);        // 工作在 Station 模式（连路由器）
  readWiFi(ssid, password);   // 读取WiFi信息
  WiFi.begin(ssid, password); // 开始连接WiFi
  // 配置网络信息
  if(connectWiFi())
  {
    Serial.println("WIFI Connected");
  }else{
    startAP();
  }
  // 设置静态IP
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
    Serial.println("STA 静态IP 配置失败！");
  }

  // Serial.println("\nWiFi 已连接");
  // Serial.println("\n连接成功！");
  // Serial.print("IP 地址: ");
  // Serial.println(WiFi.localIP());       // 应该显示你设置的 192.168.31.188
  // Serial.print("子网掩码: ");
  // Serial.println(WiFi.subnetMask());
  // Serial.print("网关: ");
  // Serial.println(WiFi.gatewayIP());
  // musicStart();

  // mqtt ---
  // client.setServer(mqtt_server, mqtt_port);
  // client.setCallback(callback);

  // http
  // server.on("/open", handleOpenDoor);
  // server.on("/lock", handleLockDoor);
  server.on("/", handleRoot);
  server.on("/show", handleShow);
  server.begin();

  // udp & tcp
  udp.begin(UDP_PORT);
  tcpServer.begin();
  
  // ota
  
  // ArduinoOTA.setPort(8888);
  // ArduinoOTA.setHostname("myesp8266");
  // ArduinoOTA.setPassword("MENG");
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
  ArduinoOTA.begin();

  // mDNS
  if(MDNS.begin("lycx")){
    MDNS.addService("http","tcp", 80);
  }
  

  buzzerStop();
}

void loop() {
  // put your main code here, to run repeatedly:
  buzzerTask();   // ★ 必须周期性调用
  // musicTask();
  ledUpdate();

  // adc测试
  unsigned long currentMillis = millis();

  // 判断是否到达 1 秒
  if (currentMillis - lastSampleTime >= sampleInterval)
  {
    lastSampleTime = currentMillis;

    float raw = readADC();

    float adcVoltage = raw * (1.0 / 1023.0);
    float realVoltage = adcVoltage * DIVIDER_RATIO;

    // Serial.println("-------------");
    // Serial.print("Raw ADC: ");
    // Serial.println(raw);
    
    if(raw<=588)duration=time_arr[0];
    else if(raw<=665)duration=time_arr[1];
    else if(raw<=742)duration=time_arr[2];
    else if(raw<=819)duration=time_arr[3];
    else duration=time_arr[4];
    
    // Serial.print("A0 Voltage: ");
    // Serial.print(adcVoltage, 4);
    // Serial.println(" V");

    // Serial.print("Real Voltage: ");
    // Serial.print(realVoltage, 4);
    // Serial.println(" V");


  }
  // buzzerPlayStartup();
  // buzzerPlaySuccess();
  // buzzerPlayFail();
  // buzzerPlayWarning();
  // musicStart();


  // mqtt ----
  // if (!client.connected())
  //   reconnect();

  // client.loop();
  // static unsigned long last = 0;
  // if (millis() - last > 3000)
  // {
  //   last = millis();
  //   client.publish(pub_topic, "Hello MQTT");
  // }


  // http
  server.handleClient();
  dnsServer.processNextRequest();

  // udp & tcp
  udp_poll();
  tcp_poll();
  // ota
  ArduinoOTA.handle();
  wifi_check();
  MDNS.update();

  // 心跳包保活
  if(keepaliveFlag){
    keepaliveFlag = false;
    //  Serial.println("♥");
  }else{
    //
  }

  // 轮询开门标志
  if(opendoorFlag){
    // 解锁
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(LED_BUILTIN, LOW);
    myservo.write(180);

    if(buzzPlayFlag == false){
      buzzPlayFlag = true;
      Serial.println("\n已开门\n");
      Serial.println(duration);
      buzzerPlaySuccess();
    }
    if(buzzPlayFlag == true){
      unsigned long buzzCurrentTimeCnt = millis();
      if(millis() - buzzCurrentTimeCnt >= 5000){
        buzzerStop();
      }
    }
  
    
   if(opendoorTick == 0){
      opendoorTick = millis();
    }

    // 判断是否超过 n 秒
    if(millis() - opendoorTick >= duration * 1000){
      // 超时锁定
        opendoorFlag = false;
        buzzPlayFlag = false;
        opendoorTick = 0;           // 重置计时
        digitalWrite(RELAY_PIN, LOW);
        digitalWrite(LED_BUILTIN, HIGH);
        myservo.write(0);
        buzzerStop();
        Serial.println("\n已锁定\n");
    }
  
  }else{
    // 直接锁定
    opendoorTick = 0;  
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
    myservo.write(0);
    buzzPlayFlag = false;
    buzzerStop();
  }
  
}
