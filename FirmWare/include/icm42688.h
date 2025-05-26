#pragma once

#include <Arduino.h>
#include <ICM42688.h>

// Fonction d'initialisation du capteur
void setupICM();

// Fonction de debug pour afficher les valeurs capteurs
void print_Acc_XYZ();

// Tâche FreeRTOS : lecture des valeurs capteur + estimation
void sensorValues(void*);

// Fonctions de calibration
void calibrateGyro();
void calibrateAcc();
;

struct SensorData {
    float ax, ay, az;
    float gyrox, gyroy, gyroz;
    float theta, thetaDot, thetaDotDot;
};
void getSensorData(SensorData* data);
