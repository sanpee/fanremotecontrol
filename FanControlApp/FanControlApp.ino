#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiManager.h> 
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include "HwConfig.h"

unsigned long previousMillis = 0;
unsigned long previousRefreshMillis = 0;
unsigned long interval = 30000;
unsigned long count = 0;

#define MAX_DISPOFFSET 110

void setup() {

  Serial.begin(115200);
  Serial.println("Initializing");
  
  Display_Init();
  Display_Clear();
  Display_DrawText(0,0,"Initializing..."); 
  if (SPIFFS.begin(true)) {
    Serial.println("SPIFFS ok!"); 
    Display_DrawText(0,10,"SPIFFS................[OK]");     
  } 
  else {
    Serial.println("SPIFFS failed!");
    Display_DrawText(0,10,"SPIFFS............[FAILED]");     
    return;
  }

  pinMode(LED, OUTPUT);
	digitalWrite(LED, HIGH);

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  WiFiManager wifiManager;
  // wifiManager.resetSettings();  
  Display_DrawText(0,20,"AP: FanRemoteControl-Setup");     
  if(wifiManager.autoConnect("FanRemoteControl-Setup")){
    HttpServer_Init();
    RFControl_Enable_Tx();
    RFControl_Enable_Rx();
  
    char szHomeUrl[512] = {};
    snprintf(szHomeUrl, sizeof(szHomeUrl), "http://%s", WiFi.localIP().toString().c_str() );
    Serial.println(szHomeUrl);
    Diplay_QRCode(szHomeUrl, count);
  }
}

void loop(){

  RFControl_WaitForCode();

  unsigned long currentMillis = millis();
  
  // Refresh display every second approximate
  if(currentMillis-previousRefreshMillis > 5000){
    count++;
    digitalWrite(LED, count%2?LOW:HIGH); 
     
    char szHomeUrl[512]= {};
    snprintf(szHomeUrl, sizeof(szHomeUrl), "http://%s", WiFi.localIP().toString().c_str() );
    if( count%MAX_DISPOFFSET < MAX_DISPOFFSET/2 ){
      Diplay_QRCode(szHomeUrl, count%MAX_DISPOFFSET);  
    }
    else {
      Diplay_QRCode(szHomeUrl, MAX_DISPOFFSET - (count%MAX_DISPOFFSET) - 1);  
    }
    previousRefreshMillis = currentMillis;
  }
  
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status()!=WL_CONNECTED)&&((currentMillis - previousMillis)>=interval)) {
    Serial.println(" Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
    count = 0;
  }   
}
