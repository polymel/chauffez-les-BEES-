#include <Wire.h>                // Bibliothèque pour la communication I2C
#include <Adafruit_GFX.h>         // Bibliothèque graphique Adafruit
#include <Adafruit_SSD1306.h>     // Bibliothèque pour l'écran SSD1306

// Définir l'adresse I2C de l'écran OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  // Initialisation de la communication série
  Serial.begin(115200);
  
  // Initialisation de l'écran OLED
  if (!display.begin(SSD1306_I2C_ADDRESS, OLED_RESET)) {
    Serial.println(F("Échec de l'initialisation de l'écran OLED"));
    while (true);
  }

  display.clearDisplay(); // Effacer l'écran

  // Afficher un texte sur l'écran
  display.setTextSize(1);      // Taille du texte
  display.setTextColor(SSD1306_WHITE); // Couleur du texte
  display.setCursor(0, 0);     // Position du curseur
  display.println(F("Bonjour, ESP32!"));
  display.display();           // Mettre à jour l'écran avec le texte
}

void loop() {
  // Pas de code nécessaire dans loop pour l'affichage statique
}
