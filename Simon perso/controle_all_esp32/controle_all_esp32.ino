// // Si TIMER_BASE_CLK n'est pas defini, on le definit sur 80MHz (valeur utilisee sur ESP32 Arduino 3.2.0)
// #ifndef TIMER_BASE_CLK
//   #define TIMER_BASE_CLK 80000000
// #endif
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>
#include <Adafruit_INA219.h>
#include <WiFi.h>
#include <WebServer.h>

const char *ssid = "chauffez-les-BEES-";
IPAddress local_IP(192, 168, 1, 2);
IPAddress gateway(192, 168, 1, 2);
IPAddress subnet(255, 255, 255, 0);
WebServer server(80);

Adafruit_INA219 ina219;
// Definition des broches
#define SDA_PIN 21
#define SCL_PIN 22
#define ONE_WIRE_BUS 14   // Broche du DS18B20 (DATA_temp)
#define TEMP_SENSOR_PIN ONE_WIRE_BUS
#define TEMP_ID1 28392CD90F000071
#define TEMP_ID2 2820D4D90F000024
#define TEMP_ID3 2811D4D90F0000FE
#define TEMP_ID4 288CD4D90F000005
#define TEMP_ID5 2816408E
#define SIGNAL1 2  
#define SIGNAL2 5
// Temperature cible
float TEMP_TARGET = 50.0;
float TEMP_HYSTERESIS = 2.0; // Hysteresis : 2°C
// Initialisation du bus OneWire et du capteur
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
int numberOfDevices=5; // Number of temperature devices found
float temperaturemin=0,temperaturemax=60;
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address
// Variables de controle
bool heating = false;
bool signal1_on = false;
bool signal2_on = false;

