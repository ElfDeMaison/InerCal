#pragma once

#include <WebServer.h>
#include <WebSocketsServer.h>

// Initialisation des routes + SPIFFS
void setupServer();

// Tâche FreeRTOS : gestion des connexions client
void handleRoot(void*);

// Tâche FreeRTOS : diffusion des données en temps réel
void broadcastSensorData(void*);
