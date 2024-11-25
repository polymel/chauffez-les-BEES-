#include <Wire.h>
#include <Adafruit_INA219.h>
 
// Création de l'objet INA219
Adafruit_INA219 ina219;
 
void setup() {
  Serial.begin(115200); // Initialisation de la communication série
  Wire.begin(21, 22);   // Configuration de l'I2C (SDA sur GPIO21, SCL sur GPIO22)
 
  // Initialisation de l'INA219
  if (!ina219.begin()) {
    Serial.println("echec de la communication avec l'INA219");
    while (1); // Boucle infinie en cas d'erreur
  }
  Serial.println("INA219 initialise avec succes !");
}
 
void loop() {
  // Lecture de la tension sur le bus en volts
  float tensionBus = ina219.getBusVoltage_V();
  // Lecture du courant consommé en milliampères
  float courant = ina219.getCurrent_mA();
 
  // Affichage des résultats dans le moniteur série
  Serial.print("Tension du bus : ");
  Serial.print(tensionBus);
  Serial.println(" V");
 
  Serial.print("Courant : ");
  Serial.print(courant);
  Serial.println(" mA");
 
  Serial.println("----------------------------");
 
  delay(1000); // Pause de 1 seconde avant la prochaine mesure
}
 