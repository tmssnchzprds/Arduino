//------------------------OBTENER TEMPERATURA DE TODOS LOS SENSORES DS18B20 SEGÚN SU TIEMPO DEFINIDO EN ESTRUCTURA -----------------------------
/*
 * Función que obtiene 
 * los datos de la temperatura DS18B20, recorre el array de sensores y publica cada temperatura con el topic que contienen en la estructura creada.
 * Variables:
 *    El nombre del topic, estará formado por una / + Nombre de la placa + Nombre del sensor (en la estructura array i,contiene la palabra "temp")
 *    Ejemplo: /placa1_Esp32/temp1; 
 */
void comprobarTEMPERATURAS_DS18B20() {
  

    for (int i=0;i<numSensoresDs18b20;i++){
      
      float tempC = -127.00;
      String strtemp = "";  
      
      if(tiempoActual > (sensorTempDs18b20[i].tiempoDeMuestra+sensorTempDs18b20[i].tiempoEntreLecturas)){  //Si ha pasado el numero de segundos asignados a este sensor
        
        sensorTempDs18b20[i].tiempoDeMuestra = millis();   //Actualiza el tiempo 1
                
        sensors_DS19B20.requestTemperaturesByAddress(sensorTempDs18b20[i].direccionFisica);                    
        
        sensors_DS19B20.setResolution(sensorTempDs18b20[i].direccionFisica, sensorTempDs18b20[i].resolucion);   
        
        tempC = sensors_DS19B20.getTempC(sensorTempDs18b20[i].direccionFisica);                                

        if (tempC == -127.00) {
            tempC = 0.01;   //Error pero pongo el valor 0.1;                
            Serial.println("Error al tomar la temperatura del sensor: "+sensorTempDs18b20[i].nomSendorTemp+": [" +  String(TOPIC_TEMPERATURA_DS18B20_PE) + "] " + sensorTempDs18b20[i].nomSendorTemp);
            
        }  
        
        char valueStr[50];  //  Este array de caracteres contendrá la temperatura obtenida del sensor Ds18b20 en un momento de tiempo determinado.

        strtemp = String(tempC, 2);   // **************   Temperatura en texto con 2 decimales *************
        
        strtemp.toCharArray(valueStr, 50);  //Se pasa la temperatura al array de caracteres valueStr, formato necesario para publicarla
        
        
        String nombreTopic = "/" + NOMPLACA + "/" + sensorTempDs18b20[i].nomSendorTemp;
             
        nombreTopic.toCharArray(TOPIC_TEMPERATURA_DS18B20_PE, 50);
      
        Serial.println("Enviando "+sensorTempDs18b20[i].nomSendorTemp+": [" +  String(TOPIC_TEMPERATURA_DS18B20_PE) + "] " + strtemp);
      
        /// ******* PUBLICACION MQTT ******** ////
        client.publish(TOPIC_TEMPERATURA_DS18B20_PE, valueStr);  //Publica en mqtt la temperatura del sensor obtenida
      }
                       
    }
    
}

/*
 * Cambiarmos el tiempo entre lecturas del sensor de temperatura pasado como parámetro.
 */
void cambiartiempoEntreLecturas_DS18B20(String nomSensor,unsigned long t_e_l){

  for (int i=0;i<numSensoresDs18b20;i++){
          if (nomSensor == "UpdateIntervalo_"+sensorTempDs18b20[i].nomSendorTemp){
            Serial.println("cambiando el tiempo entre lecturas------------------<<<<<<<");     
            sensorTempDs18b20[i].tiempoEntreLecturas=t_e_l;
          }
            
    }  
}

/*
 * Subcriber a todos los cambio en los periodos de temperaturas.
 */
void subscribe_DS18B20(){
  char NOM_SENSOR_TEMP[50];
  
  for (int i=0;i<numSensoresDs18b20;i++){
     String STRING_NOM_SENSOR_TEMP = "/" + NOMPLACA + "/" + "UpdateIntervalo_"+sensorTempDs18b20[i].nomSendorTemp;
     Serial.print("STRING_NOM_SENSOR_TEMP=");
     Serial.println(STRING_NOM_SENSOR_TEMP);
     STRING_NOM_SENSOR_TEMP.toCharArray(NOM_SENSOR_TEMP, 50);
     client.subscribe(NOM_SENSOR_TEMP);// subscribe to the command topic - will listen here for comands to the 
                      
  }  
}

