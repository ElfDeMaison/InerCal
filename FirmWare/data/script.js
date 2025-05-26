let isRunning = false;
let ws = null;
let isCalibrating = false; // Indicateur pour savoir si la calibration est en cours

let axData = [];
let ayData = [];
let azData = [];
let gyroxData = [];
let gyroyData = [];
let gyrozData = [];
let thetaData = [];
let thetaDotData = [];
let thetaDotDotData = [];
let timeLabels = [];
let dataBuffer = [];

const canvas = document.getElementById("accGraph");
const ctx = canvas.getContext("2d");

let startTime = 0;
let chronoOffset = 0;

function startData() {
  if (!isRunning) {
    isRunning = true;
    startTime = Date.now(); // <-- démarre le chrono
    dataBuffer = [];

    ws = new WebSocket("ws://" + window.location.hostname + ":81/");

    document.getElementById("calibrateButton").disabled = true;

    ws.onopen = () => {
      console.log("Connexion WebSocket ouverte");
      ws.send("start");
    };

    ws.onmessage = (event) => {
      const data = JSON.parse(event.data);
      dataBuffer.push(data);  // 🔹 Ajoute à la liste
      

      document.getElementById("accX").textContent = data.ax.toFixed(2);
      document.getElementById("accY").textContent = data.ay.toFixed(2);
      document.getElementById("accZ").textContent = data.az.toFixed(2);
      document.getElementById("GyroX").textContent = data.gyrox.toFixed(2);
      document.getElementById("GyroY").textContent = data.gyroy.toFixed(2);
      document.getElementById("GyroZ").textContent = data.gyroz.toFixed(2);
      document.getElementById("Theta").textContent = data.theta.toFixed(0);
      document.getElementById("ThetaDot").textContent = data.thetaDot.toFixed(2);
      document.getElementById("ThetaDotDot").textContent = data.thetaDotDot.toFixed(2);

      // Temps écoulé = chronoOffset (temps cumulé avant) + temps depuis le dernier start
      const elapsedTime = chronoOffset + (Date.now() - startTime) / 1000;
      document.getElementById("chrono").textContent = elapsedTime.toFixed(1) + " s";

      // Ajout des données
      timeLabels.push(elapsedTime);
      axData.push(data.ax);
      ayData.push(data.ay);
      azData.push(data.az);
      gyroxData.push(data.gyrox);
      gyroyData.push(data.gyroy);
      gyrozData.push(data.gyroz);
      thetaData.push(data.theta);
      thetaDotData.push(data.thetaDot);
      thetaDotDotData.push(data.thetaDotDot);

      if (axData.length > 1000) {
        axData.shift();
        ayData.shift();
        azData.shift();
        gyroxData.shift();
        gyroyData.shift();
        gyrozData.shift();
        timeLabels.shift();
      }

      drawGraph();
    };

    ws.onclose = () => {
      console.log("Connexion WebSocket fermée");
      isRunning = false;
      document.getElementById("calibrateButton").disabled = false;
    };
  }
}

function stopData() {
  if (isRunning && ws) {
    ws.send("stop");

    // Attendre un petit peu avant de fermer la connexion
    setTimeout(() => {
      ws.close();
      ws = null;
      isRunning = false;

      chronoOffset += (Date.now() - startTime) / 1000;
      console.log("Arrêt des données");
    }, 300); // 300 ms de marge pour laisser le dernier message arriver
  }
}


function resetData() {
  if (!isRunning) {
    document.getElementById("accX").textContent = 0;
    document.getElementById("accY").textContent = 0;
    document.getElementById("accZ").textContent = 0;
    document.getElementById("GyroX").textContent = 0;
    document.getElementById("GyroY").textContent = 0;
    document.getElementById("GyroZ").textContent = 0;
    document.getElementById("Theta").textContent = 0;
    document.getElementById("ThetaDot").textContent = 0;
    document.getElementById("ThetaDotDot").textContent = 0;
    document.getElementById("chrono").textContent = "0.0 s";

    axData = [];
    ayData = [];
    azData = [];
    gyroxData = [];
    gyroyData = [];
    gyrozData = [];
    thetaData = [];
    thetaDotData = [];
    thetaDotDotData = [];
    timeLabels = [];
    dataBuffer = [];

    chronoOffset = 0;  // <-- remet bien le compteur à zéro
    drawGraph();
  }
}

function downloadData() {
  if (dataBuffer.length === 0) {
    alert("Aucune donnée à sauvegarder !");
    return;
  }

  // Récupération des paramètres physiques
  const m = parseFloat(document.getElementById("m").value);
  const g = parseFloat(document.getElementById("g").value);
  const D = parseFloat(document.getElementById("D").value);
  const h = parseFloat(document.getElementById("h").value);
  const fs = parseFloat(document.getElementById("fs").value);

  const exportObject = {
    parametres: { m, g, D, h, fs },
    donnees: dataBuffer
  };

  const blob = new Blob([JSON.stringify(exportObject, null, 2)], { type: "application/json" });
  const url = URL.createObjectURL(blob);
  const link = document.createElement("a");
  const timestamp = new Date().toISOString().replace(/[:.]/g, "_");

  link.href = url;
  link.download = "mesure_" + timestamp + ".json";
  link.click();

  URL.revokeObjectURL(url);
}


