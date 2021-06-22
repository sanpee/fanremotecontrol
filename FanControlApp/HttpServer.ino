#include <WiFi.h>
#include <WiFiClient.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

ArRequestHandlerFunction handleRoot([](AsyncWebServerRequest *request){
  AsyncWebServerResponse *response;
  Serial.print("GET /");
  if (SPIFFS.exists("/index.html")) { // If the file exists                      
    response = request->beginResponse(SPIFFS, "/index.html", "text/html");
    Serial.println(" - Reading from /index.html.");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);       
  }
  else {
    response = request->beginResponse(200, "text/html",  "Please setup the board!");
    Serial.println(" - index.html not found. Setup the board.");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  }  
});

ArRequestHandlerFunction handleStyleCss([](AsyncWebServerRequest *request){
  Serial.println("GET /style.css");
  AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/style.css", "text/css");
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);       
});

ArRequestHandlerFunction handleSettingsJson([](AsyncWebServerRequest *request){
  Serial.print("GET /settings");
  Serial.println(" - Reading from /settings.json.");
  AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/settings.json", "application/json");
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);       
});

ArRequestHandlerFunction onGETWifi([](AsyncWebServerRequest *request){
  Serial.print("GET /wifi");
  Serial.println(" - Reading from /wifi.html");
  AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/wifi.html", "text/html");
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);       
});

ArBodyHandlerFunction handlePostSettings([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  AsyncWebServerResponse *response;
  char szText[512] = {};
  
  snprintf(szText, sizeof(szText), "POST /settings [%d~%d]/%d", index, index+len, total);
  Serial.print(szText);
        
  File file = SPIFFS.open("/settings.json", (index==0)?"w":"a");
  if (file) {
    file.write( data, len );
    file.close();
    snprintf(szText, sizeof(szText), "Successfully written %d/%d bytes to settings.json.", len+index, total);
    Serial.println(" - Write OK");
    response = request->beginResponse(200, "text/plain", szText);
  }
  else {
    Serial.println(" - Unable to write");
    response = request->beginResponse(500, "text/plain", "Unable to write to settings.json.");
  }
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);   
});

ArBodyHandlerFunction onPOSTfsfile([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  AsyncWebServerResponse *response;
  char szText[512] = {};
  snprintf(szText, sizeof(szText), "POST /fs/file [%d~%d]/%d", index, index+len, total);
  Serial.println(szText);

  if (request->hasArg("path")) {
    File file = SPIFFS.open("/" + request->arg("path"), (index==0)?"w":"a");
    if (file) {
      file.write( data, len );
      file.close();
      response = request->beginResponse(200, "text/plain", request->arg("path") + " write completed.");
    }
    else {
      response = request->beginResponse(400, "text/plain", request->arg("path") + " unable to open.");
    }
  }
  else {
    response = request->beginResponse(400, "text/plain", "Missing 'path' argument.");
  }

  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);   
});

ArRequestHandlerFunction onDELETEfsfile([](AsyncWebServerRequest *request){
  AsyncWebServerResponse *response;
  char szText[64] = {};
  snprintf(szText, sizeof(szText), "DELETE /fs/file");
  Serial.println(szText);

  if (request->hasArg("path")) {
    if (SPIFFS.exists("/" + request->arg("path"))) {
      SPIFFS.remove("/" + request->arg("path"));
      response = request->beginResponse(200, "text/plain", request->arg("path") + " removed.");   
    }
    else {
      response = request->beginResponse(200, "text/plain", request->arg("path") + " doesn't exist.");
    }
  }
  else {
    response = request->beginResponse(400, "text/plain", "Missing 'path' argument.");
  }
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);   
});

ArRequestHandlerFunction handleGetSettings([](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response;
    char szText[512] = {};
    Serial.println("GET /settings");
    File file;
    if (SPIFFS.exists("/settings.json")) {
      file  = SPIFFS.open("/settings.json", "r");
      snprintf(szText, sizeof(szText), "Reading /settings.json [size: %d bytes]", file.size());
      Serial.println(szText);
      // char *szBuf = (char *)malloc(file.size());
      response = request->beginResponse(file, "/settings.json", "application/json");
    } 
    else {
      response = request->beginResponse(500, "text/plain", "Settings not found.");
    }
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
    if(file){
      file.close();        
    }
  
});

ArRequestHandlerFunction handleRf([](AsyncWebServerRequest *request){
  String code;
  String len = "24";
  String protocol = "11";
  String pulselen = "322";
    
  for (uint8_t i = 0; i < request->args(); i++) {
    if( strcmp(request->argName(i).c_str(),"code")==0 ) {
      code = request->arg(i);  
    }
    else if( strcmp(request->argName(i).c_str(),"len")==0 ) {
      len = request->arg(i);   
    }
    else if( strcmp(request->argName(i).c_str(),"protocol")==0 ) {
      protocol = request->arg(i);             
    }
    else if( strcmp(request->argName(i).c_str(),"pulselen")==0 ) {
      pulselen = request->arg(i);             
    }
  }
  
  if(code.length()>0)
  {
    RFControl_SendCode(atol(code.c_str()), atol(len.c_str()), atoi(pulselen.c_str()), atoi(protocol.c_str()) );
  }

  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "RF Code sent");
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);  
});

ArRequestHandlerFunction onGETfslist([](AsyncWebServerRequest *request){
  Serial.println("GET /fs/list");
  String sFileListing = "<html>\n<head></head>\n<body>\n";

  File root = SPIFFS.open("/");
  if(root){
    if(root.isDirectory()){
      sFileListing += "<ul>\n";
      File file = root.openNextFile();
      while(file){    
        if(!file.isDirectory()){
          Serial.print("Found ");
          Serial.println(file.name());
          sFileListing += String("<li>") + file.name() + "\n";
        }
        file = root.openNextFile();
      }
      sFileListing += "</ul>\n";
    } 
    else {
      Serial.println(" - not a directory");
    }      
        
  }
  else {
    Serial.println(" - failed to open directory");
  }
  sFileListing += "</body>\n</html>";
  
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", sFileListing.c_str());
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);  
  
});

AwsEventHandler WebSocket_onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      // handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
});

void HttpServer_Init(){

  ws.onEvent(WebSocket_onEvent);
  server.addHandler(&ws);

  server.on("/index.html",  HTTP_GET, handleRoot); 
  server.on("/",            HTTP_GET, handleRoot); 
  server.on("/style.css",   HTTP_GET, handleStyleCss);
  server.on("/settings",    HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, handlePostSettings);
  // server.on("/settings",    HTTP_GET, handleGetSettings);
  server.on("/settings",    HTTP_GET, handleSettingsJson );
  server.on("/rf",          HTTP_GET, handleRf);
  // server.on("/wifi",        HTTP_POST, onPOSTWifi);
  server.on("/wifi",        HTTP_GET, onGETWifi);
  
  server.on("/fs/file",     HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, onPOSTfsfile);
  server.on("/fs/file",     HTTP_DELETE, onDELETEfsfile);
  
  server.on("/fs/list",     HTTP_GET, onGETfslist);

  server.onNotFound([](AsyncWebServerRequest *request){
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += request->url();
    message += "\nMethod: ";
    message += (request->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += request->args();
    message += "\n";
    for (uint8_t i = 0; i < request->args(); i++) {
      message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
    }
    request->send(404, "text/plain", message);
  });
    
  server.begin();
  Serial.println("HTTP server started.");  
}

void HttpServer_TextAll(String text){
  ws.textAll(text);
}
  
