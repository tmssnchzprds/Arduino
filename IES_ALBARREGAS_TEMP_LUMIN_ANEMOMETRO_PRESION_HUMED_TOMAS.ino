


#include <ArduinoOTA.h>
#include <PubSubClient.h>         // MQTT publicar y subcribir mensajes
#include <OneWire.h>              // Gestionar el bus Wire
#include <DallasTemperature.h>    // Controlar la temperatura del DS18B20
#include <Adafruit_BMP280.h>
#include "DHT.h"
#include "WiFi.h" 
// Controlar la conexión a wifi

//ver pestañas
#include "Variables.h"
#include "Metodos.h"

#include "ESP8266_Utils_OTA.hpp"


//-----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------  SETUP -----------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------

void setup(){
  Serial.begin(115200);
  Serial.println("COMENZANDO EL SETUP .................................");
  pinMode(ledLuminosidad, OUTPUT);
  pinMode(pinSensorViento, INPUT);
  pinMode(pinSensorLluvia, INPUT);

  //inicializa sensor humedad DHT22
  dht.begin();

  
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    if (contSiDesconexion > 10)
    {
      Serial.println("pasado 10 seg");
       ESP.restart();
    }    
    delay(1000);
    contSiDesconexion++;
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi educarexDEV");
  Serial.println();
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  
   // Start up the library
  sensors_DS19B20.begin();

   //inicialización sensor bmp (presión atmosférica, altitud,temp)
   if (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
   // while (1) delay(10);
  }

  //inicialización OTA
	InitOTA();

  almacenarDireccionFisica();   //ALMACENA LA DIRECCION FISICA DE CADA SENSOR DS18B20 EN LA ESTRUTURA ARRAY CREADA PARA ELLO

   //********************  
  client.setServer(SERVER, SERVERPORT);
  client.setCallback(callback);
  //********************tiempoActual = millis();
  
}

//-----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------  LOOP  -----------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------

void loop(){
  
	ArduinoOTA.handle();

 if (!client.connected()) {
    reconnect();
  }
  client.loop();

  tiempoActual = millis();


   //Se obtiene presión atmosférica
  comprobarPRESION_ATMOSFERICA();

   //Se obtiene velocidad del viento 
  comprobarVIENTO();

  //Se obtiene cantidad de lluvia
  comprobarLLUVIA();

   //Se obtiene presión atmosférica
  comprobarHUMEDAD();
  
  //Se obtiene el nivel de luminosidad
  comprobarLUMINOSIDAD();
  
  //Se comprueba si hay que leer las temperaturas (de todos los sensores DS18B20)
  comprobarTEMPERATURAS_DS18B20();

  
} /////// FIN loop
