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
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  payload[sizeof(payload)] = '\0';
  var = atoi((char *)payload);
  Serial.println(String(var));
  delay(5000);
  
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
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  timeClient.begin();
  
  while(!timeClient.update()){
  timeClient.forceUpdate();
}

espClient.setX509Time(timeClient.getEpochTime());

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESPthing")) {
      Serial.println("connected");
      client.subscribe("riego/2");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      
      char buf[256];
      espClient.getLastSSLError(buf,256);
      Serial.print("WiFiClientSecure SSL error: ");
      Serial.println(buf);
      
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void certificados(){
    if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  
  Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());
  
  // Load certificate file
  File cert = SPIFFS.open("/cert.der", "r"); //replace cert.crt eith your uploaded file name
  if (!cert) {
    Serial.println("Failed to open cert file");
  }
  else
    Serial.println("Success to open cert file");
  
  delay(1000);
  
  if (espClient.loadCertificate(cert))
    Serial.println("cert loaded");
  else
    Serial.println("cert not loaded");
  
  // Load private key file
  File private_key = SPIFFS.open("/private.der", "r"); //replace private eith your uploaded file name
  if (!private_key) {
    Serial.println("Failed to open private cert file");
  }
  else
    Serial.println("Success to open private cert file");
  
  delay(1000);
  
  if (espClient.loadPrivateKey(private_key))
    Serial.println("private key loaded");
  else
    Serial.println("private key not loaded");
  
  // Load CA file
  File ca = SPIFFS.open("/ca.der", "r"); //replace ca eith your uploaded file name
  if (!ca) {
  Serial.println("Failed to open ca ");
    }
  else
    Serial.println("Success to open ca");
  
  delay(1000);
  
  if(espClient.loadCACert(ca))
    Serial.println("ca loaded");
  else
    Serial.println("ca failed");
  
  Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());
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
  
  Serial.print("Mensaje enviado: ");
  Serial.println(msg);
  client.publish("data", msg);

  }
  
void setup() {
  Serial.begin(115200);
  pinMode(A0,INPUT);  //Bateria
  pinMode(14,INPUT);  //Humedad
  Serial.setDebugOutput(true);

  bateria = ((5 * analogRead(A0)/1023 - 3.3) * 100)/0.9;
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
