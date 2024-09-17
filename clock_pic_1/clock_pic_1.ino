/* clock_pic_1  by Shinjia
  - v1.1  2024/05/10 --- update LED Brightness
  - v0.1  2024/02/06
*/

/****** WiFi AP ******/
#define WLAN_SSID    "Your_Wifi_SSID"
#define WLAN_PASS    "Your_Wifi_Password"

/***** Timer control (delay) *****/
int timer_delay_clock = 1000;  // 每秒更新畫面時間
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
#include "graphics_0.h"
#include "graphics_1.h"
#include "graphics_2.h"
#include "graphics_3.h"
#include "graphics_4.h"
#include "graphics_5.h"
#include "graphics_6.h"
#include "graphics_7.h"
#include "graphics_8.h"
#include "graphics_9.h"

#include "SPI.h"
#include "TFT_eSPI.h"

TFT_eSPI tft = TFT_eSPI();

// 時鐘公用變數
int hh=0, mm=0, ss=0;  // 時分秒
int hh0=-1, mm0=-1, ss0=-1;  // 保留前次畫圖時的數值 (初始值必須設為-1)


void draw_clock_picture() {
  // get each digit number of hh, mm, ss

  int h1 = int(hh / 10);
  int h2 = hh % 10;
  int m1 = int(mm/10);
  int m2 = mm % 10;
  int s1 = int(ss/10);
  int s2 = ss % 10;

  switch(h1) {
    case 0: tft.pushImage( 20, 28, 64, 64, num0); break;
    case 1: tft.pushImage( 20, 28, 64, 64, num1); break;
    case 2: tft.pushImage( 20, 28, 64, 64, num2); break;
    case 3: tft.pushImage( 20, 28, 64, 64, num3); break;
    case 4: tft.pushImage( 20, 28, 64, 64, num4); break;
    case 5: tft.pushImage( 20, 28, 64, 64, num5); break;
    case 6: tft.pushImage( 20, 28, 64, 64, num6); break;
    case 7: tft.pushImage( 20, 28, 64, 64, num7); break;
    case 8: tft.pushImage( 20, 28, 64, 64, num8); break;
    case 9: tft.pushImage( 20, 28, 64, 64, num9); break;
  }

  switch(h2) {
    case 0: tft.pushImage( 84, 28, 64, 64, num0); break;
    case 1: tft.pushImage( 84, 28, 64, 64, num1); break;
    case 2: tft.pushImage( 84, 28, 64, 64, num2); break;
    case 3: tft.pushImage( 84, 28, 64, 64, num3); break;
    case 4: tft.pushImage( 84, 28, 64, 64, num4); break;
    case 5: tft.pushImage( 84, 28, 64, 64, num5); break;
    case 6: tft.pushImage( 84, 28, 64, 64, num6); break;
    case 7: tft.pushImage( 84, 28, 64, 64, num7); break;
    case 8: tft.pushImage( 84, 28, 64, 64, num8); break;
    case 9: tft.pushImage( 84, 28, 64, 64, num9); break;
  }

  switch(m1) {
    case 0: tft.pushImage(168, 28, 64, 64, num0); break;
    case 1: tft.pushImage(168, 28, 64, 64, num1); break;
    case 2: tft.pushImage(168, 28, 64, 64, num2); break;
    case 3: tft.pushImage(168, 28, 64, 64, num3); break;
    case 4: tft.pushImage(168, 28, 64, 64, num4); break;
    case 5: tft.pushImage(168, 28, 64, 64, num5); break;
    case 6: tft.pushImage(168, 28, 64, 64, num6); break;
    case 7: tft.pushImage(168, 28, 64, 64, num7); break;
    case 8: tft.pushImage(168, 28, 64, 64, num8); break;
    case 9: tft.pushImage(168, 28, 64, 64, num9); break;
  }

  switch(m2) {
    case 0: tft.pushImage(232, 28, 64, 64, num0); break;
    case 1: tft.pushImage(232, 28, 64, 64, num1); break;
    case 2: tft.pushImage(232, 28, 64, 64, num2); break;
    case 3: tft.pushImage(232, 28, 64, 64, num3); break;
    case 4: tft.pushImage(232, 28, 64, 64, num4); break;
    case 5: tft.pushImage(232, 28, 64, 64, num5); break;
    case 6: tft.pushImage(232, 28, 64, 64, num6); break;
    case 7: tft.pushImage(232, 28, 64, 64, num7); break;
    case 8: tft.pushImage(232, 28, 64, 64, num8); break;
    case 9: tft.pushImage(232, 28, 64, 64, num9); break;
  }
  
  switch(s1) {
    case 0: tft.pushImage(96, 140, 64, 64, num0); break;
    case 1: tft.pushImage(96, 140, 64, 64, num1); break;
    case 2: tft.pushImage(96, 140, 64, 64, num2); break;
    case 3: tft.pushImage(96, 140, 64, 64, num3); break;
    case 4: tft.pushImage(96, 140, 64, 64, num4); break;
    case 5: tft.pushImage(96, 140, 64, 64, num5); break;
    case 6: tft.pushImage(96, 140, 64, 64, num6); break;
    case 7: tft.pushImage(96, 140, 64, 64, num7); break;
    case 8: tft.pushImage(96, 140, 64, 64, num8); break;
    case 9: tft.pushImage(96, 140, 64, 64, num9); break;
  }
  
  switch(s2) {
    case 0: tft.pushImage(160, 140, 64, 64, num0); break;
    case 1: tft.pushImage(160, 140, 64, 64, num1); break;
    case 2: tft.pushImage(160, 140, 64, 64, num2); break;
    case 3: tft.pushImage(160, 140, 64, 64, num3); break;
    case 4: tft.pushImage(160, 140, 64, 64, num4); break;
    case 5: tft.pushImage(160, 140, 64, 64, num5); break;
    case 6: tft.pushImage(160, 140, 64, 64, num6); break;
    case 7: tft.pushImage(160, 140, 64, 64, num7); break;
    case 8: tft.pushImage(160, 140, 64, 64, num8); break;
    case 9: tft.pushImage(160, 140, 64, 64, num9); break;
  }

}


