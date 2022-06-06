#include "FS.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

//OLED
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE); 

//Wifi
const char* ssid = "Note7";
const char* password = "pruebawemos";
int var;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
long lastMsg = 0;
char msg[150];
char copy[150];
String mensaje;
String formattedDate;
String dayStamp;
String timeStamp;
String mesDia;
int nodo = 2;
int humedad;
int bateria;

const char* AWS_endpoint = "a2jlcux2w0aeps-ats.iot.eu-west-1.amazonaws.com"; //MQTT broker ip

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  payload[sizeof(payload)] = '\0';
  var = atoi((char *)payload);

  delay(3000);
  u8g2.clearBuffer();  
  u8g2.setFontMode(1);
  u8g2.setFont(u8g2_font_cu12_tr);      
  u8g2.setCursor(0,15);
  u8g2.print("Nodo "+String(nodo));
  u8g2.setCursor(0,30);
  u8g2.print("Mensaje recibido: ");
  u8g2.setCursor(0,45);
  u8g2.print("Riego: "+ String(var)+" ml");
  u8g2.sendBuffer();
  delay(3000);
  u8g2.setPowerSave(1);
  
  ESP.deepSleep(3600e6);
}

WiFiClientSecure espClient;
PubSubClient client(AWS_endpoint, 8883, callback, espClient); //set MQTT port number to 8883 as per //standard



void setup_wifi() {

  delay(10);

  espClient.setBufferSizes(512, 512);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  }

  u8g2.sendBuffer();
  timeClient.begin();

  while(!timeClient.update()){
  timeClient.forceUpdate();
}

espClient.setX509Time(timeClient.getEpochTime());

}

void reconnect() {
  
  // Loop until we're reconnected
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect("ESPthing")) {
      client.subscribe("riego/2");
      
      u8g2.clearBuffer();  
      u8g2.setFontMode(1);
      u8g2.setFont(u8g2_font_cu12_tr);      
      u8g2.setCursor(0,15);      
      u8g2.print("Nodo "+String(nodo));
      u8g2.drawStr(0,30,"Conectado a MQTT");
      u8g2.sendBuffer();
    } else {
      
      u8g2.clearBuffer();  
      u8g2.setFontMode(1);
      u8g2.setFont(u8g2_font_cu12_tr);      
      u8g2.setCursor(0,15);      
      u8g2.print("Nodo "+String(nodo));
      u8g2.drawStr(0,30,"Error al conectar rc= "+client.state());
      u8g2.drawStr(0,45,"Reintentando...");
      u8g2.sendBuffer();
            
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  delay(3000);
}

void certificados(){
  bool alerta=false;
  u8g2.clearBuffer();  
  u8g2.setFontMode(1);
  u8g2.setFont(u8g2_font_cu12_tr);      
  u8g2.setCursor(0,15);      
  u8g2.print("Nodo "+String(nodo));
  u8g2.drawStr(0,30,"Abriendo certificados...");
  u8g2.sendBuffer();
  
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  
  Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());
  
  // Load certificate file
  File cert = SPIFFS.open("/cert.der", "r"); //replace cert.crt eith your uploaded file name
  if (!cert) {
    alerta=true;
  }
  
  delay(1000);
  
  if (espClient.loadCertificate(cert))
    Serial.println("cert loaded");
  else
    alerta=true;
  
  // Load private key file
  File private_key = SPIFFS.open("/private.der", "r"); //replace private eith your uploaded file name
  if (!private_key) {
    alerta=true;
  }
  
  delay(1000);
  
  if (espClient.loadPrivateKey(private_key))
    Serial.println("private key loaded");
  else
    alerta=true;
  
  // Load CA file
  File ca = SPIFFS.open("/ca.der", "r"); //replace ca eith your uploaded file name
  if (!ca) {
  alerta=true;
    }
  
  delay(1000);
  
  if(espClient.loadCACert(ca))
    Serial.println("ca loaded");
  else
    alerta=true;
  
  if(alerta){
      u8g2.clearBuffer();  
      u8g2.setFontMode(1);
      u8g2.setFont(u8g2_font_cu12_tr);      
      u8g2.setCursor(0,15);      
      u8g2.print("Nodo "+String(nodo));
      u8g2.drawStr(0,30,"Error al abrir");
      u8g2.drawStr(0,45,"certificados");
      u8g2.sendBuffer();
    }
  else{
      u8g2.clearBuffer();  
      u8g2.setFontMode(1);
      u8g2.setFont(u8g2_font_cu12_tr);
      u8g2.setCursor(0,15);      
      u8g2.print("Nodo "+String(nodo));
      u8g2.drawStr(0,30,"Certificados OK");
      u8g2.sendBuffer();
    }
  delay(3000);
  }
  
void fecha_hora(){
  //Para ajustar Diferencia horaria GMT/UTC: GMT +1 = 3600 
  //Obtenemos la fecha actual
  timeClient.update();
  formattedDate = timeClient.getFormattedDate();
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  mesDia = dayStamp.substring(5,dayStamp.length());
  if(mesDia>"03-27" & mesDia<"10-30"){
    timeClient.setTimeOffset(7200);
    }
  else{
    timeClient.setTimeOffset(3600);
    }
  formattedDate = timeClient.getFormattedDate();
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  
if (not(timeStamp>"08:00:00" & timeStamp<"09:00:00")){
    //ESP.deepSleep(3600e6);
    }
  }
  
void publicar_datos(){
  mensaje ="{\"ID\":\""+String(nodo)+"\",\"Humedad\":"+humedad+",\"Fecha\":\""+ String(dayStamp)+"\",\"Bateria\":\""+bateria+"\"}";
  //Se envia en caracteres por lo que debemos convertir el string en caracteres. 
  mensaje.toCharArray(copy, 150);
  
  //Montamos el mensaje para enviar a AWS Cloud. 
  snprintf (msg,150,copy);
  

  client.publish("data", msg); //data es nuestro tema para filtrar en AWS IoT

  u8g2.clearBuffer();  
  u8g2.setFontMode(1);
  u8g2.setFont(u8g2_font_cu12_tr);
  u8g2.setCursor(0,15);      
  u8g2.print("Nodo "+String(nodo));
  u8g2.setCursor(0,30);
  u8g2.print("Mensaje publicado:");
  u8g2.setCursor(0,45);
  u8g2.print("Humedad: "+String(humedad));
  u8g2.setCursor(0,60);
  u8g2.print("Bateria: "+String(bateria));  
  u8g2.sendBuffer();
  }
  
void setup() {
  Serial.begin(9600);
  u8g2.begin();
  pinMode(A0,INPUT);
  pinMode(14,INPUT);

  bateria = ((analogRead(A0)*4.2/1023 - 3) * 100)/1.2;
  humedad= not(digitalRead(14));
  setup_wifi();
  
  delay(10000);

  fecha_hora();
  
  certificados();

  if (!client.connected()) {
    reconnect();
  }
  publicar_datos();
  }

void loop() {  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(1000);
}
