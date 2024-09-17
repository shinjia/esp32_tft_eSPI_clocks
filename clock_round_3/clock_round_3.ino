/* clock_round_3
  by Shinjia
  - v1.2  2024/09/17 --- 重整各程式一致
  - v1.1  2024/05/10 --- update LED Brightness
  - v1.0  2024/02/06 --- TFT_eSPI Library
*/

/****** WiFi AP ******/
#define WLAN_SSID    "Your_Wifi_SSID"
#define WLAN_PASS    "Your_Wifi_Password"

/****** 數字時鐘的參數定義 (長度) ******/
#define C_xc       100  // 時鐘圓心 x
#define C_yc       120  // 時鐘圓心 y
#define C_size     180  // 時鐘的寬高 (直徑)
#define C_border1   96  // 外圓框(外)
#define C_border2   84  // 外圓框(內)
#define C_radius1   88  // 12個點(時)外
#define C_radius2   76  // 12個點(時)內
#define C_radius3   84  // 60個點(分)外
#define C_radius4   78  // 60個點(分)內
#define C_radius5   86  // 4個點(上下左右)的位置半徑
#define C_quadrant   6  // 4個點(上下左右)圓半徑
#define C_center     8  // 時鐘圓心半徑
#define hh_len      50  // 時針的長度
#define mm_len      65  // 分針的長度
#define ss_len      70  // 秒針的長度
#define hh_trilen    8  // 時針的長度(三角形往外擴)
#define mm_trilen    6  // 分針的長度(三角形往外擴)

/****** 數字時鐘的參數定義 (顏色) ******/
#define CC_Outer  TFT_BLACK
#define CC_Inner  TFT_CYAN
#define CC_Border TFT_RED
#define CC_Dot12  TFT_BLUE
#define CC_Dot60  TFT_BLUE
#define CC_Dot4   TFT_BLUE
#define CC_Center TFT_RED
#define CC_HH     TFT_BLUE
#define CC_MM     TFT_NAVY
#define CC_SS     TFT_BLACK

/***** Timer control (delay) *****/
int timer_delay_clock = 1000;  // 每秒更新時間
int timer_delay_NTP   = 60*60*1000;  // 每小時更新 NTP

unsigned long timer_next_clock;
unsigned long timer_next_NTP;

// ESP32Time
#include <ESP32Time.h>

ESP32Time rtc;

// NTP
#include <WiFi.h>
#include "time.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 8 * 3600;   // for Taipei
const int   daylightOffset_sec = 0; // 3600?

// TFT
#include "SPI.h"
#include "TFT_eSPI.h"

TFT_eSPI tft = TFT_eSPI();


// 時鐘公用變數
int hh=0, mm=0, ss=0;  // 時分秒
int hh0=-1, mm0=-1, ss0=-1;  // 保留前次畫圖時的數值 (初始值必須設為-1)
boolean is_get_NTP = false;


void draw_clock_face() {
  float th_cos, th_sin;
  float x1, y1, x2, y2;
    
  // Draw clock border
  tft.fillRect(C_xc-C_size/2, C_yc-C_size/2, C_size, C_size, CC_Outer);
  tft.fillCircle(C_xc, C_yc, C_border1, CC_Border);
  tft.fillCircle(C_xc, C_yc, C_border2, CC_Inner);

  // Draw 12 lines (hours markers)
  for(int angle=0; angle<360; angle+=30) {
    th_cos = cos((angle-90)*0.0174532925);  // (pi/180)
    th_sin = sin((angle-90)*0.0174532925);
    x1 = C_xc + C_radius1 * th_cos;
    y1 = C_yc + C_radius1 * th_sin;
    x2 = C_xc + C_radius2 * th_cos;
    y2 = C_yc + C_radius2 * th_sin;
    
    tft.drawLine(x1, y1, x2, y2, CC_Dot12);
  }

  // Draw 60 dots (minute markers)
  for(int angle=0; angle<360; angle+=6) {
    th_cos = cos((angle-90)*0.0174532925);  // (pi/180)
    th_sin = sin((angle-90)*0.0174532925);
    x1 = C_xc + C_radius3 * th_cos;
    y1 = C_yc + C_radius3 * th_sin;
    x2 = C_xc + C_radius4 * th_cos;
    y2 = C_yc + C_radius4 * th_sin;    
    tft.drawLine(x1, y1, x2, y2, CC_Dot60);
    
    // Draw main quadrant dots
    x1 = C_xc + C_radius5 * th_cos;
    y1 = C_yc + C_radius5 * th_sin;
    if(angle%90==0) tft.fillCircle(x1, y1, C_quadrant, CC_Dot4);
  }

  // Draw Center
  tft.fillCircle(C_xc, C_yc, C_center, CC_Center);
}


