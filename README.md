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

## 🧪 Guide d'utilisation sur le terrain

Voici les étapes à suivre pour utiliser le système **InerCal** avec un objet réel :

### 🔩 Installation physique

1. **Fixer l’ESP32** (et son capteur inertiel) solidement sur l’objet dont vous souhaitez mesurer le moment d'inertie.

2. **Allumer l’ESP32** (via USB ou batterie).

---

### 📶 Connexion et interface

3. **Se connecter au Wi-Fi** de l’ESP32 (point d’accès créé automatiquement).

4. Ouvrir un navigateur et accéder à l’interface via l’adresse IP indiquée dans le moniteur série (ou adresse par défaut, souvent `192.168.4.1`).

---

### ⚙️ Mesure du moment d'inertie

5. **Calibrer le gyroscope** en cliquant sur le bouton **"Calibrer le gyroscope"**.

6. Cliquer sur **"Démarrer"** pour lancer la capture des données.

7. **Faire osciller l’objet** autour de son axe de rotation (comme un pendule).

8. Une fois les oscillations terminées, cliquer sur **"Arrêter"**.

---

### 📥 Analyse des données

9. **Remplir les champs physiques** dans l’interface (masse, distance, etc.).

10. Cliquer sur **"Télécharger les données"** → un fichier `.json` est enregistré.

---

### 🧠 Post-traitement avec Python

11. Lancer le script Python d’analyse :

```bash
python analyse.py


