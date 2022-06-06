#include "FS.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <Arduino.h>

//Wifi
const char* ssid = "QTALON_FIBRA_OPTICA_1041";
const char* password = "D8564G4371";
int var;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const char* AWS_endpoint = "a2jlcux2w0aeps-ats.iot.eu-west-1.amazonaws.com"; //MQTT broker ip

void callback(char* topic, byte* payload, unsigned int length) {
  payload[sizeof(payload)] = '\0';
  var = atoi((char *)payload);
  delay(100);
  
  ESP.deepSleep(3600e6);
}

WiFiClientSecure espClient;
PubSubClient client(AWS_endpoint, 8883, callback, espClient); //set MQTT port number to 8883 as per //standard

long lastMsg = 0;
char msg[150];
char copy[150];
String mensaje;
String formattedDate;
String dayStamp;
String timeStamp;
String mesDia;
int nodo = 2;
int bateria;
int humedad;

void setup_wifi() {

  delay(10);

  espClient.setBufferSizes(512, 512);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  }
  
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
    } else {
      
      char buf[256];
      espClient.getLastSSLError(buf,256);
      
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void certificados(){
    if (!SPIFFS.begin()) {
    return;
  }
  
  Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());
  
  // Load certificate file
  File cert = SPIFFS.open("/cert.der", "r"); //replace cert.crt eith your uploaded file name
  delay(100);
  
  espClient.loadCertificate(cert);
  
  // Load private key file
  File private_key = SPIFFS.open("/private.der", "r"); //replace private eith your uploaded file name
  delay(100);  
  espClient.loadPrivateKey(private_key);
  
  // Load CA file
  File ca = SPIFFS.open("/ca.der", "r"); //replace ca eith your uploaded file name 
  delay(100);
  
  espClient.loadCACert(ca);

  }
void fecha_hora(){
  //Para ajustar Diferencia horaria GMT/UTC: GMT +1 = 3600 
  //Obtenemos la fecha actual
  timeClient.update();
  formattedDate = timeClient.getFormattedDate();
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  mesDia = dayStamp.substring(5,dayStamp.length());

  //Comprobamos la fecha actual para el cambio de hora
  if(mesDia>"03-27" & mesDia<"10-30"){
    timeClient.setTimeOffset(7200);
    }
  else{
    timeClient.setTimeOffset(3600);
    }
  formattedDate = timeClient.getFormattedDate();
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);

  //Si no se encuentra entre la hora deseada duerme otra hora
  if (not(timeStamp>"08:00:00" & timeStamp<"09:00:00")){
    ESP.deepSleep(3600e6);
    }

  }
  
void publicar_datos(){
  mensaje ="{\"ID\":\""+String(nodo)+"\",\"Humedad\":"+ String(humedad) +",\"Fecha\":\""+ String(dayStamp)+"\",\"Bateria\":\""+ String(bateria) +"\"}";
  //Se envia en caracteres por lo que debemos convertir el string en caracteres. 
  mensaje.toCharArray(copy, 150);
  
  //Montamos el mensaje para enviar a AWS. 
  snprintf (msg,150,copy);
  
  client.publish("data", msg);

  }
  
void setup() {
  pinMode(A0,INPUT);  //Bateria
  pinMode(14,INPUT);  //Humedad

  bateria = ((analogRead(A0)*4.2/1023 - 3) * 100)/1.2;
  humedad= not(digitalRead(14));
  setup_wifi();
  
  delay(1000);
  
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
