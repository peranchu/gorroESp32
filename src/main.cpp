#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>

StaticJsonDocument<200> doc;

bool SD_present;
File archivo;
File directorios;

const char *ssid = "Bocarral";
const char *pass = "TexucaLana72";

IPAddress local_IP(192, 168, 1, 200);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

AsyncWebServer server (80);

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
  if(!index){
    if(!filename.startsWith("/")) filename = "/" + filename;
    Serial.println("UploadStart: " + filename);
  
    //request->_tempFile = SD.open(filename, FILE_WRITE);
    archivo = SD.open(filename, FILE_WRITE);
  }
  if(len){
    //Serial.println("escribiendo...");
    //request->_tempFile.write(data, len);
    archivo.write(data, len);
  }

  if(final){
    Serial.println("UploadEnd: " + filename + "," + index + "," + len);
    //request->_tempFile.close();
    directorios = SPIFFS.open("/nombres.json", "a");
    if(!directorios){
      Serial.println("no se pudo abrir el archivo");
      return;
    }
      doc["nombre"] = filename;
      doc["size"] = len;
      
    if(serializeJsonPretty(doc, directorios)){
      Serial.println("Escrito");
      serializeJsonPretty(doc, Serial);
    }else{
      Serial.println("json no escrito");
    }
    
    archivo.close();
    directorios.close();

    request->send(200, "text/plain", "Archivo Subido");
  }
}

void setup() {
  Serial.begin(115200);
  //SPI.begin(18,19,23);

  pinMode(19, INPUT_PULLUP);

  if(!SPIFFS.begin(true)){
    Serial.println("Error SPIFFS");
    return;
  }
  
  Serial.print(F("iniciando Sd card..."));
  if(!SD.begin(5)){
    Serial.println(F("Error SD"));
    SD_present = false;
  }
  else{
    Serial.println(F("Sd iniciada..."));
    SD_present = true;
  }

  if(!WiFi.config(local_IP, gateway, subnet)){
    Serial.println("Fallo configuracion.");
  }

  WiFi.begin(ssid, pass);

  while (WiFi.status() !=WL_CONNECTED){
    delay(500);
    Serial.println("Conectando..");
  }

  Serial.println(WiFi.localIP());

////////////////////////////////////////////////////7
//Servir archivos desde el servidor
  server.on("/uikit.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/uikit.min.css", "text/css");
  });

  server.on("/uikit-icons.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/uikit-icons.min.js", "text/javascript");
  });

  server.on("/uikit.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/uikit.min.js", "text/javascript");
  });

////////////////////////////////////////
server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon.png", "image/png");
  });

server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script.js", "text/javascript");
  });

  server.on("/upload.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/upload.js", "text/javascript");
  });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/nombres.json", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/nombres.json", "text/json");
  });

  server.on("/subida", HTTP_POST, [](AsyncWebServerRequest *request){ 
    request->send(200);}, handleUpload);

  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
}