#include "server.h"
#include <SPIFFS.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "icm42688.h"

// Déclarations externes

WebServer server(80);
WebSocketsServer webSocket(81);

static volatile bool acquisitionActive = false;

// --- Fichiers SPIFFS disponibles (debug utile) ---
void listSPIFFSFiles() {
  File root = SPIFFS.open("/");
  if (!root) {
    Serial.println("Erreur de lecture du répertoire");
    return;
  }
  root.rewindDirectory();
  while (true) {
    File entry = root.openNextFile();
    if (!entry) break;
    Serial.println(entry.name());
    entry.close();
  }
}

// --- Routes fichiers statiques (HTML, CSS, JS) ---
void setupStaticRoutes() {
  server.on("/", HTTP_GET, []() {
    File file = SPIFFS.open("/index.html", "r");
    if (!file) return server.send(404, "text/plain", "File not found");
    server.send(200, "text/html", file.readString());
    file.close();
  });

  server.on("/style.css", HTTP_GET, []() {
    File file = SPIFFS.open("/style.css", "r");
    if (!file) return server.send(404, "text/plain", "CSS file not found");
    server.streamFile(file, "text/css");
    file.close();
  });

  server.on("/script.js", HTTP_GET, []() {
    File file = SPIFFS.open("/script.js", "r");
    if (!file) return server.send(404, "text/plain", "JS file not found");
    server.streamFile(file, "application/javascript");
    file.close();
  });
}

// --- Routes de commandes (start/stop/calibrations) ---
void setupCommandRoutes() {
  server.on("/start", HTTP_GET, []() {
    acquisitionActive = true;
    server.send(200, "text/plain", "Acquisition démarrée !");
  });

  server.on("/stop", HTTP_GET, []() {
    acquisitionActive = false;
    server.send(200, "text/plain", "Acquisition arrêtée !");
  });

  server.on("/calibrate", HTTP_GET, []() {
    if (!acquisitionActive) {
      calibrateAcc();
      server.send(200, "text/plain", "Acc calibré !");
    } else {
      server.send(400, "text/plain", "Arrête l'acquisition avant calibration.");
    }
  });

  server.on("/calibrate2", HTTP_GET, []() {
    if (!acquisitionActive) {
      calibrateGyro();
      server.send(200, "text/plain", "Gyro calibré !");
    } else {
      server.send(400, "text/plain", "Arrête l'acquisition avant calibration.");
    }
  });
}

// --- Gestion des WebSockets ---
void setupWebSocketEvents() {
  webSocket.onEvent([](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    String message = String((char*)payload);

    switch (type) {
      case WStype_CONNECTED:
        Serial.printf("[WebSocket] Client %u connecté\n", num);
        webSocket.sendTXT(num, "Bienvenue sur le serveur WebSocket !");
        break;

      case WStype_DISCONNECTED:
        Serial.printf("[WebSocket] Client %u déconnecté\n", num);
        break;

      case WStype_TEXT:
        Serial.printf("[WebSocket] Message reçu de %u: %s\n", num, payload);
        if (message == "start") {
          acquisitionActive = true;
          webSocket.sendTXT(num, "Acquisition démarrée !");
        } else if (message == "stop") {
          acquisitionActive = false;
          webSocket.sendTXT(num, "Acquisition arrêtée !");
        } else {
          webSocket.sendTXT(num, "Commande inconnue !");
        }
        break;

      default:
        break;
    }
  });
}

// --- Setup complet du serveur SPIFFS + WebSocket ---
void setupServer() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Erreur lors du montage SPIFFS");
    return;
  }
  server.begin();
  listSPIFFSFiles();
  setupStaticRoutes();
  setupCommandRoutes();
  setupWebSocketEvents();
  webSocket.begin();
}

// --- Tâche qui gère les clients Web ---
void handleRoot(void*) {
  while (true) {
    server.handleClient();
    webSocket.loop();
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// --- Tâche qui diffuse les données capteurs ---
void broadcastSensorData(void*) {
  int messageCount = 0;
  unsigned long acquisitionStartTime = 0;
  bool wasActive = false;

  const TickType_t period = pdMS_TO_TICKS(25);  // 40 Hz
  TickType_t lastWakeTime;

  while (true) {
    if (acquisitionActive) {
      if (!wasActive) {
        // Démarrage d’une nouvelle session
        acquisitionStartTime = millis();
        messageCount = 0;
        lastWakeTime = xTaskGetTickCount();  // initialisé AU DÉBUT de l’acquisition
        wasActive = true;
      }

      struct SensorData data;
      getSensorData(&data);

      StaticJsonDocument<256> jsonDoc;
      jsonDoc["count"] = messageCount;
      jsonDoc["ax"] = data.ax;
      jsonDoc["az"] = data.az;
      jsonDoc["ay"] = data.ay;
      jsonDoc["gyrox"] = data.gyrox ;
      jsonDoc["gyroy"] = data.gyroy ;
      jsonDoc["gyroz"] = data.gyroz ;
      jsonDoc["theta"] = data.theta;
      jsonDoc["thetaDot"] = data.thetaDot;
      jsonDoc["thetaDotDot"] = data.thetaDotDot;

      String jsonString;
      serializeJson(jsonDoc, jsonString);

      if (jsonString.indexOf("theta") < 0 || jsonString.indexOf("ax") < 0) {
        Serial.println("Erreur de sérialisation WebSocket !");
      } else {
        webSocket.broadcastTXT(jsonString);
        messageCount++;

        if (messageCount % 100 == 0) {
          Serial.printf("%d messages envoyés\n", messageCount);
        }
      }

      vTaskDelayUntil(&lastWakeTime, period);  // synchronisation stricte 40 Hz

    } else {
      if (wasActive) {
        unsigned long duration = millis() - acquisitionStartTime;
        float seconds = duration / 1000.0;
        Serial.printf("Acquisition arrêtée. Durée : %.2f s — Total envoyé : %d messages\n",
                      seconds, messageCount);
        wasActive = false;
      }

      vTaskDelay(pdMS_TO_TICKS(200));  // attend tranquillement si désactivé
    }
  }
}
