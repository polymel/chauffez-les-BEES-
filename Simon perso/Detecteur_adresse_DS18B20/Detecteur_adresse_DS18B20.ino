#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define RX_PIN  34
#define TX_PIN  35
#define TEMP_SENSOR_PIN 7// broche 18 sur heltec LoRa V3
// Initialisation du bus OneWire et du capteur
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress; 
void setup() {
   Serial.begin(115200);  // Initialise le port série
  //temp config
  // Start up the library for temp sensor
  sensors.begin();
  if (sensors.getAddress(tempDeviceAddress,0)) {
      Serial.print("Found device ");
      Serial.print(" with address: ");
      printAddress(tempDeviceAddress);
      Serial.println();
  } else{
    Serial.print("Found ghost device");
    Serial.print(" but could not detect address. Check power and cabling");
  }
  
    // Configuration de la liaison série alternative (UART)
    Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
    Serial.println("UART initialisé");
}

void loop() {
  // put your main code here, to run repeatedly:

}

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}
