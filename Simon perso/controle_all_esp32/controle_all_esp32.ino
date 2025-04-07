#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>
#define TIMER_BASE_CLK 80000000
#include <ESP32TimerInterrupt.h>
#include <Adafruit_INA219.h>
Adafruit_INA219 ina219;

// Définition des broches
#define SDA_PIN 21
#define SCL_PIN 22
#define ONE_WIRE_BUS 14   // Broche du DS18B20 (DATA_temp)
#define TEMP_SENSOR_PIN ONE_WIRE_BUS
#define TEMP_ID1 2816408EFA001D0
#define TEMP_ID2 2816408EBE3980D
#define TEMP_ID3 2816408E
#define TEMP_ID4 2816408E
#define TEMP_ID5 2816408E

#define SIGNAL1 2  
#define SIGNAL2 5
#define FREQ_PWM1 400000 
#define FREQ_PWM2 FREQ_PWM1
#define DUTY_CYCLE1 0.5  // 50%
#define DUTY_CYCLE2 0.5  // 50%

ESP32Timer ITimer1(0);  // Timer pour SIGNAL1
ESP32Timer ITimer2(1);  // Timer pour SIGNAL2

volatile bool state1 = false;
volatile bool state2 = false;
int val=0;
bool IRAM_ATTR onTimer1(void *arg) {
  digitalWrite(SIGNAL1, !digitalRead(SIGNAL1));
  return true;
}

bool IRAM_ATTR onTimer2(void *arg) {
  digitalWrite(SIGNAL2, !digitalRead(SIGNAL2));
  return true;
}



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
// Pour comprendre le type de message que l'on recoit, on met une lettre à chaque début de ligne
// I: Information
// E: Erreur
// M: Mesure

void setup() {
  Serial.begin(115200);  // Initialise le port série
    while (!Serial) {
      delay(1);
  }
  Serial.print("I: ");
  Serial.println("ESP32 connected to UART");
  //temp config
  // Start up the library for temp sensor
  sensors.begin();
  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();
   // locate devices on the bus
  Serial.print("I: ");
  Serial.print("Locating temp sensors devices... ");
  Serial.print("Found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" devices.");
 // Loop through each device, print out address
  for(int i=0;i<numberOfDevices; i++) {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)) {
		  Serial.print("I: ");
      Serial.print("SENSOR: ");
      Serial.print(i, DEC);
      Serial.print(" ADDRESS: ");
      printAddress(tempDeviceAddress);
      Serial.println();
		} else {
		  Serial.print("E: ");
		  Serial.print("Ghost device at ");
		  Serial.print(i, DEC);
		  Serial.println(" but could not detect address. Check power and cabling");
		}
  }
  //end temp config

    // Configuration de l'I2C
   if(!Wire.begin(SDA_PIN, SCL_PIN)){
		  Serial.print("E: ");
      Serial.println("I2C non initialisé");
   }else{
      Serial.print("I: ");
      Serial.println("I2C initialisé");
   }

  setCpuFrequencyMhz(80);
  //INA219 
  uint32_t currentFrequency;
  if (! ina219.begin()) {
		  Serial.print("E: ");
      Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }else{
		  Serial.print("I: ");
      Serial.println("INA219 initialisé et calibré pour utilisation: 32V,2A");
  }

  Serial.println("I: Initialisation PWM 1 et 2...");
  pinMode( SIGNAL1, OUTPUT);
  pinMode( SIGNAL2, OUTPUT);
  analogWriteResolution(SIGNAL1,16);
  analogWriteResolution(SIGNAL2,16);
  val=65535*50/100;
  analogWrite(SIGNAL2,val); 
  analogWrite(SIGNAL1,val ); 
  // uint32_t halfPeriod1 = (1.0 / FREQ_PWM1) * 1e6 * DUTY_CYCLE1;
  // uint32_t halfPeriod2 = (1.0 / FREQ_PWM2) * 1e6 * DUTY_CYCLE2;
  // ITimer1.attachInterruptInterval(halfPeriod1, onTimer1);
  // ITimer2.attachInterruptInterval(halfPeriod2, onTimer2);
  Serial.println("I: PWM 1 et 2 configuré");

}

void loop() {
  // Lire la valeur du capteur de température
  sensors.requestTemperatures(); // Send the command to get temperatures
  // Loop through each device, print out temperature data
  for(int i=0;i<numberOfDevices; i++) {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)){
		  Serial.print("I: ");
      Serial.print("Capteur: ");
      Serial.println(i,DEC);
      float tempC = sensors.getTempC(tempDeviceAddress);
      if (tempC == DEVICE_DISCONNECTED_C) {
        Serial.println("E: Déconnecté !");
        return;
      }
		  Serial.print("M: ");
      Serial.print(tempC);
      Serial.println("°C");
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
  
  Serial.print("M:");
  Serial.print("VBUS: "); Serial.print(busvoltage); Serial.print(" V |");
  Serial.print("VSHUNT: "); Serial.print(shuntvoltage); Serial.print(" mV |");
  Serial.print("VLOAD: "); Serial.print(loadvoltage); Serial.print(" V |");
  Serial.print("CURRENT: "); Serial.print(current_mA); Serial.print(" mA |");
  Serial.print("POWER: "); Serial.print(power_mW/ 1000); Serial.println(" W");


  
  delay(500);
  Serial.print("I: Next measurment in.. ");
  delay(100);
  Serial.print("3");
  delay(1000);
  Serial.print(" 2");
  delay(1000);
  Serial.print(" 1");
  delay(1000);
  Serial.println("");
  Serial.println("");
  delay(3000);
}
// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}
