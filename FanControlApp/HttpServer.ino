#include <WiFi.h>
#include <WiFiClient.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

ArRequestHandlerFunction onGETSettings([](AsyncWebServerRequest *request){
  Serial.print("GET /settings");
  Serial.println(" - Reading from /settings.json.");
  AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/settings.json", "application/json");
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);       
});

ArBodyHandlerFunction onPOSTSettings([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
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
  
  if (request->hasArg("path")) {
    snprintf(szText, sizeof(szText), "POST /fs/file [%d~%d]/%d -> %s", index, index+len, total, request->arg("path").c_str());
    Serial.println(szText);
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


ArRequestHandlerFunction onGETRf([](AsyncWebServerRequest *request){
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

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  server.on("/settings",    HTTP_GET,     onGETSettings );
  server.on("/settings",    HTTP_POST,    [](AsyncWebServerRequest * request){}, NULL, onPOSTSettings);
  server.on("/rf",          HTTP_GET,     onGETRf);
  server.on("/fs/file",     HTTP_POST,    [](AsyncWebServerRequest * request){}, NULL, onPOSTfsfile);
  server.on("/fs/file",     HTTP_DELETE,  onDELETEfsfile);
  server.on("/fs/list",     HTTP_GET,     onGETfslist);

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
  
