#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//pour INA219 bibliotheque Adafruit

#include <Adafruit_INA219.h>
Adafruit_INA219 ina219;

// Définition des broches
#define SDA_PIN 33
#define SCL_PIN 36
#define RX_PIN  34
#define TX_PIN  35
#define ONE_WIRE_BUS 14   // Broche du DS18B20 (DATA_temp)
#define TEMP_SENSOR_PIN 7//ONE_WIRE_BUS après
#define TEMP_ID1 2816408EFA001D0
#define TEMP_ID2 2816408EBE3980D
#define TEMP_ID3 2816408E
#define TEMP_ID4 2816408E
#define TEMP_ID5 2816408E

// #define SIGNAL_1 24  
// #define SIGNAL_2 29

// Température cible
const float TEMP_TARGET = 50.0;
const float TEMP_HYSTERESIS = 2.0; // Hystérésis : 2°C

// Initialisation du bus OneWire et du capteur
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);

int numberOfDevices=5; // Number of temperature devices found
float temperaturemin=0,temperaturemax=60;
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address
// Variables de contrôle
bool heating = false;




void setup() {
  Serial.begin(115200);  // Initialise le port série
    while (!Serial) {
      delay(1);
  }

  //temp config
  // Start up the library for temp sensor
  sensors.begin();
  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();
   // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" devices.");
 // Loop through each device, print out address
  for(int i=0;i<numberOfDevices; i++) {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)) {
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
      printAddress(tempDeviceAddress);
      Serial.println();
		} else {
		  Serial.print("Found ghost device at ");
		  Serial.print(i, DEC);
		  Serial.print(" but could not detect address. Check power and cabling");
		}
  }
  //end temp config

    // Configuration de l'I2C
    Wire.begin(SDA_PIN, SCL_PIN);
    Serial.println("I2C initialisé");


    // Configuration de la liaison série alternative (UART)
    Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
    Serial.println("UART initialisé");


  //INA219 
  uint32_t currentFrequency;
  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }else{
    Serial.println("INA219 initialisé");
  }
  

    // // Configurer les GPIO comme entrées ou sorties
    // pinMode(SIGNAL_1, INPUT);
    // pinMode(SIGNAL_2, INPUT);

}

void loop() {
  // Lire la valeur du capteur de température
  sensors.requestTemperatures(); // Send the command to get temperatures
  
  // Loop through each device, print out temperature data
  for(int i=0;i<numberOfDevices; i++) {
    // Search the wire for address
      if(sensors.getAddress(tempDeviceAddress, i)){
      
      // Output the device ID
      Serial.print("Capteur: ");
      Serial.println(i,DEC);
      // Print the data
      float tempC = sensors.getTempC(tempDeviceAddress);
      if (tempC == DEVICE_DISCONNECTED_C) {
      Serial.println("E: Déconnecté !");
      return;
      }
      Serial.print("M: ");
      Serial.println(tempC);
      Serial.print("°C");
      if(temperaturemin>= tempC){
        temperaturemin=tempC;
      }
      if(temperaturemax<= tempC){
        temperaturemax=tempC;
      }

      // Régulation de la température avec hystérésis
      if (temperaturemax > TEMP_TARGET + TEMP_HYSTERESIS) {
        heating = false; // Désactiver le chauffage
      }else if (temperaturemin < TEMP_TARGET - TEMP_HYSTERESIS) {
        heating = true; // Activer le chauffage
      }
      
    } 	
  }
    // // Lire les signaux numériques
    // int signal1 = digitalRead(SIGNAL_1);
    // int signal2 = digitalRead(SIGNAL_2);
    // Serial.print("Signal 1: "); Serial.println(signal1);
    // Serial.print("Signal 2: "); Serial.println(signal2);

    // // Vérifier la communication série avec un périphérique externe
    // if (Serial1.available()) {
    //     char c = Serial1.read();
    //     Serial.print("Reçu sur UART: ");
    //     Serial.println(c);
    // }
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
  Serial.println("");

  delay(2000);
}
// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}