/*
 * 
 */
String nomSendorTempCallBack(String topic){
  int indice_final = topic.lastIndexOf('/');
  String nomSendorTemp = topic.substring(indice_final+1);
  return nomSendorTemp;
}
  
  

//------------------------CALLBACK-----------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String cargaUtil=(char*)payload;
  Serial.print("Mensaje Recibido: [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.print("Recibido el mensaje:");
  Serial.println(cargaUtil);


  //Si la ultima parte del topic coincide con algun nombre del array de sensores, modifica el valor con la carga util.
  //Aquí se modifica el tiempo entre lecturar de cada sensor.
  cambiartiempoEntreLecturas_DS18B20(nomSendorTempCallBack(topic),atol(cargaUtil.c_str()));
  
}
//------------------------RECONNECT-----------------------------


void reconnect() {
  // Loop hasta que estamos conectados
  while (!client.connected()) {
     if (contSiDesconexion > 10)
      {
        Serial.println("pasado 10 seg");
         ESP.restart();
      }  
      contSiDesconexion++;

    Serial.print("Intentando conexion MQTT...");
    // Crea un ID de cliente al azar
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    USERNAME.toCharArray(PLACA, 50);
    if (client.connect(clientId.c_str(), PLACA, PASSWORD)) {
      Serial.println("conectado");
      //se subcribe a cualquier cambio en el intervalo de actualización de las temperatura
      subscribe_DS18B20();
      
 ////client.subscribe(BTO_ALIMENTAR);// subscribe to the command topic - will listen here for comands to the RFLink 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//------------------------ALMACENA LA DIRECCION FISICA DE CADA SENSOR DS18B20 EN LA ESTRUTURA ARRAY CREADA PARA ELLO -----------------------------
/*
 * Función que recorre todos los sensores DS18B20 conectados y obtiene su dirección física. 
 * Luego se rellenan los datos y se asigna al array de sensores de temperatura.
 * 
 * Importante: El nombre del sensor de temperatura, sale de concatenar 
 * la palabra "temp" con el indice incremental (1,2,3,.. hasta 10, capacidad del array 
 * si se quiere más hay que modificar el array de 10 estructuras) de sensores que se van identificando.
 */
void almacenarDireccionFisica(){

  byte addr[8];
  byte posicion=0;
  DeviceAddress   devAddr;             //dirección fisica en Hex
  unsigned long   tiempoRefresco=0;    //Tiempo entre lectura inicial, al arrancar la placa (setup)

  
  numSensoresDs18b20=sensors_DS19B20.getDeviceCount(), DEC;
  Serial.println("-------------------------------------");
  Serial.print(numSensoresDs18b20);
  Serial.println(" sensores Conectados.");



  while (oneWire.search(addr)) {
   
    
    Serial.print(String(posicion)+" ----> ROM o DIRECCION BUSCADA =");
    
    for(uint8_t i = 0; i < 8; i++) {
      Serial.write(' ');
      Serial.print(addr[i], HEX);
      devAddr[i]=addr[i];
    } 
    Serial.println();

    tiempoRefresco=tiempoRefresco+10000;
    
    
    sensorTempAuxiliar={
      {"temp"+String(posicion+1)},
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // dirección hex
      12,
      0.0,
      50.0,
      0.0,
      0,
      1000  //30000=30 segundos (1000 = 1 sg)
    };
    
    sensors_DS19B20.getAddress(sensorTempAuxiliar.direccionFisica, posicion);
    sensorTempAuxiliar.resolucion= 12;
    sensorTempAuxiliar.tiempoEntreLecturas= tiempoRefresco;
    
    sensorTempDs18b20[posicion] = sensorTempAuxiliar;

    posicion++;
  }

}

//------------------------RUIDO -----------------------------


void comprobarRUIDO() {
  
  String strtemp = "";
  
  if(tiempoActual > (tiempoRuido+3000)){  //Si ha pasado 30 segundos (30000=30 segundos)

        int lightVal;                     // guarda el valor de la luminosidad
        
        //valor maximo del sensor es 4095
        tiempoRuido=tiempoActual;
                
        char valueStr[50];  //  Este array de caracteres contendrá la temperatura obtenida del sensor Ds18b20 en un momento de tiempo determinado.

        lightVal = analogRead(RuidoPin); // Lee el nivel de luz actual
        
        strtemp = String(lightVal);   // **************   Valor Luminosidad en texto *************
        
        strtemp.toCharArray(valueStr, 50);  //Se pasa la temperatura al array de caracteres valueStr, formato necesario para publicarla en mqtt
        
        
        String nombreTopic = "/" + NOMPLACA + "/RUIDO";
             
             nombreTopic.toCharArray(TOPIC_RUIDO, 50);
            
             Serial.println("Ruido: " + strtemp);
            
             /// ******* PUBLICACION MQTT ******** ////
             client.publish(TOPIC_RUIDO, valueStr);  //Publica en mqtt la temperatura del sensor obtenida
      }
  
} 

//------------------------LUMINOSIDAD -----------------------------


void comprobarLUMINOSIDAD() {
  
  String strtemp = "";
  
  if(tiempoActual > (tiempoLuminosidad+3000)){  //Si ha pasado 30 segundos (30000=30 segundos)

        int lightVal;                     // guarda el valor de la luminosidad
        
        //valor maximo del sensor es 4095
        tiempoLuminosidad=tiempoActual;
                
        char valueStr[50];  //  Este array de caracteres contendrá la temperatura obtenida del sensor Ds18b20 en un momento de tiempo determinado.

        lightVal = analogRead(LDRPin); // Lee el nivel de luz actual
        
        strtemp = String(lightVal);   // **************   Valor Luminosidad en texto *************

        strtemp.toCharArray(valueStr, 50);  //Se pasa la temperatura al array de caracteres valueStr, formato necesario para publicarla en mqtt
        
        
        String nombreTopic = "/" + NOMPLACA + "/Luminosidad";
             
             nombreTopic.toCharArray(TOPIC_LUMINOSIDAD, 50);
            
             Serial.println("Luminosidad: " + strtemp);
            
             /// ******* PUBLICACION MQTT ******** ////
             client.publish(TOPIC_LUMINOSIDAD, valueStr);  //Publica en mqtt la temperatura del sensor obtenida
      }
  
} 
 
//------------------------ *************************************************** -------------------------------------- 

// Complete Instructions to Get and Change ESP MAC Address: https://RandomNerdTutorials.com/get-change-esp32-esp8266-mac-address-arduino/

#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
//------------------------PRESION ATMOSFÉRICA, Y ALTITUD -----------------------------
void comprobarPRESION_ATMOSFERICA() {

    
  String strtemp = "";
          
   if(tiempoActual > (tiempoPresionAtm+tiempoComprobarPresionAtmosferica)){  //Si ha pasado 5 minutos (1000=1 segundos)
        
    float Temp= bmp.readTemperature();
    Serial.print(F("Temperature = "));
    Serial.print(Temp);
    Serial.println(" *C");
      
    float Presion= bmp.readPressure();
    Serial.print(F("Pressure = "));
    Serial.print(Presion);
    Serial.println(" Pa");

    float Altitud = bmp.readAltitude(1013.25);
    Serial.print(F("Approx altitude = "));
    Serial.print(Altitud); // Adjusted to local forecast! 
    Serial.println(" m");

    Serial.println();
        
        String Temp_Presion_Altitud;
        Temp_Presion_Altitud = String(Temp) + ":" + String(Presion) + ":" + String(Altitud);
        
        tiempoPresionAtm=tiempoActual;
        
                     
        char valueStr[50];  //  Este array de caracteres contendrá la presión en un momento de tiempo determinado.

        strtemp = Temp_Presion_Altitud;   // **************   Valor presion en texto *************
        strtemp.toCharArray(valueStr, 50);  //Se pasa la temperatura al array de caracteres valueStr, formato necesario para publicarla en mqtt
        
        
        String nombreTopic = "/" + NOMPLACA + "/Temp_Presion_Altitud";
             
        nombreTopic.toCharArray(TOPIC_TEMP_PRESION_ALTITUD, 50);
            
        //Serial.println("----------->Temp, Presión atmosférica, Altitud cada 10 seg: " + String(Temp_Presion_Altitud));
            
        /// ******* PUBLICACION MQTT ******** ////
        client.publish(TOPIC_TEMP_PRESION_ALTITUD, valueStr);  //Publica en mqtt la presion obtenida del sensor bmp
       
   }
    
} 


//------------------------procesar VIENTO -----------------------------

void procesarContadorViento() {
   int estadoContadorViento = digitalRead(pinSensorViento); // Lee el valor en pin35 si se obtiene voltaje
  
  if(tiempoActual > (tiempoEntreLecturasViento+10)){  //Si pasa 10 milisegundos)
    if ((estadoContadorViento==HIGH) && (estadoContadorViento!=ultimoEstadoContactoViento)){
      
          // turn LED on
        digitalWrite(ledLuminosidad, HIGH);
        contViento++;
        //Serial.println("encendido");
        ultimoEstadoContactoViento=estadoContadorViento;
        Serial.println("Contador Pulsos de Viento: " + String(contViento));
             
    } else {  ///APAGADO
      // turn LED off
      if (estadoContadorViento!=ultimoEstadoContactoViento){
        digitalWrite(ledLuminosidad, LOW);
        //Serial.println("apagado");
        ultimoEstadoContactoViento=estadoContadorViento;
        }
    }    
    
    tiempoEntreLecturasViento=tiempoActual;
  }

   
}


//------------------------ comprobar VIENTO -----------------------------
void comprobarVIENTO() {
  procesarContadorViento();

  
  String strtemp = "";
  
  
       
   if(tiempoActual > (tiempoViento+tiempoComprobarViento)){  //Si ha pasado 5 de segundos (1000=1 segundos)
        
        float VientoVal;                     // guarda el valor leido del viento
        
        
        tiempoViento=tiempoActual;
                            
        char valueStr[50];  //  Este array de caracteres contendrá la temperatura obtenida del sensor Ds18b20 en un momento de tiempo determinado.

        VientoVal=(contViento*0.22776)/ (tiempoComprobarViento/1000);      //  1 contacto = 0.22776 m cada media vuelta        
        VientoVal=VientoVal*3.6; //3600/1000 pasar metros por segundos a kilometros por hora
        strtemp = String(VientoVal);   // **************   Valor Viento en texto *************
        strtemp.toCharArray(valueStr, 50);  //Se pasa el valor del viento al array de caracteres valueStr, formato necesario para publicarla en mqtt
        
        
        String nombreTopic = "/" + NOMPLACA + "/Viento";
             
        nombreTopic.toCharArray(TOPIC_VIENTO, 50);
            
        //Serial.println("Velocidad simulada viento cada 5 seg: " + strtemp);
        Serial.println("  ");

        Serial.println("**********->Velocidad simulada viento cada 5 seg: " + strtemp + " km /h");

        contViento=0;
            
        /// ******* PUBLICACION MQTT ******** ////
        client.publish(TOPIC_VIENTO, valueStr);  //Publica en mqtt la cantidad de viento del sensor obtenida
       
   }
    
} 


//------------------------procesar LLUVIA -----------------------------

void procesarContadorLluvia() {
   int estadoContadorLluvia = digitalRead(pinSensorLluvia); // Lee el valor en pin34 si se obtiene voltaje
  if(tiempoActual > (tiempoEntreLecturasLluvia+10)){  //Si pasa 10 milisegundos)
    if ((estadoContadorLluvia==HIGH) && (estadoContadorLluvia!=ultimoEstadoContactoLluvia)){

          // turn LED on
        digitalWrite(ledLuminosidad, HIGH);
        contLluvia++;
        //Serial.println("encendido");
        ultimoEstadoContactoLluvia=estadoContadorLluvia;
        Serial.println("Contador Pulsos de Lluvia: " + String(contLluvia));
             
    } else {  ///APAGADO
      // turn LED off
      if (estadoContadorLluvia!=ultimoEstadoContactoLluvia){
        digitalWrite(ledLuminosidad, LOW);
        //Serial.println("apagado");
        ultimoEstadoContactoLluvia=estadoContadorLluvia;
        }
    }    
    
    tiempoEntreLecturasLluvia=tiempoActual;
  }

   
}


//------------------------ comprobar LLUVIA -----------------------------
void comprobarLLUVIA() {
  procesarContadorLluvia();

  
  String strtemp = "";
  

   
        
   if(tiempoActual > (tiempoLluvia+tiempoComprobarLluvia)){  //Si ha pasado 10 de segundos (1000=1 segundos)
        
        float LluviaVal;                     // guarda el valor leido del lluvia
        //float contLitrosxMetroxContacto = 0.2909091;   //valor calculado por Isidoro y Antonio
        float contLitrosxMetroxContacto = 0.2794;     //valor según manual   
        tiempoLluvia=tiempoActual;
                            
        char valueStr[50];  //  Este array de caracteres contendrá la lluvia en un momento de tiempo determinado.

        LluviaVal=contLluvia*contLitrosxMetroxContacto;      //  1 contacto = 0.2909091 l/m2 (ya que en cada 1 contacto han caido 1,65 mm de agua)   
        strtemp = String(LluviaVal);   // **************   Valor Lluvia en texto *************
        strtemp.toCharArray(valueStr, 50);  //Se pasa la cantidad de agua al array de caracteres valueStr, formato necesario para publicarla en mqtt
        
        
        String nombreTopic = "/" + NOMPLACA + "/Lluvia";
             
        nombreTopic.toCharArray(TOPIC_LLUVIA, 50);
            
        //Serial.println("Velocidad simulada viento cada 10 seg: " + strtemp);
        //Serial.println("  ");
        //Serial.println("----------->Cantidad de lluvia cada 10 seg: " + strtemp + " litros x metroCuadrados");
             
        contLluvia=0;
            
        /// ******* PUBLICACION MQTT ******** ////
        client.publish(TOPIC_LLUVIA, valueStr);  //Publica en mqtt la cantidad de agua de lluvia del sensor obtenida
       
   }
    
} 



//------------------------HUMEDAD -----------------------------
void comprobarHUMEDAD() {

     
  String strtemp = "";
   if(tiempoActual > (tiempoHumedad+tiempoComprobarHumedad)){  //Si ha pasado 4 minutos(1000=1 segundos)
        
    // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float Humedad_DHT11 = dht.readHumidity();
  // Read temperature as Celsius (the default)
//  float Temp_DHT11_C = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
//  float Temp_DHT11_F = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(Humedad_DHT11) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  

    Serial.println();
        
        String Humedad_Temp;
        Humedad_Temp = String(Humedad_DHT11);
        
        tiempoHumedad=tiempoActual;
        
                     
        char valueStr[50];  //  Este array de caracteres humedad en un momento de tiempo determinado.

        strtemp = Humedad_Temp;   // **************   Valor presion en texto *************
        strtemp.toCharArray(valueStr, 50);  //Se pasa la temperatura al array de caracteres valueStr, formato necesario para publicarla en mqtt
        
        
        String nombreTopic = "/" + NOMPLACA + "/Humedad_Temp_DHT11";
             
        nombreTopic.toCharArray(TOPIC_HUMEDAD_TEMP, 50);
            
        //Serial.println("----------->Temp, Humedad cada 4 min: " + String(Humedad_Temp));
            
        /// ******* PUBLICACION MQTT ******** ////
        client.publish(TOPIC_HUMEDAD_TEMP, valueStr);  //Publica en mqtt la humedad y temperatura en ºC y ºF obtenida del sensor dht
       
   }
    
} 
