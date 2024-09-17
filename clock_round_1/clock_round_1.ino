/* clock_round_1
  by Shinjia
  - v1.2  2024/09/17  - Init, 重新程式, 基本的時鐘畫面
*/

/****** WiFi AP ******/
#define WLAN_SSID    "Your_Wifi_SSID"
#define WLAN_PASS    "Your_Wifi_Password"

/***** Timer control (delay) *****/
int timer_delay_clock = 1000;  // 每秒更新時間
int timer_delay_NTP   = 60*60*1000;  // 每小時更新 NTP

unsigned long timer_next_clock;
unsigned long timer_next_NTP;

// RTC
#include <ESP32Time.h>
ESP32Time rtc;

// NTP
#include <WiFi.h>
#include "time.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 8 * 3600;   // for Taipei
const int   daylightOffset_sec = 0; // 3600?

boolean is_get_NTP = false;

// TFT
#include "SPI.h"
#include "TFT_eSPI.h"

TFT_eSPI tft = TFT_eSPI();

// 時鐘公用變數
int hh=0, mm=0, ss=0;  // 時分秒
int hh0=-1, mm0=-1, ss0=-1;  // 保留前次畫圖時的數值 (初始值必須設為-1)


void draw_clock_analog() {  
  // 宜設為參數
  int xc = 160;  // 圓心x
  int yc = 120;  // 圓心y 
  int hh_len =  70;  // 時針的長度
  int mm_len =  90;  // 分針的長度
  int ss_len = 110;  // 秒針的長度

  // 注意：鐘面數值的 hh 要用 12 小時制
  float deg_ss, deg_mm, deg_hh;  // 時分秒的角度值
  float x_hh, y_hh, x_mm, y_mm, x_ss, y_ss;  // 時分秒針的頂點位置

  // 清除前次的指針(時、分、秒)
  deg_ss = ss0*6;                  // 每秒6度
  deg_mm = mm0*6 + deg_ss*0.01666667;  // 1/60 (角度含秒)
  deg_hh = (hh0%12)*30 + deg_mm*0.0833333;  // 1/12 (角度含分及秒)
  
  x_hh = xc + hh_len*cos((deg_hh-90)*0.0174532925);    
  y_hh = yc + hh_len*sin((deg_hh-90)*0.0174532925);
  x_mm = xc + mm_len*cos((deg_mm-90)*0.0174532925);    
  y_mm = yc + mm_len*sin((deg_mm-90)*0.0174532925);
  x_ss = xc + ss_len*cos((deg_ss-90)*0.0174532925);    
  y_ss = yc + ss_len*sin((deg_ss-90)*0.0174532925);

  // draw face
  tft.fillCircle(xc, yc, 116, TFT_BLACK);  // 內部清除
  tft.drawCircle(xc, yc, 118, TFT_DARKGREY);  // 圓形外框

  // Draw Center
  tft.drawLine(xc, yc, x_hh, y_hh, TFT_RED);
  tft.drawLine(xc, yc, x_mm, y_mm, TFT_GREEN);
  tft.drawLine(xc, yc, x_ss, y_ss, TFT_BLUE);
}


void update_time() {
  struct tm timeinfo = rtc.getTimeStruct();
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  
  hh = timeinfo.tm_hour;
  mm = timeinfo.tm_min;
  ss = timeinfo.tm_sec;
  
  if(hh0==-1 && mm0==-1 && ss0==-1) {
    hh0 = hh;
    mm0 = mm;
    ss0 = ss;
  }

  // Draw Analog Clock (pointer)
  draw_clock_analog();
    
  // 保留舊的值
  hh0 = hh;
  mm0 = mm;
  ss0 = ss;
}


void get_NTP_update_RTC() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)){      
    rtc.setTimeStruct(timeinfo); 
    is_get_NTP = true;
  }
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}


void wifi_connect_tft() {
  Serial.printf("Connecting to %s ", WLAN_SSID);
  tft.printf("Connecting to %s ", WLAN_SSID);
  
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      tft.print(".");
  }
  Serial.println(" CONNECTED");
  tft.println(" CONNECTED");
}


void setup() {
  Serial.begin(115200);

  // TFT
  tft.begin();
  tft.fillScreen(TFT_BLUE); 
  tft.setRotation(3);  
  
  // 有需要，可設定 LED 顯示亮度  
  // pinMode(TFT_LED, OUTPUT);
  // analogWrite(TFT_LED, TFT_LED_BRIGHTNESS);

  // Waitting for WiFi connect
  wifi_connect_tft();

  //init and get the time
  get_NTP_update_RTC();
  
  // 清除 TFT 畫面
  tft.fillScreen(TFT_BLACK);  // Clear
}


void loop() {
  // 若尚未取得網路時間
  if(!is_get_NTP) {
    get_NTP_update_RTC();
  }

  // 間隔一段時間要求重新取得網路時間 NTP
  if(millis()>timer_next_NTP) {
    timer_next_NTP = millis() + timer_delay_NTP;
    is_get_NTP = false;
  }
  
  // 間隔一段時間顯示時間 hh,mm,ss
  if(millis()>timer_next_clock) {
    timer_next_clock = millis() + timer_delay_clock;
    update_time();
  }

  // Do something
}
