/* colck_pic_prepare by Shinjia
  - v1.1  2024/05/10 --- update LED Brightness
  - v0.1  2024/02/05
*/

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

void setup() {

  // 設定 LED 顯示亮度  
  pinMode(TFT_LED, OUTPUT);
  analogWrite(TFT_LED, TFT_LED_BRIGHTNESS);

  tft.init();
  tft.setRotation(3);
  tft.setSwapBytes(true);  // 修正位元順序，將 RGB 轉成 BGR (for pushImage)
  
  // Clear
  tft.fillScreen(TFT_BLACK);

  Serial.begin(115200);
}

void loop(void) {
  // get data
  int value = random(0, 9999);
  int v1 = int(value / 1000);
  int v2 = int((value%1000) / 100);
  int v3 = int((value%100) / 10);
  int v4 = value % 10;

  Serial.print(value);  Serial.print("\t");
  Serial.print(v1);  Serial.print("\t");
  Serial.print(v2);  Serial.print("\t");
  Serial.print(v3);  Serial.print("\t");
  Serial.print(v4);  Serial.println();

  tft.pushImage(  0,  0, 64, 64, num1);
  tft.pushImage( 64,  0, 64, 64, num2);
  tft.pushImage(128,  0, 64, 64, num3);
  tft.pushImage(192,  0, 64, 64, num4);
  tft.pushImage(256,  0, 64, 64, num5);
  tft.pushImage(  0, 64, 64, 64, num6);
  tft.pushImage( 64, 64, 64, 64, num7);
  tft.pushImage(128, 64, 64, 64, num8);
  tft.pushImage(192, 64, 64, 64, num9);
  tft.pushImage(256, 64, 64, 64, num0);

  switch(v1) {
    case 0: tft.pushImage( 32, 140, 64, 64, num1); break;
    case 1: tft.pushImage( 32, 140, 64, 64, num2); break;
    case 2: tft.pushImage( 32, 140, 64, 64, num3); break;
    case 3: tft.pushImage( 32, 140, 64, 64, num4); break;
    case 4: tft.pushImage( 32, 140, 64, 64, num5); break;
    case 5: tft.pushImage( 32, 140, 64, 64, num6); break;
    case 6: tft.pushImage( 32, 140, 64, 64, num7); break;
    case 7: tft.pushImage( 32, 140, 64, 64, num8); break;
    case 8: tft.pushImage( 32, 140, 64, 64, num9); break;
    case 9: tft.pushImage( 32, 140, 64, 64, num0); break;
  }

  switch(v2) {
    case 0: tft.pushImage( 96, 140, 64, 64, num1); break;
    case 1: tft.pushImage( 96, 140, 64, 64, num2); break;
    case 2: tft.pushImage( 96, 140, 64, 64, num3); break;
    case 3: tft.pushImage( 96, 140, 64, 64, num4); break;
    case 4: tft.pushImage( 96, 140, 64, 64, num5); break;
    case 5: tft.pushImage( 96, 140, 64, 64, num6); break;
    case 6: tft.pushImage( 96, 140, 64, 64, num7); break;
    case 7: tft.pushImage( 96, 140, 64, 64, num8); break;
    case 8: tft.pushImage( 96, 140, 64, 64, num9); break;
    case 9: tft.pushImage( 96, 140, 64, 64, num0); break;
  }

  switch(v3) {
    case 0: tft.pushImage(160, 140, 64, 64, num1); break;
    case 1: tft.pushImage(160, 140, 64, 64, num2); break;
    case 2: tft.pushImage(160, 140, 64, 64, num3); break;
    case 3: tft.pushImage(160, 140, 64, 64, num4); break;
    case 4: tft.pushImage(160, 140, 64, 64, num5); break;
    case 5: tft.pushImage(160, 140, 64, 64, num6); break;
    case 6: tft.pushImage(160, 140, 64, 64, num7); break;
    case 7: tft.pushImage(160, 140, 64, 64, num8); break;
    case 8: tft.pushImage(160, 140, 64, 64, num9); break;
    case 9: tft.pushImage(160, 140, 64, 64, num0); break;
  }

  switch(v4) {
    case 0: tft.pushImage(224, 140, 64, 64, num1); break;
    case 1: tft.pushImage(224, 140, 64, 64, num2); break;
    case 2: tft.pushImage(224, 140, 64, 64, num3); break;
    case 3: tft.pushImage(224, 140, 64, 64, num4); break;
    case 4: tft.pushImage(224, 140, 64, 64, num5); break;
    case 5: tft.pushImage(224, 140, 64, 64, num6); break;
    case 6: tft.pushImage(224, 140, 64, 64, num7); break;
    case 7: tft.pushImage(224, 140, 64, 64, num8); break;
    case 8: tft.pushImage(224, 140, 64, 64, num9); break;
    case 9: tft.pushImage(224, 140, 64, 64, num0); break;
  } 

  delay(1000);
}