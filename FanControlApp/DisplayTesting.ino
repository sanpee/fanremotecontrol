#include "HwConfig.h"

void DisplayTestingInit() {
  DISP_INIT();
}

#if defined(WIFI_Kit_32)
void DisplayTesting() {
  uint8_t buf[8]={ 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F };
  Heltec.display->clear();  
  Heltec.display->drawRect(0,0,8,8);
  Heltec.display->drawFastImage(0,0,8,8,buf);
  Heltec.display->display();  
}
#else
void DisplayTesting() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    u8g2.drawStr(0,10,"Hello World!");  // write something to the internal memory
    u8g2.drawBox(0,10,128,54);
  }
  while(u8g2.nextPage());
}
#endif


