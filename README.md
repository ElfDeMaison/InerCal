# InerCal – Système de calcul du moment d'inertie embarqué

**InerCal** est un système embarqué destiné à mesurer le **moment d'inertie d'un objet** à l'aide d'un **balancier à deux fils**. Il s’appuie sur une carte **ESP32** et embarque une interface web accessible en Wi-Fi. Le système permet de récupérer les données de mesure directement via un navigateur web sans besoin de logiciel externe.

---

## Objectif

Le but du projet est de fournir un outil **simple, autonome et portable** pour estimer le moment d'inertie d'objets à partir des oscillations d’un système pendulaire.

Le système :

- enregistre les oscillations via un capteur (type ICM-42688),
- envoie les données sur un serveur web embarqué dans l’ESP32,
- affiche les mesures sur une **interface web responsive**.

---

## Matériel requis

- 1x **ESP32** (WROOM ou équivalent)
- 1x **Capteur inertiel** (ex. ICM-42688 via SPI ou I2C)

---

