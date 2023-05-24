


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


//-----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------  SETUP -----------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------

void setup(){
  Serial.begin(115200);
  Serial.println("COMENZANDO EL SETUP .................................");
  pinMode(ledLuminosidad, OUTPUT);
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
 
  Serial.println("Connected to the WiFi");
  Serial.println();
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("IP: ");  
  Serial.println(WiFi.localIP());
  
   // Start up the library
  sensors_DS19B20.begin();

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
   //comprobarPRESION_ATMOSFERICA();

   //Se obtiene velocidad del viento 
   //comprobarVIENTO();

  //Se obtiene cantidad de lluvia
   //comprobarLLUVIA();

   //Se obtiene presión atmosférica
  comprobarHUMEDAD();
  
  //Se obtiene el nivel de luminosidad
  comprobarLUMINOSIDAD();

  //Se obtiene el nivel de luminosidad
  comprobarRUIDO();
  
  //Se comprueba si hay que leer las temperaturas (de todos los sensores DS18B20)
  comprobarTEMPERATURAS_DS18B20();

  
} /////// FIN loop