/* clock_basic_2
  by Shinjia
  - v1.2  2024/09/17  - 舊版誤存錯亂，重新整理
  - v1.1  2024/05/09  - update LED Brightness 
  - v1.0  2024/02/06  - for TFT_eSPI Library
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


void draw_clock() {
  struct tm timeinfo = rtc.getTimeStruct();
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  
  // Draw Clock  
  char StringBuff[50]; //50 chars should be enough
  strftime(StringBuff, sizeof(StringBuff), "%Y/%m/%d  %H:%M:%S", &timeinfo);

  // draw text
  tft.setCursor(40, 50);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print(StringBuff);  

  // draw mark of clock
  //             ----------------------------------------------------
  String strH = "  0   1   2   3   4   5   6   7   8   9  10  11  12 ";
  String strM = "  0   5  10  15  20  25  30  35  40  45  50  55  60 ";
  tft.setTextSize(1);
  tft.setCursor(0, 120);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println(strH);

  tft.setTextSize(1);
  tft.setCursor(0, 170);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println(strM);

  // 清除舊的
  int x_hh, x_mm, x_ss;

  x_hh = map(hh0%12, 0, 11, 10, 300);  // 十二小時制
  x_mm = map(mm0, 0, 59, 10, 300);
  x_ss = map(ss0, 0, 59, 10, 300);
  
  if(hh!=hh0) { tft.fillCircle(x_hh, 100, 10, TFT_BLACK); }
  if(mm!=mm0) { tft.fillCircle(x_mm, 150, 10, TFT_BLACK); }
  tft.fillCircle(x_ss, 200, 10, TFT_BLACK); 

  // 畫新的
  x_hh = map(hh%12, 0, 11, 10, 300);  // 十二小時制
  x_mm = map(mm, 0, 59, 10, 300);
  x_ss = map(ss, 0, 59, 10, 300);
  tft.fillCircle(x_hh, 100, 10, TFT_RED);
  tft.fillCircle(x_mm, 150, 10, TFT_GREEN);
  tft.fillCircle(x_ss, 200, 10, TFT_BLUE); 

  // 畫橫線
  for(int i=-2; i<2; i++) {
    tft.drawFastHLine(10, 100+i, 300, TFT_RED);
    tft.drawFastHLine(10, 150+i, 300, TFT_GREEN);
    tft.drawFastHLine(10, 200+i, 300, TFT_BLUE);
  }
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

  // Draw Digital Clock
  draw_clock();
  
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
  }
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  
  is_get_NTP = true;
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
  
  // 如需要，可設定 LED 顯示亮度  
  pinMode(TFT_LED, OUTPUT);
  analogWrite(TFT_LED, TFT_LED_BRIGHTNESS);

  // Waitting for WiFi connect
  wifi_connect_tft();

  //init and get the time
  get_NTP_update_RTC();
  
  tft.fillScreen(TFT_BLACK); 
  
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  is_get_NTP = false;
  
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
  }

  // Do something
}
