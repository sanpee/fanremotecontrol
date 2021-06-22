#include <qrcode.h>
#include "HwConfig.h"

#define QR_VER      2

static byte flipByte(uint8_t c){
  uint8_t r=0;
  for(byte i = 0; i < 8; i++){
    r <<= 1;
    r |= c & 1;
    c >>= 1;
  }
  return r;
}

void Display_Init() {
  DISP_INIT();
}

void Display_Clear() {
//#if defined(WIFI_Kit_32)
  Heltec.display->clear();
// #endif 
}

void Display_DrawText(int x, int y,String text) {
#if defined(WIFI_Kit_32)
  Heltec.display->setFont(ArialMT_Plain_10);
  // The coordinates define the left starting point of the text
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->drawString(x,y,text);
  Heltec.display->display();
#endif
}

void Diplay_QRCode(const char* Text, int x_offset) {
  QRCode qrcode;
  uint8_t qrcodeBytes[qrcode_getBufferSize(QR_VER)];
  char s[128];
      
  qrcode_initText(&qrcode, qrcodeBytes, QR_VER, ECC_LOW, Text);
  int bytesperrow = (qrcode.size+8)/8;
  int margin = bytesperrow*8-qrcode.size;
  
  snprintf(s, sizeof(s), "QR buffer size = %d bytes", qrcode_getBufferSize(QR_VER));
  // Serial.println(s);
  snprintf(s, sizeof(s), "QR size = %d x %d", qrcode.size, qrcode.size);
  // Serial.println(s);
  snprintf(s, sizeof(s), "bytes per line: %d", bytesperrow );
  // Serial.println(s);  
  
  int bitcount = 0;
  int n, bitoffset;
  
  uint8_t *qrBuffer = (uint8_t *)malloc(qrcode.size*qrcode.size*8);
  memset( qrBuffer, 0xFF, qrcode.size*qrcode.size*8 );
  for (uint8_t y = 0; y < qrcode.size; y++) { // Row
    for (uint8_t x = 0; x < qrcode.size; x++) { // Column
        
        n = y*bytesperrow+(x/8);
        bitoffset = x%8;
        
        if (qrcode_getModule(&qrcode, x, y)) {
          qrBuffer[n] &= (~( 0x80 >> bitoffset)); // Little Endian 
          // Serial.print("#");
        }
        else {
          // Serial.print(" ");  
        }
    }
    snprintf(s, sizeof(s), "[n=%d]", n);
    // Serial.println(s);  
  }  

/*
  disp.firstPage();
  do {
    disp.drawBox( 0, 0, qrcode.size+margin*2, qrcode.size+margin*2 );
    disp.drawBitmap( margin, margin, bytesperrow, qrcode.size, qrBuffer );
  } while(disp.nextPage());
  */

  uint16_t bytesperrow2;
  uint8_t scale = 2;
  uint8_t *qrBuffer2 = ResizeImage(qrBuffer, qrcode.size, qrcode.size, bytesperrow, scale, &bytesperrow2);
  int image_w = bytesperrow2*8; // align to bytes
  int image_h = qrcode.size*scale;
  int margin2 = image_w-qrcode.size*scale;
  snprintf(s, sizeof(s), "Final image size[%dx%d] %d", image_w, image_h, margin2 );
  // Serial.println(s);  

#if defined(WIFI_Kit_32)
  for (int i=0; i<bytesperrow2*image_h;i ++) {
    qrBuffer2[i] = flipByte(qrBuffer2[i]);
  }
  Heltec.display->clear();
  for (int i=1;i<=margin2;i++) {
    Heltec.display->drawRect(x_offset+margin2-i, margin2-i, image_h+i*2, image_h+i*2);  
  }
  Heltec.display->drawFastImage(x_offset + margin2, margin2, image_w, image_h, qrBuffer2);
  // Heltec.display->drawXbm(x_offset + margin2, margin2, image_w, image_h, qrBuffer2);
  Heltec.display->display();
#else      
  u8g2.firstPage();
  do {
    // disp.drawBox( 0, 0, qrcode.size+margin*2, qrcode.size+margin*2 );
    // disp.drawBitmap( margin, margin, bytesperrow, qrcode.size, qrBuffer );
    u8g2.drawBox( x_offset, 0, qrcode.size*scale+margin2*2, qrcode.size*scale+margin2*2 );
    u8g2.drawBitmap( x_offset + margin2, margin2, bytesperrow2, qrcode.size*scale, qrBuffer2 );  
  } while(u8g2.nextPage());
#endif

  free(qrBuffer2);
  free(qrBuffer);
}

/*
  Remember to free return buffer
*/
uint8_t* ResizeImage(uint8_t *pImageBuf, uint16_t w, uint16_t h, uint8_t inBytePerRow, uint8_t scale, uint16_t* pOutBytesPerRow){
  char s[80];
  uint16_t rowLen = ((w*scale)+8)/8;
  uint16_t bufSize = h*scale*rowLen;
  uint8_t *buf = (uint8_t *)malloc(bufSize);  
  memset( buf, 0x00, bufSize ); 

  *pOutBytesPerRow = rowLen;
  
  snprintf(s, sizeof(s), "Resize[w=%d, bytes/row=%dB, new w=%d, new bytes/row=%dB]", w, inBytePerRow, w*scale, rowLen);
  // Serial.println(s);  

  for( uint16_t y=0; y<h*scale; y++){
    uint16_t rowBegin = y*rowLen;
    uint16_t bitCount = 0;

    snprintf(s, sizeof(s), "[rowBegin=%3d]", rowBegin);
    // Serial.print(s);  
    
    for( uint16_t x=0; x<inBytePerRow*8; x++){
        for( uint8_t n=0; n<scale && bitCount<rowLen*8; n++){
          
          uint16_t inBytePos = ((y/scale)*inBytePerRow)+(x/8);
          // snprintf(s, sizeof(s), "[%3d]", inPos);
          // Serial.print(s);  
          
          if( (pImageBuf[inBytePos] & (0x80>>(x%8)))!=0 ){
            buf[rowBegin+bitCount/8] |= (0x80>>(bitCount%8));
            // Serial.print("#");
          }
          else{
            // Serial.print(" ");
          }
          bitCount++;  
        }
    }
    snprintf(s, sizeof(s), "<bitcount=%d>", bitCount);
    // Serial.println(s);  
  }
  
  return buf;
}
