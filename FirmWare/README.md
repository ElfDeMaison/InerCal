# InerCal software

## Prérequis

- [PlatformIO](https://platformio.org/)
- Python 3

## Installation

# Cloner le dépôt
```bash
git clone https://github.com/votre-utilisateur/InerCal.git
cd InerCal
```

# Créer un environnement virtuel Python
```bash
python3 -m venv env
source env/bin/activate  # Sous Windows : env\Scripts\activate
pip install -r requirements.txt
```  
## Upload websever data

Run this command to upload the webserver data (html, JS, CSS...) to the ESP32:

`pio run --target uploadfs`

## Flash ESP32

`pio run --target upload`

## Buid Code 

`pio run`
