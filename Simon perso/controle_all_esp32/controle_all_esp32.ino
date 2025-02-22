#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Définition des broches
#define SDA_PIN 33
#define SCL_PIN 36
#define RX_PIN  34
#define TX_PIN  35
#define ONE_WIRE_BUS 14   // Broche du DS18B20
#define SIGNAL_1 24  
#define SIGNAL_2 29

// Température cible
const float TEMP_TARGET = 50.0;
const float TEMP_HYSTERESIS = 2.0; // Hystérésis : 2°C

// Initialisation du bus OneWire et du capteur
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Variables de contrôle
bool heating = false;

void setup() {
    Serial.begin(115200);  // Initialise le port série

    // Configuration de l'I2C
    Wire.begin(SDA_PIN, SCL_PIN);
    Serial.println("I2C initialisé");

    // Configurer les GPIO comme entrées ou sorties
    pinMode(TEMP_SENSOR_PIN, INPUT);
    pinMode(SIGNAL_1, INPUT);
    pinMode(SIGNAL_2, INPUT);

    // Configuration de la liaison série alternative (UART)
    Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
    Serial.println("UART initialisé");
}

void loop() {
    // Lire la valeur du capteur de température
    int tempValue = analogRead(TEMP_SENSOR_PIN);
    Serial.print("Temp Sensor Value: ");
    Serial.println(tempValue);

    // Lire les signaux numériques
    int signal1 = digitalRead(SIGNAL_1);
    int signal2 = digitalRead(SIGNAL_2);
    Serial.print("Signal 1: "); Serial.println(signal1);
    Serial.print("Signal 2: "); Serial.println(signal2);

    // Vérifier la communication série avec un périphérique externe
    if (Serial1.available()) {
        char c = Serial1.read();
        Serial.print("Reçu sur UART: ");
        Serial.println(c);
    }
  if (temperature == DEVICE_DISCONNECTED_C) {
    Serial.println("Erreur : Capteur déconnecté !");
    return;
  }
  
  Serial.print("Température actuelle : ");
  Serial.println(temperature);

  // Régulation de la température avec hystérésis
  if (temperature < TEMP_TARGET - TEMP_HYSTERESIS) {
    heating = true; // Activer le chauffage
  } else if (temperature > TEMP_TARGET + TEMP_HYSTERESIS) {
    heating = false; // Désactiver le chauffage
  }

    delay(1000);
}
