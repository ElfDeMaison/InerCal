#include <WiFi.h>
#include <Arduino.h>
#include <SPI.h>
#include "wifi_config.h"
#include "icm42688.h"
#include "server.h"
#include "freertos/FreeRTOS.h"

#define LED 2  // LED intégrée pour debug visuel

float previousAcc = 0;
float currentAcc = 0;

// Tâche de clignotement LED (utilisée pour confirmer que le système tourne)
void blinker(void*) {
  pinMode(LED, OUTPUT);
  while (true) {
    digitalWrite(LED, HIGH);
    vTaskDelay(pdMS_TO_TICKS(500));
    digitalWrite(LED, LOW);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// Fonction d'initialisation principale
void setup() {
  Serial.begin(115200);
  delay(5000);  // Pour laisser le temps d'ouvrir le moniteur série

  Serial.println("Boot ESP32...");

  // Tâche clignotante de debug
  xTaskCreate(blinker, "Blinker", 1024, NULL, 1, NULL);
  Serial.println("Tâche Blinker OK");

  // Création d'un point d'accès Wi-Fi local
  WiFi.softAP(ssid, password);
  Serial.println("Point d'accès Wi-Fi créé !");
  Serial.println("Adresse IP : " + WiFi.softAPIP().toString());

  // Initialisation du capteur ICM42688 via SPI
  Serial.println("Initialisation ICM...");
  setupICM();
  Serial.println("IMU OK");

  // Initialisation du serveur SPIFFS et WebSocket
  Serial.println("Setup Serveur...");
  setupServer();
  Serial.println("Serveur OK");

  // Tâches FreeRTOS : capteur, communication, Web
  xTaskCreatePinnedToCore(sensorValues, "SensorValues", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(broadcastSensorData, "BroadcastData", 8192, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(handleRoot, "HandleRoot", 4096, NULL, 1, NULL, 0);

  Serial.println("Setup terminé !");
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(100));  // Laisse le système tourner sans bloquer
}
