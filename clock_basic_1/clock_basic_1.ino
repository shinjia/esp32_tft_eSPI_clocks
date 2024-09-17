/* clock_basic_1
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


void draw_clock_digital() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  
  // Draw Text
  char StringBuff[50]; //50 chars should be enough
  
  tft.setTextSize(3);
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);

  // Line 1
  strftime(StringBuff, sizeof(StringBuff), " %H:%M:%S ", &timeinfo);
  tft.setCursor(60, 80);
  tft.println(StringBuff);

  // Line2
  strftime(StringBuff, sizeof(StringBuff), "%Y-%m-%d", &timeinfo);
  tft.setCursor(60, 120);
  tft.println(StringBuff);  
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
  
  // 如有需要，可設定 LED 顯示亮度  
  // pinMode(TFT_LED, OUTPUT);
  // analogWrite(TFT_LED, TFT_LED_BRIGHTNESS);

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
    draw_clock_digital();
  }

  // Do something
}