// === Mesures energie ===
float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float loadvoltage = 0;
float power_mW = 0;
// === Page HTML avec boutons ===
String pageHTML() {
  // Partie statique du HTML
  String html = R"====(
  <!DOCTYPE html>
  <html>
  <head>
    <title>ESP32 Monitor</title>
    <style>
      body {
        margin: 0;
        font-family: 'Times New Roman', Times, serif;
        background: url('https://images.unsplash.com/photo-1520607162513-77705c0f0d4a') no-repeat center center fixed;
        background-size: cover;
        color: #333;
      }
      .container {
        background-color: rgba(255,255,255,0.9);
        padding: 20px;
        margin: 50px auto;
        width: 90%;
        max-width: 600px;
        border-radius: 15px;
        box-shadow: 0 0 10px rgba(0,0,0,0.3);
      }
      h2 {
        text-align: center;
      }
      button {
        background-color: white;
        color: black;
        border: 2px solid #555;
        padding: 10px 20px;
        margin: 5px;
        font-size: 16px;
        cursor: pointer;
        border-radius: 5px;
      }
      button:hover {
        background-color: #eee;
      }
      .form-group {
        margin-top: 15px;
      }
      input[type=number] {
        padding: 8px;
        width: 100px;
        margin-right: 10px;
      }
    </style>
  </head>
  <body>
    <div class="container">
      <h2>Controle PWM</h2>
      <p>SIGNAL1: )====" + String(signal1_on ? "ON" : "OFF") + R"====(</p>
      <a href="/toggle1"><button>Toggle SIGNAL1</button></a><br>
      <p>SIGNAL2: )====" + String(signal2_on ? "ON" : "OFF") + R"====(</p>
      <a href="/toggle2"><button>Toggle SIGNAL2</button></a><br><br>

      <h2>Regulation</h2>
      <form action="/setParams" method="GET">
        <div class="form-group">
          Temperature cible: <input type="number" name="target" value=")====" + String(TEMP_TARGET) + R"====(" step="0.1"> °C
        </div>
        <div class="form-group">
          Hysteresis: <input type="number" name="hyst" value=")====" + String(TEMP_HYSTERESIS) + R"====(" step="0.1"> °C
        </div>
        <button type="submit">Appliquer</button>
      </form>

      <h2>Mesures</h2>
      <h2>Temperatures capteurs</h2>
      <div id="tempList">Chargement...</div>

      <h2>INA219</h2>
      <div id="ina219">
        <p>VBUS: -- V</p>
        <p>VSHUNT: -- mV</p>
        <p>VLOAD: -- V</p>
        <p>COURANT: -- mA</p>
        <p>PUISSANCE: -- W</p>
      </div>

      <h2>Mode de chauffage</h2>
      <form action="/setHeatingMode" method="GET">
        <label for="mode">Mode:</label>
        <select name="mode" id="mode">
          <option value="manual">Manuel</option>
          <option value="auto">Automatique</option>
        </select>
        <br><br>
        <button type="button" onclick="updateHeatingMode()">Appliquer</button>
      </form>

      <script>
        // Fonction pour envoyer les données via AJAX
        function updateHeatingMode() {
          const mode = document.getElementById("mode").value;
          const url = `/setMode?mode=${mode}`;

          fetch(url)
            .then(response => {
              if (response.ok) {
                alert("Mode de chauffage mis à jour !");
              } else {
                alert("Erreur lors de la mise à jour du mode.");
              }
            })
            .catch(error => console.error('Erreur:', error));
        }
      <script>
        function fetchData() {
          fetch('/data')
            .then(response => response.json())
            .then(data => {
              // Temperatures
              let temps = "<ul>";
              data.temperature.forEach(t => {
                temps += "<li>Capteur " + t.id + ": " + t.value.toFixed(2) + " °C</li>";
              });
              temps += "</ul>";
              document.getElementById("tempList").innerHTML = temps;

              // INA219
              const ina = data.ina219;
              document.getElementById("ina219").innerHTML = `
                <p>VBUS: ${ina.busvoltage.toFixed(2)} V</p>
                <p>VSHUNT: ${ina.shuntvoltage.toFixed(2)} mV</p>
                <p>VLOAD: ${ina.loadvoltage.toFixed(2)} V</p>
                <p>COURANT: ${ina.current_mA.toFixed(2)} mA</p>
                <p>PUISSANCE: ${(ina.power_mW / 1000).toFixed(2)} W</p>
              `;
            })
            .catch(err => console.error("Erreur fetch /data:", err));
        }

        // Appel initial et intervalle de mise à jour
        fetchData();
        setInterval(fetchData, 2000);  // met à jour toutes les 2 secondes
      </script>

    </div>
  </body>
  </html>
  )====";

  return html;
}


// Pour comprendre le type de message que l'on recoit, on met une lettre à chaque debut de ligne
// I: Information
// E: Erreur
// M: Mesure

void setup() {
  setCpuFrequencyMhz(80);
  Serial.begin(115200);  // Initialise le port serie
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
      Serial.print("SENSOR found n° ");
      Serial.print(i, DEC);
      Serial.print(" at ADDRESS: ");
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
      Serial.println("I2C non initialise");
   }else{
      Serial.print("I: ");
      Serial.println("I2C initialise");
   }

  //INA219 
  uint32_t currentFrequency;
  if (! ina219.begin()) {
		  Serial.print("E: ");
      Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }else{
    ina219.setCalibration_32V_2A();
    Serial.print("I: ");
    Serial.println("INA219 initialise et calibre pour utilisation: 32V,2A");
  }
  // === PWM ===
  Serial.println("I: Initialisation PWM 1 et 2...");
  pinMode(SIGNAL1, OUTPUT);
  pinMode(SIGNAL2, OUTPUT);
  analogWrite(SIGNAL1, 0);
  analogWrite(SIGNAL2, 0);
  Serial.println("I: PWM 1 et 2 configure");
  
  // === WiFi Access Point ===
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid);
  Serial.print("I: Connectez-vous au WiFi : ");
  Serial.println(ssid);
  Serial.print("I: Page Web dispo sur : ");
  Serial.println(WiFi.softAPIP());

  // === Routes serveur Web ===
  server.on("/", []() {
    server.send(200, "text/html", pageHTML());
  });
  server.on("/on", []() {
    heating = true;
    server.sendHeader("Location", "/");
    server.send(303);
  });
  server.on("/off", []() {
    heating = false;
    server.sendHeader("Location", "/");
    server.send(303);
  });
  server.on("/setParams", handleSetParams);
  server.on("/data", handleGetData);
  server.on("/setMode", handleSetMode);


  server.begin();
  Serial.println("I: Serveur web lance !");

  Serial.println("I: Fin Initialisation\n");

}

