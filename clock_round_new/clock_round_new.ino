/* clock_round_new
  by Shinjia + ChatGPT
  - v１.0  2024/09/17  - 時針為紅色帶圓圈，分針和秒針無圓圈，僅顯示數字
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

#define TFT_LIGHTBLUE 0x7D7C  // 定義淺藍色

// 時鐘公用變數
int hh=0, mm=0, ss=0;  // 時分秒
int hh0=-1, mm0=-1, ss0=-1;  // 保留前次畫圖時的數值 (初始值必須設為-1)

// 圓心座標與半徑
int xc = 160;  // 圓心x
int yc = 120;  // 圓心y
int radius = 100;  // 鐘面半徑

// 數字的位置座標
int num_pos_x[13];  // 1~12，索引從1開始
int num_pos_y[13];

// 預先計算好數字的位置
void calculate_number_positions() {
  for(int num=1; num<=12; num++) {
    float angle = (num - 3) * 30 * 0.0174532925; // 將每個數字轉換成角度，並轉為弧度
    num_pos_x[num] = xc + radius * cos(angle);
    num_pos_y[num] = yc + radius * sin(angle);
  }
}

// 畫出鐘面與數字
void draw_clock_face() {
  // 畫外圈
  tft.drawCircle(xc, yc, radius + 2, TFT_DARKGREY);
  tft.drawCircle(xc, yc, radius + 4, TFT_DARKGREY);

  // 畫數字
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);  // 中心對齊
  tft.setTextSize(2);

  for(int num=1; num<=12; num++) {
    tft.drawString(String(num), num_pos_x[num], num_pos_y[num]);
  }
}

// 前一次指針的端點座標
float hh_x0, hh_y0, mm_x0, mm_y0, ss_x0, ss_y0;
float hh_px0, hh_py0;

// 本次指針的端點座標
float hh_x, hh_y, mm_x, mm_y, ss_x, ss_y;
float hh_px, hh_py;

// 清除前一次的繪圖
void clear_previous() {
  // 清除前一次的秒針
  tft.drawLine(xc, yc, ss_x0, ss_y0, TFT_BLACK);
  // 清除前一次的秒數字
  tft.setTextColor(TFT_BLACK, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.drawString(String(ss0), ss_x0, ss_y0);

  // 清除前一次的分針
  tft.drawLine(xc, yc, mm_x0, mm_y0, TFT_BLACK);
  // 清除前一次的分數字
  tft.setTextColor(TFT_BLACK, TFT_BLACK);
  tft.drawString(String(mm0), mm_x0, mm_y0);

  // 清除前一次的時針和其圓圈
  tft.drawLine(xc, yc, hh_x0, hh_y0, TFT_BLACK);
  tft.fillCircle(hh_px0, hh_py0, 12, TFT_BLACK);
  // 清除前一次的時數字
  tft.setTextColor(TFT_BLACK, TFT_BLACK);
  tft.drawString(String(hh0 % 12 == 0 ? 12 : hh0 % 12), hh_px0, hh_py0);

  // 重畫被清除區域內的鐘面（避免殘影）
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  // 重畫數字（如果圓圈位置有數字）
  for(int num=1; num<=12; num++) {
    if(abs(mm_x0 - num_pos_x[num]) < 7 && abs(mm_y0 - num_pos_y[num]) < 7) {
      tft.drawString(String(num), num_pos_x[num], num_pos_y[num]);
    }
    if(abs(ss_x0 - num_pos_x[num]) < 7 && abs(ss_y0 - num_pos_y[num]) < 7) {
      tft.drawString(String(num), num_pos_x[num], num_pos_y[num]);
    }
    if(abs(hh_px0 - num_pos_x[num]) < 7 && abs(hh_py0 - num_pos_y[num]) < 7) {
      tft.drawString(String(num), num_pos_x[num], num_pos_y[num]);
    }
  }
}

// 畫出新的指針和標記
void draw_current() {
  // 畫時針圓圈（紅色）
  tft.fillCircle(hh_px, hh_py, 12, TFT_RED);
  // 畫時針
  tft.drawLine(xc, yc, hh_x, hh_y, TFT_RED);
  // 在時針圓圈上顯示時數
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.drawString(String(hh % 12 == 0 ? 12 : hh % 12), hh_px, hh_py);

  // 畫分針
  tft.drawLine(xc, yc, mm_x, mm_y, TFT_GREEN);
  // 在分針末端顯示分鐘數字（綠色）
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString(String(mm), mm_x, mm_y);

  // 畫秒針
  tft.drawLine(xc, yc, ss_x, ss_y, TFT_LIGHTBLUE);
  // 在秒針末端顯示秒數字（淺藍色）
  tft.setTextColor(TFT_LIGHTBLUE, TFT_BLACK);
  tft.drawString(String(ss), ss_x, ss_y);
}

void update_time() {
  struct tm timeinfo = rtc.getTimeStruct();
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  hh = timeinfo.tm_hour;
  mm = timeinfo.tm_min;
  ss = timeinfo.tm_sec;

  // 計算角度（度數），時針不含小數點，停留在整數位置
  int display_hh = hh % 12;
  if(display_hh == 0) display_hh = 12;
  float angle_hh = display_hh * 30; // 每小時30度

  float angle_mm = (mm + ss / 60.0) * 6;  // 每分鐘6度
  float angle_ss = ss * 6;  // 每秒6度

  // 時針長度保持到達圓周數字的位置
  float hh_length = radius;  // 時針長度，達到鐘面外圓
  float mm_length = hh_length * 0.75;  // 分針短一點，不壓到時針圓圈
  float ss_length = mm_length * 0.75;  // 秒針更短，不壓到分針數字

  // 計算指針端點座標
  hh_x = xc + hh_length * cos((angle_hh - 90) * 0.0174532925);
  hh_y = yc + hh_length * sin((angle_hh - 90) * 0.0174532925);

  mm_x = xc + mm_length * cos((angle_mm - 90) * 0.0174532925);
  mm_y = yc + mm_length * sin((angle_mm - 90) * 0.0174532925);

  ss_x = xc + ss_length * cos((angle_ss - 90) * 0.0174532925);
  ss_y = yc + ss_length * sin((angle_ss - 90) * 0.0174532925);

  // 時針圓圈位置
  hh_px = hh_x;
  hh_py = hh_y;

  // 如果不是第一次運行，清除前一次的繪圖
  if(hh0 != -1) {
    clear_previous();
  }

  // 畫出新的指針和標記
  draw_current();

  // 更新前一次的值
  hh0 = hh;
  mm0 = mm;
  ss0 = ss;

  hh_x0 = hh_x;
  hh_y0 = hh_y;
  mm_x0 = mm_x;
  mm_y0 = mm_y;
  ss_x0 = ss_x;
  ss_y0 = ss_y;

  hh_px0 = hh_px;
  hh_py0 = hh_py;
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

  // TFT 初始化
  tft.init();
  tft.fillScreen(TFT_BLACK); 
  tft.setRotation(3);  

  // 連接 WiFi
  wifi_connect_tft();

  // 取得 NTP 時間並更新 RTC
  get_NTP_update_RTC();

  // 清除 TFT 畫面
  tft.fillScreen(TFT_BLACK);  // 清除畫面

  // 計算數字的位置
  calculate_number_positions();

  // 畫出鐘面與數字
  draw_clock_face();

  // 設定下一次更新時間
  timer_next_clock = millis() + timer_delay_clock;
  timer_next_NTP = millis() + timer_delay_NTP;
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