void draw_clock_pointer() {  
  //Draw Analog Clock
  // 注意：鐘面數值的 hh 要用 12 小時制
  float deg_ss, deg_mm, deg_hh;  // 時分秒的角度值
  float x_hh, y_hh, x_mm, y_mm, x_ss, y_ss;  // 時分秒針的頂點位置
  float x_h3a, y_h3a, x_m3a, y_m3a;
  float x_h3b, y_h3b, x_m3b, y_m3b;

  // 清除前次的指針(時、分、秒)
  deg_ss = ss0*6;                  // 每秒6度
  // deg_mm = mm0*6 + deg_ss*0.01666667;  // 1/60 (角度含秒)
  deg_mm = mm0*6;  // 為了避免每秒更新造成的閃礫，放棄秒數的微小角度，如此便可以每分鐘更新一次
  deg_hh = (hh0%12)*30 + deg_mm*0.0833333;  // 1/12 (角度含分及秒)
  
  x_hh = C_xc + hh_len*cos((deg_hh-90)*0.0174532925);    
  y_hh = C_yc + hh_len*sin((deg_hh-90)*0.0174532925);
  x_mm = C_xc + mm_len*cos((deg_mm-90)*0.0174532925);    
  y_mm = C_yc + mm_len*sin((deg_mm-90)*0.0174532925);
  x_ss = C_xc + ss_len*cos((deg_ss-90)*0.0174532925);    
  y_ss = C_yc + ss_len*sin((deg_ss-90)*0.0174532925);
  
  x_h3a = C_xc + hh_trilen*cos((deg_hh-90+90)*0.0174532925);
  y_h3a = C_yc + hh_trilen*sin((deg_hh-90+90)*0.0174532925);
  x_h3b = C_xc + hh_trilen*cos((deg_hh-90-90)*0.0174532925);
  y_h3b = C_yc + hh_trilen*sin((deg_hh-90-90)*0.0174532925);
  
  x_m3a = C_xc + mm_trilen*cos((deg_mm-90+90)*0.0174532925);
  y_m3a = C_yc + mm_trilen*sin((deg_mm-90+90)*0.0174532925);
  x_m3b = C_xc + mm_trilen*cos((deg_mm-90-90)*0.0174532925);
  y_m3b = C_yc + mm_trilen*sin((deg_mm-90-90)*0.0174532925);

  // 分的部分有改變時，同時更新分針及時針
  if(mm0!=mm) {
    tft.fillTriangle(x_hh, y_hh, x_h3a, y_h3a, x_h3b, y_h3b, CC_Inner);
    tft.fillTriangle(x_mm, y_mm, x_m3a, y_m3a, x_m3b, y_m3b, CC_Inner);
  }
  // tft.drawLine(C_xc, C_yc, x_hh, y_hh, CC_Inner);
  // tft.drawLine(C_xc, C_yc, x_mm, y_mm, CC_Inner);
  tft.drawLine(C_xc, C_yc, x_ss, y_ss, CC_Inner);
  
  // 畫出此次的指針(時、分、秒)
  deg_ss = ss*6;                  // 每秒6度
  // deg_mm = mm*6 + deg_ss*0.01666667;  // 1/60 (角度含秒)
  deg_mm = mm*6;  // 放棄秒的微小角度
  deg_hh = (hh%12)*30 + deg_mm*0.0833333;  // 1/12 (角度含分及秒)
  
  x_hh = C_xc + hh_len*cos((deg_hh-90)*0.0174532925);    
  y_hh = C_yc + hh_len*sin((deg_hh-90)*0.0174532925);
  x_mm = C_xc + mm_len*cos((deg_mm-90)*0.0174532925);    
  y_mm = C_yc + mm_len*sin((deg_mm-90)*0.0174532925);
  x_ss = C_xc + ss_len*cos((deg_ss-90)*0.0174532925);    
  y_ss = C_yc + ss_len*sin((deg_ss-90)*0.0174532925);

  x_h3a = C_xc + hh_trilen*cos((deg_hh-90+90)*0.0174532925);
  y_h3a = C_yc + hh_trilen*sin((deg_hh-90+90)*0.0174532925);
  x_h3b = C_xc + hh_trilen*cos((deg_hh-90-90)*0.0174532925);
  y_h3b = C_yc + hh_trilen*sin((deg_hh-90-90)*0.0174532925);

  x_m3a = C_xc + mm_trilen*cos((deg_mm-90+90)*0.0174532925);
  y_m3a = C_yc + mm_trilen*sin((deg_mm-90+90)*0.0174532925);
  x_m3b = C_xc + mm_trilen*cos((deg_mm-90-90)*0.0174532925);
  y_m3b = C_yc + mm_trilen*sin((deg_mm-90-90)*0.0174532925);

  tft.fillTriangle(x_hh, y_hh, x_h3a, y_h3a, x_h3b, y_h3b, CC_HH);
  tft.fillTriangle(x_mm, y_mm, x_m3a, y_m3a, x_m3b, y_m3b, CC_MM);
//  tft.drawLine(C_xc, C_yc, x_hh, y_hh, CC_HH);
//  tft.drawLine(C_xc, C_yc, x_mm, y_mm, CC_MM);
  tft.drawLine(C_xc, C_yc, x_ss, y_ss, CC_SS);

  // 圓心
  tft.fillCircle(C_xc, C_yc, C_center, CC_Center);
}


