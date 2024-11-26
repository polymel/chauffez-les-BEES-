#include <Wire.h>          // Bibliothèque pour la communication I2C
#include "SSD1306Wire.h"   // Bibliothèque pour l'écran OLED

#include "pins_arduino.h"

SSD1306Wire display(0x3C, SDA_OLED, SCL_OLED);

// Fonction pour afficher un message sur le terminal série et l'écran OLED
void printToBoth(const char* message) {
  Serial.println(message);    // Imprime sur le port série par défaut
  
  
  display.clear();
  
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0,0,message);     // Affiche le message sur l'écran
  display.display();          // Met à jour l'écran avec le nouveau message
}

/******************
 * VextOn and VextOff are
 * taken right from Heltec.cpp.
 * These functions turn power 
 * to the display on and off.
 *
 * displayReset() is from 
 * HT_Display.cpp in the Heltec
 * code.  It resets the display
 *****************/
void VextON(void) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void)  //Vext default OFF
{
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, HIGH);
}

void displayReset(void) {
  // Send a reset
  pinMode(RST_OLED, OUTPUT);
  digitalWrite(RST_OLED, HIGH);
  delay(1);
  digitalWrite(RST_OLED, LOW);
  delay(10);
  digitalWrite(RST_OLED, HIGH);
  delay(10);
}
void drawTempBar(int temp,int num,int consigne) {
  Serial.print("Capteur num:");
  Serial.print(num);
  Serial.print("; T°:");
  Serial.println(temp);
  // draw the temperature bar
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawProgressBar(27, num*10+17, 100,6, (temp/float(consigne))*100);
  display.drawString(0, num*10+13, String(num) + ":" + String(temp) + "°");
}
void Tempconsigne(int consigne) {
  Serial.print("Consigne:");
  Serial.println(consigne);
  // draw the temperature bar
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(128, 15, "Tcons:"+String(consigne)+"°");
}

void setup() {
  // Initialisation de la communication série
  Serial.begin(115200);
  while (!Serial) {
    // Attendre l'ouverture du terminal série
    delay(100);
  }
  
  VextON();
  displayReset();
  
  // Initialisation de l'écran OLED
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  
  // Afficher un message de démarrage
  printToBoth("Bonjour, ESP32!");
}

void loop() {
  // clear the display
  // display.clear();
  
  // display.display();
  // delay(2000); 
  // Exemple d'affichage d'un message périodique
  printToBoth("Message periodique.");



int consigne=60;
drawTempBar(40,1,60);
drawTempBar(10,2,60);
drawTempBar(80,3,60);
drawTempBar(60,4,60);
Tempconsigne(consigne);
  // Draw a line horizontally
  display.drawHorizontalLine(0, 16, 140);
    // write the buffer to the display
  display.display();
  delay(2000);  // Attendre 2 secondes avant de réafficher
}
