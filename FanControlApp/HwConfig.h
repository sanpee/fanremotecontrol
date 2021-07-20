#ifndef _HWCONFIG_H_
#define _HWCONFIG_H_

#if defined(WIFI_Kit_32)
  #include <heltec.h>
  #define DISP_INIT() Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, false /*Serial Enable*/) 
#else 
  #include <U8g2lib.h>
  // Pins for OLED display, hardware dependent
  #define OLED_SCK    23
  #define OLED_SDA    22
  #define OLED_RES    19
  #define OLED_DC     18
  #define OLED_CS     17
  #define LED         16
  #define DISP_INIT() u8g2.begin() 
  U8G2_SH1106_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, OLED_SCK, OLED_SDA, OLED_CS, OLED_DC, OLED_RES );
#endif

#define RF_TX_DATA 27   // Heltec WIFI Kit32
#define RF_RX_DATA 35   // Heltec WIFI Kit32

#endif // _HWCONFIG_H_
