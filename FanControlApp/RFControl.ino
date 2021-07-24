#include <RCSwitch.h>
#include "HwConfig.h"

RCSwitch RFTransceiver = RCSwitch();

void RFControl_Enable_Rx() {
  pinMode(RF_RX_DATA, INPUT);
  RFTransceiver.enableReceive(RF_RX_DATA);
}

void RFControl_Enable_Tx() {
  pinMode(RF_TX_DATA, OUTPUT);
  RFTransceiver.enableTransmit(RF_TX_DATA);
}

void RFControl_SendCode(unsigned long data, unsigned int len, int nPulseLength, int nProtocol) {
  RFTransceiver.setProtocol(nProtocol, nPulseLength);
  RFTransceiver.send(data, len);
  
  String codeJson = CodeToJson(data, len, nPulseLength, 0, nProtocol);
  Serial.println( String("TX->") + codeJson);  
}

void RFControl_WaitForCode() {
  if (RFTransceiver.available()) {
    String codeJson = CodeToJson(RFTransceiver.getReceivedValue(), RFTransceiver.getReceivedBitlength(), RFTransceiver.getReceivedDelay(), RFTransceiver.getReceivedRawdata(),RFTransceiver.getReceivedProtocol());
    Serial.println(String("RX<-") + codeJson);
    HttpServer_TextAll(codeJson);
    RFTransceiver.resetAvailable();
  }
}

String CodeToJson(unsigned long data, unsigned int length, unsigned int delay, unsigned int* raw, unsigned int protocol) {
  char txtBuf[128] = {};
  // snprintf( txtBuf, sizeof(txtBuf), "\"rf\":{ \"code\":\"%d\", \"len\":%2d, \"protocol\":%2d, \"pulselen\":%3d }", data, length, protocol, delay );
  snprintf( txtBuf, sizeof(txtBuf), "\"rf\":{ \"code\":\"0x%08lX\", \"len\":%2d, \"protocol\":%2d, \"pulselen\":%3d }", data, length, protocol, delay );
  return String(txtBuf);  
}

// The following can be removed

const char* bin2tristate(const char* bin);
char * dec2binWzerofill(unsigned long Dec, unsigned int bitLength);

void output(const char *header, unsigned long decimal, unsigned int length, unsigned int delay, unsigned int* raw, unsigned int protocol) {

  const char* b = dec2binWzerofill(decimal, length);
  Serial.print("Decimal: ");
  Serial.print(decimal);
  Serial.print(" (");
  Serial.print( length );
  Serial.print("Bit) Binary: ");
  Serial.print( b );
  Serial.print(" Tri-State: ");
  Serial.print( bin2tristate( b) );
  Serial.print(" PulseLength: ");
  Serial.print(delay);
  Serial.print(" microseconds");
  Serial.print(" Protocol: ");
  Serial.println(protocol);
  
  Serial.print("Raw data: ");
  for (unsigned int i=0; i<= length*2; i++) {
    Serial.print(raw[i]);
    Serial.print(",");
  }
  Serial.println();
  Serial.println();  
}

const char* bin2tristate(const char* bin) {
  static char returnValue[50];
  int pos = 0;
  int pos2 = 0;
  while (bin[pos]!='\0' && bin[pos+1]!='\0') {
    if (bin[pos]=='0' && bin[pos+1]=='0') {
      returnValue[pos2] = '0';
    } else if (bin[pos]=='1' && bin[pos+1]=='1') {
      returnValue[pos2] = '1';
    } else if (bin[pos]=='0' && bin[pos+1]=='1') {
      returnValue[pos2] = 'F';
    } else {
      return "not applicable";
    }
    pos = pos+2;
    pos2++;
  }
  returnValue[pos2] = '\0';
  return returnValue;
}

char * dec2binWzerofill(unsigned long Dec, unsigned int bitLength) {
  static char bin[64]; 
  unsigned int i=0;

  while (Dec > 0) {
    bin[32+i++] = ((Dec & 1) > 0) ? '1' : '0';
    Dec = Dec >> 1;
  }

  for (unsigned int j = 0; j< bitLength; j++) {
    if (j >= bitLength - i) {
      bin[j] = bin[ 31 + i - (j - (bitLength - i)) ];
    } else {
      bin[j] = '0';
    }
  }
  bin[bitLength] = '\0';
  
  return bin;
}
