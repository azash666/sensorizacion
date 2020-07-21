//Version para Wemos D1 mini
#include "Wire.h"
#include "SoftwareSerial.h"
#include "TFMini.h"
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>
#include <DNSServer.h>
#include <EEPROM.h>

/* Network settings */
const char* ssid = "720tec-iot"; // SSID - your WiFi's name 
const char* password = "ioniPRim65!"; // Password 
//const char* device_name = "dispositivo"; // you can access controller by http://dispositivo.local/
IPAddress ip(192,168,1,1);  // static IP adress of device 
IPAddress gateway(192,168,1,1); // Gatway
IPAddress subnet(255,255,255,0); // Network mask
DNSServer dnsServer;
WiFiServer server(80);
WebSocketsServer webSocket(81);

const bool debug = false;

int LIMITE_TECHO = 600;
int LIMITE_ACELERACION = 144;
const long TIEMPO_ENTRE_LECTURAS = 200;

const int insideLedPin = D5;
const int notInsideLedPin = D6;
const int hitLedPin = D7;
const int notHitLedPin = D8;

SoftwareSerial mySerial(D4, D3); // RX, TX

TFMini tfmini;

int maximo_aceleracion = 0;

float x, y, z;
float max_x, max_y, max_z;
int distancia_techo;

bool insideLed = false;
bool golpeLed;
bool reset_var = false;
unsigned long siguienteComprobacionTiempo;

const int lenHistorial = 20;
int posicionHistorial = 0;
int historial[lenHistorial];
int tiempos[lenHistorial];

void setup() {
    Serial.begin(115200);
    mySerial.begin(115200);
    EEPROM.begin(512);
    delay(100);
    byte aux;
    aux = EEPROM.read(1);
    LIMITE_TECHO = aux*50;
    aux = EEPROM.read(0);
    LIMITE_ACELERACION = aux*aux/4;
    golpeLed = EEPROM.read(3)>0;
    pinMode(insideLedPin, OUTPUT);
    pinMode(notInsideLedPin, OUTPUT);
    pinMode(hitLedPin, OUTPUT);
    pinMode(notHitLedPin, OUTPUT);
    inicializa_web();
    inicializa_mediciones();
}

void loop() {
    unsigned long tiempo = millis();
    maximo_aceleracion =  max(maximo_aceleracion, lee_acelerometro());
    max_x = abs(max_x)>abs(x)?max_x:x;
    max_y = abs(max_y)>abs(y)?max_y:y;
    max_z = abs(max_z)>abs(z)?max_z:z;
    if ( siguienteComprobacionTiempo <= tiempo) {
        siguienteComprobacionTiempo = tiempo + TIEMPO_ENTRE_LECTURAS;
        distancia_techo = 30;//medirDistanciaLiDAR();
        insideLed = distancia_techo < LIMITE_TECHO;
        if (!golpeLed) {
            if (!reset_var){
                if (maximo_aceleracion > LIMITE_ACELERACION){ 
                  golpeLed = true;
                  EEPROM.write(3,1);
                  EEPROM.commit();
                }
            }
            else
                reset_var = false;
        } else {
            if (reset_var) {
                reset_var = false;
                golpeLed = false;
                EEPROM.write(3,0);
                EEPROM.commit();
            }
        }
        if (maximo_aceleracion > LIMITE_ACELERACION){
          //EEPROM.write(2, 1);
          historico(maximo_aceleracion, millis()/100);
        }
        
        webSocket.loop();
        String texto = ProcessRequest();
        webSocket.broadcastTXT(texto);
        max_x = 0; max_y=0; max_z=0;
        maximo_aceleracion = 0;
    }

    if (insideLed) {
        digitalWrite(insideLedPin, HIGH);
        digitalWrite(notInsideLedPin, LOW);
    } else {
        digitalWrite(insideLedPin, LOW);
        digitalWrite(notInsideLedPin, HIGH);
    }
    if (golpeLed) {
        digitalWrite(hitLedPin, HIGH);
        digitalWrite(notHitLedPin, LOW);
    } else {
        digitalWrite(hitLedPin, LOW);
        digitalWrite(notHitLedPin, HIGH);
    }

    //web
    dnsServer.processNextRequest();
    WiFiClient client = server.available();
    client.flush();
    String currentLine = "";
    if(client.available()){
      client.println("HTTP/1.1 200 OK");
      client.println("");                                     //No olvidar esta línea de separación
      buildHTML2(client);
    }
       
}

void historico(int fuerza, int tiempo){
  if(tiempos[posicionHistorial-1]!=tiempo){
    if(posicionHistorial < lenHistorial){
      historial[posicionHistorial] = fuerza;
      tiempos[posicionHistorial] = tiempo;
      posicionHistorial++;
    }else{
      for(int i=1; i<lenHistorial;i++){
        historial[i-1] = historial[i];
        tiempos[i-1] = tiempos[i];
      }
      historial[lenHistorial-1] = fuerza;
      tiempos[lenHistorial-1] = tiempo;
    }
  }
}
