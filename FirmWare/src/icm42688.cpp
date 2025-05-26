// icm42688.cpp
#include "icm42688.h"

ICM42688 imu(SPI, 5);

// Offsets de calibration (accéléromètre et gyroscope)
float offsetX = 0, offsetY = 0, offsetZ = 0;
float offsetGyroX = 0, offsetGyroY = 0, offsetGyroZ = 0;

// Axe gravitationnel mesuré à la calibration
float g0x = 0, g0y = 0, g0z = 0;

// Variables partagées entre tâches (externes)
unsigned long lastTime = 0;
unsigned long currentTime = 0;
float ax, ay, az, gyrox, gyroy, gyroz;

// Estimation de l'angle (theta), sa vitesse et son accélération
float thetaHat = 0;
float thetaDotHat_t = 0;
float thetaDotHat_t_1 = 0;
float thetaDotDotHat_t = 0;

static volatile float latestTheta = 0;
static volatile float latestThetaDot = 0;
static volatile float latestThetaDotDot = 0;

// Initialisation SPI et du capteur ICM42688
void setupICM() {
    SPI.begin(8, 20, 10);  // SCK = GPIO8, MISO = GPIO20, MOSI = GPIO10
    Serial.println("Début initialisation ICM42688...");

    int status = imu.begin();
    if (status < 0) {
        Serial.println("Échec de la connexion au ICM42688 !");
        while (1);
    }
    Serial.println("ICM42688 connecté avec succès !");
}

// Affiche les valeurs brutes du capteur
void print_Acc_XYZ() {
    imu.getAGT();
    Serial.print("accX: "); Serial.print(imu.accX());
    Serial.print("\taccY: "); Serial.print(imu.accY());
    Serial.print("\taccZ: "); Serial.print(imu.accZ());
    Serial.print("\tgyrX: "); Serial.print(imu.gyrX());
    Serial.print("\tgyrY: "); Serial.print(imu.gyrY());
    Serial.print("\tgyrZ: "); Serial.print(imu.gyrZ());
    Serial.print("\ttemp: "); Serial.println(imu.temp());
}

// Tâche FreeRTOS : lecture des capteurs + calcul de l'angle
void sensorValues(void*) {
    while (true) {
            imu.getAGT();

            ax = imu.accX() - offsetX;
            ay = imu.accY() - offsetY;
            az = imu.accZ() - offsetZ;

            gyrox = imu.gyrX() - offsetGyroX;
            gyroy = imu.gyrY() - offsetGyroY;
            gyroz = imu.gyrZ() - offsetGyroZ;

            // Calcul des valeurs angulaires
            currentTime = millis();
            float dt = (currentTime - lastTime) / 1000.0f;

            thetaDotHat_t = gyroz;  // Choix de l'axe z ici (modifiable)
            thetaHat += thetaDotHat_t * dt;

            // Ramener theta entre -180° et +180°
            if (thetaHat > 180.0f)
            thetaHat -= 360.0f;
            else if (thetaHat < -180.0f)
            thetaHat += 360.0f;
            thetaHat = remainderf(thetaHat, 360.0f);  



            thetaDotDotHat_t = (thetaDotHat_t - thetaDotHat_t_1) / dt;
            thetaDotHat_t_1 = thetaDotHat_t;

            latestTheta = thetaHat;
            latestThetaDot = thetaDotHat_t;
            latestThetaDotDot = thetaDotDotHat_t;

            lastTime = currentTime;
            //print_Acc_XYZ();
        vTaskDelay(pdMS_TO_TICKS(5));  // Fréquence : 100 Hz
    }
}

// Calibration du gyroscope sur 1000 échantillons
void calibrateGyro() {
    float sumX = 0, sumY = 0, sumZ = 0;
    for (int i = 0; i < 1000; i++) {
        imu.getAGT();
        sumX += imu.gyrX();
        sumY += imu.gyrY();
        sumZ += imu.gyrZ();
        delay(2);
    }
    offsetGyroX = sumX / 1000.0;
    offsetGyroY = sumY / 1000.0;
    offsetGyroZ = sumZ / 1000.0;

    //  Remise à zéro des états angulaires
    thetaHat = 0;
    thetaDotHat_t = 0;
    thetaDotHat_t_1 = 0;
    thetaDotDotHat_t = 0;

    latestTheta = 0;
    latestThetaDot = 0;
    latestThetaDotDot = 0;

    Serial.println("Gyroscope calibré. État angulaire remis à zéro.");
}

// Calibration de l'accéléromètre avec retrait de la gravité
void calibrateAcc() {
    float sumX = 0, sumY = 0, sumZ = 0;
    for (int i = 0; i < 1000; i++) {
        imu.getAGT();
        sumX += imu.accX();
        sumY += imu.accY();
        sumZ += imu.accZ();
        delay(2);
    }
    offsetX = sumX / 1000.0;
    offsetY = sumY / 1000.0;
    offsetZ = sumZ / 1000.0 - 1.0;  // Suppression gravité (1g)

    g0x = offsetX;
    g0y = offsetY;
    g0z = offsetZ;
}

void getSensorData(SensorData *data)
{
    data->ax = ax;
    data->ay = ay;
    data->az = az;
    data->gyrox = gyrox;
    data->gyroy = gyroy;
    data->gyroz = gyroz;
    data->theta = latestTheta;
    data->thetaDot = latestThetaDot;
    data->thetaDotDot = latestThetaDotDot;
}
