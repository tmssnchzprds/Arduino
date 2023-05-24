/*
 * Cosas que habría que cambiar para otra placa del mismo tipo ESP32
 * 1.Cambiar la variable: NOMPLACA = "nuevo_nombre_dado_a_placa", ej:"placa2_Esp32" 
 * 2.Si se va ha cambiar el puerto de lectura que sea distinto del 4 (G4), cambiar la variable:#define ONE_WIRE_BUS X (sustituir X por el nuevo pinout)
 * 3.Si se va ha cambiar el servidor MQTT, modificar las variables:
 *        - SERVER
 *        - SERVERPORT
 *        - USERNAME
 *        - PASSWORD
 * 5. Si se van a registrar, más de 10 sensores DS18B20, hay que ampliar el array de sensores, cambiar la variable: sensorTempDs18b20[10], sustituir 10 por más.
 */
// String nombreTopic = "/" + NOMPLACA + "/" + sensorTempDs18b20[i].nomSendorTemp;
             
//******************************* DEFINICIONES ***************************
String NOMPLACA = "ies/planta0/patio"; 
#define ONE_WIRE_BUS 4                    // DS18B20 on ESP32 corresponds to G4 on Placa Física
//------------------------------------

unsigned long previousTiempo = 0;         // guarda el tiempo 

/*
 * Estructura creada para contener datos de cada sensor.
 */
struct datoSensorDs18b20{
  String          nomSendorTemp;          //nombre. Importante: este nombre se saca de concatenar la palabra "temp" con un número (1,2,3...10, incrementandose según encuentra sensores Ds18b20)
  DeviceAddress   direccionFisica;        //dirección fisica en Hex
  byte            resolucion;             //resolución de 9, 10, 11 o 12 bits se corresponde con la precisión de 0.5°C, 0.25°C, 0.125°C, o 0.0625°C, respectivamente.
  float           ult_temp;               //
  float           min_temp;               //
  float           max_temp;               //
  unsigned long   tiempoDeMuestra;        // guarda el tiempoDeMuestra (en miliseg, 1000 milisegundos = 1sg) para cada sensor
  unsigned long   tiempoEntreLecturas;    // guarda el tiempo (miliseg, 1000 =1sg) a transcurrir para una nueva temperatura
  //char updateTiempoEntreLecturas[50];   // variable para actualizar el tiempo entre lecturas de un sensor desde nodered, por subscribe (reconnect).
};



const char* ssid     = "";   //wifi a la que se conecta la placa
const char* password = "";   //contraseña wifi a la que se conecta la placa

datoSensorDs18b20 sensorTempDs18b20[10];          //Array de 10 sensores.
unsigned long intervalo_Toma_Temp_DS18B20[10];    //Variable que almacena el intervalo de tiempo en el que se producirá la próxima lectura de temperatura


//Variable patron para componer datos y luego guardarlos en array de senser de Ds18b20;
datoSensorDs18b20 sensorTempAuxiliar; 

int numSensoresDs18b20;

/////// en local mosquitto 
char   SERVER[50]   = ""; //ip del servidor mqtt
int    SERVERPORT   = 1883;
String USERNAME = "";   //usuario mqtt
char   PASSWORD[50] = "";  //contraseña mqtt    


char TOPIC_TEMPERATURA_DS18B20_PE[50]; //Temperatura en array de caracteres necesario para publicar en mqtt
char TOPIC_LUMINOSIDAD[50]; //Luminosidad mqtt
char TOPIC_VIENTO[50]; //Valor de Viento para mqtt
char TOPIC_LLUVIA[50]; //Valor de Lluvia para mqtt
char TOPIC_HUMEDAD_TEMP[50]; // unido datos de Temp Humedad
char TOPIC_TEMP_PRESION_ALTITUD[50]; // unido datos de Temp Presion Altitud mqtt


unsigned long tiempoLuminosidad = 0;
unsigned long tiempoViento = 0;
unsigned long tiempoLluvia = 0;

unsigned long tiempoEntreLecturasViento = 0;
unsigned long tiempoEntreLecturasLluvia = 0;

unsigned long tiempoPresionAtm = 0;
unsigned long tiempoHumedad = 0;

unsigned long tiempoDeMuestra = 0;
unsigned long tiempoActual = 0;

//tiempos a los que ocurre una lectura de determinado tipo
unsigned long tiempoComprobarPresionAtmosferica = 300000;
unsigned long tiempoComprobarViento = 5000; //Cada 5 seg (1000=1 segundos)
unsigned long tiempoComprobarLluvia = 60000;   // LLuvia caida en 1 minuto (=60000) (1000=1 segundos)
unsigned long tiempoComprobarHumedad = 240000;   // Si ha pasado 4 minutos(1000=1 segundos)       



char PLACA[50];

OneWire oneWire(ONE_WIRE_BUS);            // Se crea la variable oneWire
DallasTemperature sensors_DS19B20(&oneWire);      // Se crea la variable DS18B20 sobre el bus Wire

//-------------------------------------------------------------------------
//Presión atmosférica,altitud
Adafruit_BMP280 bmp; // I2C

//-------------------------------------------------------------------------
WiFiClient espClient;
PubSubClient client(espClient);
//-------------------------------------------------------------------------

const int LDRPin = 36;            //pin de entrada sensor de luminosidad (resistencia lumínica)
const int pinSensorViento = 35;   //pin de sensor de viento (solo entrada)
const int pinSensorLluvia = 34;   //pin 34 de sensor de lluvia (solo entrada)



const int ledLuminosidad = 26;    //led que se encience si la luminosidad sobrepasa un valor



int contSiDesconexion = 0;        //Si se pierde la desconexión lo intenta a los 10 veces antes de reset de placa

int contViento = 0;
int contLluvia = 0;

byte ultimoEstadoContactoViento = LOW;  //Variable para medir un intervalo de tiempo en concreto (utilizada para viento)
byte ultimoEstadoContactoLluvia = LOW;  //Variable para medir un intervalo de tiempo en concreto (utilizada para lluvia)
/////datos Humedad
const int DHTPIN = 17;   //pin 17 de sensor DHT22 humedad y temp
DHT dht(DHTPIN, DHT22);   //DHT22 es el tipo de sensor




//-------------------------------------------------------------------------