void loop() {
  // Lire la valeur du capteur de temperature
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
        Serial.println("E: Deconnecte !");
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

      // Regulation de la temperature avec hysteresis
      if (temperaturemax > TEMP_TARGET + TEMP_HYSTERESIS) {
        heating = false; // Desactiver le chauffage
      }else if (temperaturemin < TEMP_TARGET - TEMP_HYSTERESIS) {
        heating = true; // Activer le chauffage
      }
      
    } 	
  }

  // === Commande des PWM ===
if (heating) {
  analogWrite(SIGNAL1, 200);  // Signal1 : 50%
  analogWrite(SIGNAL2, 200);  // Signal2 : 25%
  signal1_on = true;
  signal2_on = true;
} else {
  analogWrite(SIGNAL1, 0);
  analogWrite(SIGNAL2, 0);
  signal1_on = false;
  signal2_on = false;
}

  // === Mesures INA219 ===
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

  // === Web server ===
  server.handleClient();
  delay(500);
  Serial.print("I: Next measurment in.. ");
  delay(100);
  Serial.print("3");
  delay(1000);
  Serial.print(" 2");
  delay(1000);
  Serial.print(" 1");
  delay(500);
  Serial.println("");
  Serial.println("");
}
// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}
void handleSetParams() {
  if (server.hasArg("target")) {
    TEMP_TARGET = server.arg("target").toFloat();
  }
  if (server.hasArg("hyst")) {
    TEMP_HYSTERESIS = server.arg("hyst").toFloat();
  }
  server.sendHeader("Location", "/");
  server.send(303);
}
void handleSetMode() {
  if (server.hasArg("mode")) {
    String mode = server.arg("mode");
    if (mode == "auto") {
      heating = true;  // Mode automatique, chauffage activé
    } else if (mode == "manual") {
      heating = false; // Mode manuel, chauffage désactivé par défaut
    }
  }
  server.send(200, "text/plain", "OK");  // Réponse simple
}

void handleGetData() {
  String json = "{";

  json += "\"temperature\":[";

  for (int i = 0; i < numberOfDevices; i++) {
    if (sensors.getAddress(tempDeviceAddress, i)) {
      float tempC = sensors.getTempC(tempDeviceAddress);
      char addr[17];
      sprintf(addr, "%02X%02X%02X%02X%02X%02X%02X%02X",
              tempDeviceAddress[0], tempDeviceAddress[1],
              tempDeviceAddress[2], tempDeviceAddress[3],
              tempDeviceAddress[4], tempDeviceAddress[5],
              tempDeviceAddress[6], tempDeviceAddress[7]);

      json += "{";
      json += "\"id\":\"" + String(addr) + "\",";
      json += "\"value\":" + String(tempC);
      json += "}";

      if (i < numberOfDevices - 1) json += ",";
    }
  }
  json += "],";

  // INA219
  json += "\"ina219\":{";
  json += "\"busvoltage\":" + String(busvoltage) + ",";
  json += "\"shuntvoltage\":" + String(shuntvoltage) + ",";
  json += "\"loadvoltage\":" + String(loadvoltage) + ",";
  json += "\"current_mA\":" + String(current_mA) + ",";
  json += "\"power_mW\":" + String(power_mW);
  json += "}";

  json += "}";
  server.send(200, "application/json", json);
}