function calibrateAcc() {
  isCalibrating = true;  // Marquer que la calibration est en cours
  document.getElementById("calibrateButton").disabled = true; // Désactiver le bouton de calibration pendant la calibration
  
  fetch('/calibrate')
    .then(response => {
      if (response.ok) {
        alert("Accelerometre calibré !");
        document.getElementById("calibrateButton").disabled = false;  // Réactiver le bouton après la calibration
        isCalibrating = false; // Marquer la fin de la calibration
      } else {
        alert("Erreur lors de la calibration.");
        document.getElementById("calibrateButton").disabled = false;  // Réactiver le bouton en cas d'erreur
        isCalibrating = false; // Marquer la fin de la calibration
      }
    })
    .catch(error => {
      console.error("Erreur réseau :", error);
      document.getElementById("calibrateButton").disabled = false;  // Réactiver le bouton en cas d'erreur
      isCalibrating = false; // Marquer la fin de la calibration
    });
}

function calibrateGyro() {
  isCalibrating = true;  // Marquer que la calibration est en cours
  document.getElementById("calibrateButton2").disabled = true; // Désactiver le bouton de calibration pendant la calibration
  
  fetch('/calibrate2')
    .then(response => {
      if (response.ok) {
        alert("Gyroscope calibré !");
        document.getElementById("calibrateButton2").disabled = false;  // Réactiver le bouton après la calibration
        isCalibrating = false; // Marquer la fin de la calibration
      } else {
        alert("Erreur lors de la calibration.");
        document.getElementById("calibrateButton2").disabled = false;  // Réactiver le bouton en cas d'erreur
        isCalibrating = false; // Marquer la fin de la calibration
      }
    })
    .catch(error => {
      console.error("Erreur réseau :", error);
      document.getElementById("calibrateButton2").disabled = false;  // Réactiver le bouton en cas d'erreur
      isCalibrating = false; // Marquer la fin de la calibration
    });
}

function drawGraph() {
  const width = canvas.width;
  const height = canvas.height;
  ctx.clearRect(0, 0, width, height);

  const centerY = height / 2;
  const scale = 2;

  // Fond dégradé
  const gradient = ctx.createLinearGradient(0, 0, 0, height);
  gradient.addColorStop(0, "#f0f8ff");
  gradient.addColorStop(1, "#ffffff");
  ctx.fillStyle = gradient;
  ctx.fillRect(0, 0, width, height);

  // Calcul du temps total
  const duration = timeLabels[timeLabels.length - 1] - timeLabels[0] || 1;
  const stepX = duration > 0 ? width / duration : 1;

  // Repères verticaux (temps)
  ctx.strokeStyle = "#e0e0e0";
  ctx.lineWidth = 1;
  for (let i = 0; i < timeLabels.length; i++) {
    const x = (timeLabels[i] - timeLabels[0]) * stepX;
    ctx.beginPath();
    ctx.moveTo(x, 0);
    ctx.lineTo(x, height);
    ctx.stroke();
  }

  // Axe horizontal
  ctx.strokeStyle = "#000000";
  ctx.lineWidth = 2;
  ctx.beginPath();
  ctx.moveTo(0, centerY);
  ctx.lineTo(width, centerY);
  ctx.stroke();

  // Fonction pour dessiner la courbe
  function drawCurve(data, color) {
    const len = Math.min(data.length, timeLabels.length);
    if (len < 2) return;

    ctx.beginPath();
    let x = 0;
    let y = centerY - data[0] * scale;
    ctx.moveTo(x, y);

    for (let i = 1; i < len; i++) {
      x = (timeLabels[i] - timeLabels[0]) * stepX;
      y = centerY - data[i] * scale;
      ctx.lineTo(x, y);
    }

    ctx.strokeStyle = color;
    ctx.lineWidth = 2;
    ctx.stroke();
  }

  // Dessiner les courbes pour X, Y, Z
  drawCurve(thetaDotData, "#32CD32");      // Vert pour theta

}


// Gestion du clic sur le bouton de calibration
document.getElementById("calibrateButton2").addEventListener("click", () => {
  if (!isRunning && !isCalibrating) {
    calibrateGyro();
  } else {
    alert("Impossible de calibrer pendant la mesure.");
  }
});
// Gestion du clic sur le bouton de calibration
document.getElementById("calibrateButton").addEventListener("click", () => {
  if (!isRunning && !isCalibrating) {
    calibrateAcc();
  } else {
    alert("Impossible de calibrer pendant la mesure.");
  }
});

// Initialise la connexion WebSocket dès le chargement de la page
const socket = new WebSocket("ws://192.168.4.2:8766/"); 

socket.onopen = function () {
  console.log("WebSocket connecté !");
};

socket.onerror = function (error) {
  console.error("Erreur WebSocket :", error);
};

socket.onmessage = function (event) {
  console.log("Message reçu du serveur :", event.data);
};

// Fonction appelée quand on clique sur "Envoyer les paramètres"
function sendUserParams() {
  const m = parseFloat(document.getElementById("m").value);
  const g = parseFloat(document.getElementById("g").value);
  const D = parseFloat(document.getElementById("D").value);
  const h = parseFloat(document.getElementById("h").value);
  const fs = parseFloat(document.getElementById("fs").value);

  if (isNaN(m) || isNaN(g) || isNaN(D) || isNaN(h) || isNaN(fs)) {
    alert("Veuillez remplir tous les champs correctement.");
    return;
  }

  const paramMessage = {
    type: "config",
    m: m,
    g: g,
    D: D,
    h: h,
    fs: fs
  };

  console.log("Envoi des paramètres :", paramMessage);
  socket.send(JSON.stringify(paramMessage));
}
