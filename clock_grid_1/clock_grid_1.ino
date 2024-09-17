/* clock_grid_1
   by Shinjia + Assistant
   - v1.5  2024/09/17
*/

#include <WiFi.h>
#include "time.h"
#include <ESP32Time.h>
#include "SPI.h"
#include "TFT_eSPI.h"

/****** WiFi AP ******/
#define WLAN_SSID    "Your_Wifi_SSID"
#define WLAN_PASS    "Your_Wifi_Password"

/***** Timer control (delay) *****/
int timer_delay_clock = 1000;  // 每秒更新時間
int timer_delay_NTP   = 60 * 60 * 1000;  // 每小時更新 NTP

unsigned long timer_next_clock = 0;
unsigned long timer_next_NTP = 0;

// RTC
ESP32Time rtc;

// NTP
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 8 * 3600;   // 台北時區
const int   daylightOffset_sec = 0; // 夏令時偏移

boolean is_get_NTP = false;

// TFT
TFT_eSPI tft = TFT_eSPI();

// 時鐘公用變數
int hh = 0, mm = 0, ss = 0;  // 時分秒
int prev_hh = -1, prev_mm = -1, prev_ss = -1; // 上一次的時分秒

// 定義格子大小和位置
int gridCols = 6;
int gridRows = 10;
int gridWidth = 320;
int gridHeight = 200;
int cellWidth = gridWidth / gridCols;    // 320 / 6 = 約53
int cellHeight = gridHeight / gridRows;  // 200 / 10 = 20

void draw_clock() {
  // 取得當前時間
  struct tm timeinfo = rtc.getTimeStruct();
  hh = timeinfo.tm_hour;
  mm = timeinfo.tm_min;
  ss = timeinfo.tm_sec;

  // 更新時、分、秒塊
  for (int i = 0; i < 60; i++) {
    // 需要更新的條件
    bool updateCell = false;

    // 判斷是否需要清除上一個狀態
    if (i == prev_hh || i == prev_mm || i == prev_ss) {
      updateCell = true;
    }
    // 判斷是否需要繪製新的狀態
    if (i == hh || i == mm || i == ss) {
      updateCell = true;
    }

    if (updateCell) {
      // 計算格子位置
      int col = i % gridCols;
      int row = i / gridCols;
      int x = col * cellWidth;
      int y = row * cellHeight;

      // 重新繪製格子背景，填充為黑色
      tft.fillRect(x + 1, y + 1, cellWidth - 2, cellHeight - 2, TFT_BLACK);

      // 繪製彩色填充，略小於格子尺寸，避免覆蓋格線
      int offset = 2; // 內縮2像素
      uint16_t fillColor = TFT_BLACK;

      // 確定該格子應該填充的顏色
      bool isHH = (i == hh);
      bool isMM = (i == mm);
      bool isSS = (i == ss);

      if (isHH && isMM && isSS) {
        fillColor = TFT_WHITE; // 紅+綠+藍=白
      } else if (isHH && isMM) {
        fillColor = TFT_YELLOW; // 紅+綠=黃
      } else if (isHH && isSS) {
        fillColor = TFT_MAGENTA; // 紅+藍=洋紅
      } else if (isMM && isSS) {
        fillColor = TFT_CYAN; // 綠+藍=青
      } else if (isHH) {
        fillColor = TFT_RED;
      } else if (isMM) {
        fillColor = TFT_GREEN;
      } else if (isSS) {
        fillColor = TFT_BLUE;
      }

      if (fillColor != TFT_BLACK) {
        tft.fillRect(x + offset, y + offset, cellWidth - 2 * offset, cellHeight - 2 * offset, fillColor);
      }

      // 繪製數字，背景顏色與填充顏色一致
      tft.setTextColor(TFT_WHITE, fillColor);
      tft.setTextDatum(MC_DATUM);
      tft.setTextFont(2);
      tft.drawString(String(i), x + cellWidth / 2, y + cellHeight / 2);
    }
  }

  // 更新前一次的時間記錄
  prev_hh = hh;
  prev_mm = mm;
  prev_ss = ss;

  // 更新時間顯示
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(4);
  tft.fillRect(0, tft.height() - 30, tft.width(), 30, TFT_BLACK); // 清除舊的時間顯示
  tft.drawString(rtc.getTime("%Y/%m/%d %H:%M:%S"), tft.width() / 2, tft.height() - 5);
}

void get_NTP_update_RTC() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    rtc.setTimeStruct(timeinfo);
  }
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
  tft.setRotation(3);  // 根據需要調整旋轉方向

  // 連接 WiFi
  wifi_connect_tft();

  // 從 NTP 獲取時間
  get_NTP_update_RTC();

  // 清除畫面
  tft.fillScreen(TFT_BLACK);

  // 初始化格子，繪製格線
  for (int i = 0; i < 60; i++) {
    int col = i % gridCols;
    int row = i / gridCols;

    int x = col * cellWidth;
    int y = row * cellHeight;

    // 繪製格線
    tft.drawRect(x, y, cellWidth, cellHeight, TFT_WHITE);
  }

  // 在繪製格線之後，顯示所有的數字
  for (int i = 0; i < 60; i++) {
    int col = i % gridCols;
    int row = i / gridCols;

    int x = col * cellWidth;
    int y = row * cellHeight;

    // 繪製數字
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextFont(2);
    tft.drawString(String(i), x + cellWidth / 2, y + cellHeight / 2);
  }
}

void loop() {
  // 若尚未取得網路時間
  if (!is_get_NTP) {
    get_NTP_update_RTC();
  }

  // 間隔一段時間要求重新取得網路時間 NTP
  if (millis() > timer_next_NTP) {
    timer_next_NTP = millis() + timer_delay_NTP;
    is_get_NTP = false;
  }

  // 間隔一段時間顯示時間 hh,mm,ss
  if (millis() > timer_next_clock) {
    timer_next_clock = millis() + timer_delay_clock;
    draw_clock();
  }

  // Do something
}