void update_time() {
  struct tm timeinfo = rtc.getTimeStruct();
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  
  hh = timeinfo.tm_hour;
  mm = timeinfo.tm_min;
  ss = timeinfo.tm_sec;
  
  if(hh0==-1 && mm0==-1 && ss0==-1) {
    hh0 = hh;
    mm0 = mm;
    ss0 = ss;
  }

  // Draw Clock using picture
  draw_clock_picture();
  
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

  // 設定 LED 顯示亮度  
  pinMode(TFT_LED, OUTPUT);
  analogWrite(TFT_LED, TFT_LED_BRIGHTNESS);

  // 先啟動 tft
  tft.init();
  tft.setSwapBytes(true);  // 修正位元順序，將 RGB 轉成 BGR (for pushImage)
  tft.setRotation(3);
  tft.fillScreen(TFT_BLUE);

  // Waitting for WiFi connect
  wifi_connect_tft();

  //init and get the time
  get_NTP_update_RTC();

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  is_get_NTP = false;
  
  // timer
  timer_next_clock = millis();
  timer_next_NTP = millis();

  // Clear  
  tft.fillScreen(TFT_BLACK);
  // disconnect WiFi as it's no longer needed
  // WiFi.disconnect(true);
  // WiFi.mode(WIFI_OFF);
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

    // draw_clock_picture();
  }

  // Do something
  // draw colon
  if(millis()%1000<800) {
    tft.fillCircle(160-3+1, 28+32-10, 6, TFT_CYAN);
    tft.fillCircle(160-3+1, 28+32+10, 6, TFT_CYAN);
  }
  else {
    tft.fillCircle(160-3+1, 28+32-10, 6, TFT_BLACK);
    tft.fillCircle(160-3+1, 28+32+10, 6, TFT_BLACK);
  }
}
