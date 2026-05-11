#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266_Seniverse.h> // 引入太极创客的心知天气库

/* ================== 配置区域 ================== */
const char* ssid       = "14";             // WiFi 名称
const char* password   = "wn0512210126";         // WiFi 密码

// 心知天气API请求所需信息
String reqUserKey = "SqvsUjCPm8Wy5dcSW";         // 心知天气私钥
String reqLocation = "chengdu";                   // 目标城市拼音
String reqUnit = "c";                             // 摄氏度

/* ================== 对象与变量 ================== */
WiFiUDP ntpUDP;
// 配置阿里云 NTP 服务器，东八区(8 * 3600 = 28800 秒)，内部每 60 秒更新一次
NTPClient timeClient(ntpUDP, "ntp.aliyun.com", 28800, 60000); 

// 建立 WeatherNow 对象用于获取心知天气信息
WeatherNow weatherNow;
Forecast forecast; // 【新增】天气预报对象

unsigned long lastWeatherTime = 0;
const unsigned long weatherInterval = 1800000;    // 天气请求间隔：30分钟 (毫秒)

unsigned long lastTimeSync = 0;
const unsigned long timeSyncInterval = 3600000;   // 向STM32同步时间间隔：1小时 (毫秒)

/* ================== 函数声明 ================== */
void fetchAndSendWeather();
void sendTimeToSTM32();

void setup() {
  // 与 STM32 通信的波特率必须保持一致
  Serial.begin(115200);
  Serial.println(); // 先打印个空行防乱码
  Serial.print("正在尝试连接 WiFi: ");
  Serial.println(ssid);
  
  // 1. 连接 WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("."); // 没连上就一直打印点
  }

  Serial.println("\nWiFi 连接成功！");
  Serial.print("ESP8266 的 IP 地址是: ");
  Serial.println(WiFi.localIP());

  // 2. 启动 NTP 客户端获取时间
  timeClient.begin();
  timeClient.update();

  // 3. 配置心知天气请求信息
  weatherNow.config(reqUserKey, reqLocation, reqUnit);
  forecast.config(reqUserKey, reqLocation, reqUnit); // 【新增】配置预报

  // 4. 开机立即向 STM32 发送一次时间和天气数据
  sendTimeToSTM32();
  fetchAndSendWeather();
}

void loop() {
  // 保持 NTP 客户端后台自动更新
  timeClient.update();

  // 获取当前系统运行毫秒数
  unsigned long currentMillis = millis();

// ================= 【新增】任务 0：监听 STM32 的主动请求 =================
  if (Serial.available() > 0) {
    String req = Serial.readStringUntil('\n');
    // 如果收到了 STM32 发来的同步指令
    if (req.indexOf("#SYNC") >= 0) {
      // 强制立刻下发一次时间和天气
      sendTimeToSTM32();
      fetchAndSendWeather();
      // 重置定时器，避免马上又触发定时发送
      lastTimeSync = currentMillis;
      lastWeatherTime = currentMillis;
    }
  }

  // ================= 任务 1：定时下发时间帧 =================
  if (currentMillis - lastTimeSync >= timeSyncInterval) {
    lastTimeSync = currentMillis;
    sendTimeToSTM32();
  }

  // ================= 任务 2：定时请求并下发天气帧 =================
  if (currentMillis - lastWeatherTime >= weatherInterval) {
    lastWeatherTime = currentMillis;
    fetchAndSendWeather();
  }
}

/* ================== 获取时间并发送至 STM32 ================== */
void sendTimeToSTM32() {
  // 【关键修复】使用标准的 64 位 time_t 类型来接收时间戳，防止内存指针溢出读取垃圾数据
  time_t epochTime = (time_t)timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);
  
  // 提取年月日时分秒
  int yy = ptm->tm_year % 100; // 仅取后两位
  int MM = ptm->tm_mon + 1;
  int dd = ptm->tm_mday;
  int hh = timeClient.getHours();
  int mm = timeClient.getMinutes();
  int ss = timeClient.getSeconds();

  // 按照我们设计的自定义协议发给 STM32 (格式: #TIME:26,03,27,12,30,00\r\n)
  Serial.printf("#TIME:%02d,%02d,%02d,%02d,%02d,%02d\r\n", yy, MM, dd, hh, mm, ss);
}

/* ================== 获取天气并发送至 STM32 ================== */
void fetchAndSendWeather() {
  if (WiFi.status() != WL_CONNECTED) return;

  // 调用太极创客库中的 update() 函数，向服务器请求更新天气数据
  if(weatherNow.update()) {  
    // 提取核心数据：天气代码和温度
    int code = weatherNow.getWeatherCode();
    int temp = weatherNow.getDegree();
    
    // 按照自定义协议发给 STM32 (格式: #WEAT:04,25\r\n)
    Serial.printf("#WEAT:%02d,%d\r\n", code, temp);
  }

  // --- 【新增】获取并发送天气预报 ---
  if (forecast.update()) {
    int t0_code = forecast.getDayCode(0); // 今天
    int t0_h = forecast.getHigh(0);       
    int t0_l = forecast.getLow(0);        
    
    int t1_code = forecast.getDayCode(1); // 明天
    int t1_h = forecast.getHigh(1);       
    int t1_l = forecast.getLow(1);        

    int t2_code = forecast.getDayCode(2); // 后天
    int t2_h = forecast.getHigh(2);       
    int t2_l = forecast.getLow(2);        
    
    // 按新协议发送给 STM32 (格式变为9个参数)
    Serial.printf("#FCST:%02d,%d,%d,%02d,%d,%d,%02d,%d,%d\r\n", 
                  t0_code, t0_h, t0_l, 
                  t1_code, t1_h, t1_l,
                  t2_code, t2_h, t2_l);
    }
}