void draw_clock_digital() {
  struct tm timeinfo = rtc.getTimeStruct();
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  // Draw Digital Clock
  char StringBuff[50]; //50 chars should be enough

  // line1 (H:i:s)
  strftime(StringBuff, sizeof(StringBuff), "%H:%M:%S", &timeinfo);
  tft.setCursor(202, 180);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println(StringBuff);  
    
  // line2 (Y/m/d)
  strftime(StringBuff, sizeof(StringBuff), "%Y/%m/%d", &timeinfo);  
  tft.setCursor(190, 210);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println(StringBuff);
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
  draw_clock_pointer();
  
  // Draw Digital Clock
  draw_clock_digital();

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

  // TFT 先啟用
  tft.begin();
  tft.fillScreen(TFT_BLUE);   // Clear
  tft.setRotation(3);

  // 如有需要，可設定 LED 顯示亮度  
  // pinMode(TFT_LED, OUTPUT);
  // analogWrite(TFT_LED, TFT_LED_BRIGHTNESS);

  // RTC
  rtc.setTime(30, 24, 15, 17, 1, 2021);  // 亂設
  
  // Waitting for WiFi connect
  wifi_connect_tft();
   
  //init and get the time
  get_NTP_update_RTC();

  timer_next_clock = millis();
  
  // 清除 TFT 畫面
  tft.fillScreen(TFT_BLACK);   // Clear

  // 畫出時鐘表面
  draw_clock_face();
  
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
