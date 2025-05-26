import numpy as np
import json
import scipy.signal as signal
import scipy.optimize as optimize
import matplotlib.pyplot as plt
from tkinter import filedialog, Tk

# === Interface fichier ===
Tk().withdraw()
json_file = filedialog.askopenfilename(title="Sélectionner le fichier de mesure", filetypes=[("Fichier JSON", "*.json")])
if not json_file:
    raise FileNotFoundError("Aucun fichier sélectionné.")

# === Charger le contenu complet du fichier JSON
with open(json_file) as f:
    obj = json.load(f)

# === Extraire les paramètres
params = obj["parametres"]
m = float(params["m"])
g = float(params["g"])
D = float(params["D"])
h = float(params["h"])
fs = int(params["fs"])

print(f"Paramètres : m={m}, g={g}, D={D}, h={h}, fs={fs}")

# === Extraire les données
data = obj["donnees"]
if not data:
    raise ValueError("Aucune donnée chargée.")

thetaDot_deg = np.array([d['thetaDot'] for d in data])
theta_deg    = np.array([d['theta']     for d in data])
time         = np.arange(len(theta_deg)) / fs

if len(theta_deg) < 200:
    raise ValueError("Trop peu de points pour une analyse fiable.")

# === Nettoyage initial
nb_trim_start = int(15 * fs)
nb_trim_end = int(2 * fs)

thetaDot_deg = thetaDot_deg[nb_trim_start:-nb_trim_end]
theta_deg    = theta_deg[nb_trim_start:-nb_trim_end]
time         = time[nb_trim_start:-nb_trim_end]

# === Conversion en radians
thetaDot = np.deg2rad(thetaDot_deg)
theta    = np.deg2rad(theta_deg)

# === Filtrage et dérivée
thetaDot_filtered = signal.savgol_filter(thetaDot, window_length=41, polyorder=2)
thetaDotDot = np.gradient(thetaDot_filtered, time)

# === Modèle physique
def erreur(params):
    I, C, K_D = params
    denom = np.sqrt(1 - 0.5 * (D/h)**2 * (1 - np.cos(theta)))
    restoring = (m * g * D**2) / (4 * h) * np.sin(theta) / denom
    model = - (K_D * thetaDot_filtered**2 + C * thetaDot_filtered + restoring) / I
    return np.sum((thetaDotDot - model)**2)

params_init = [0.01, 0.001, 0.001]
bounds = [(1e-4, None), (0, None), (0, None)]
result = optimize.minimize(erreur, params_init, bounds=bounds)

if not result.success:
    print("Optimisation non convergée :", result.message)

I_estime, C_estime, K_D_estime = result.x

print("\n=== Résultats de l'estimation ===")
print(f"Inertie I estimée           : {I_estime:.6f} kg·m²")
print(f"Damping visqueux C         : {C_estime:.6f} kg·m²/s")
print(f"Damping aérodynamique K_D  : {K_D_estime:.6f} kg·m²/rad²")

# === Modèle recalculé
denom = np.sqrt(1 - 0.5 * (D/h)**2 * (1 - np.cos(theta)))
restoring = (m * g * D**2) / (4 * h) * np.sin(theta) / denom
thetaDotDot_model = - (K_D_estime * thetaDot_filtered**2 + C_estime * thetaDot_filtered + restoring) / I_estime

# === Affichage
plt.figure(figsize=(12, 5))
plt.plot(time, thetaDot_filtered, label="ThetaDot filtré")
plt.title("Vitesse angulaire")
plt.xlabel("Temps (s)")
plt.ylabel("Vitesse angulaire (rad/s)")
plt.legend()
plt.grid(True)

plt.figure(figsize=(12, 5))
plt.plot(time, thetaDotDot, label="ThetaDotDot mesurée", color="orange")
plt.plot(time, thetaDotDot_model, label="ThetaDotDot modèle", linestyle="--")
plt.title("Accélération angulaire : mesurée vs modèle")
plt.xlabel("Temps (s)")
plt.ylabel("Accélération angulaire (rad/s²)")
plt.legend()
plt.grid(True)

plt.show()
