// ============================================================
//  TIPE IRRIGATION — Identification expérimentale de H(p)
//  Objectif : enregistrer θ(t) et identifier R et T du modèle
//             H(p) = R / (1 + T·p)
//
//  Protocole :
//    1. Sol sec au départ (θ₀ ≈ 0)
//    2. Pompe à Q_max constant pendant PUMP_DURATION secondes
//    3. Enregistrement de θ(t) toutes les SAMPLE_INTERVAL ms
//    4. Affichage CSV sur port série → copier dans tableur
//
//  Matériel : Arduino UNO, capteur capacitif v1.2 (A0),
//             module relais (D7), pompe
// ============================================================

// ---- Paramètres matériel ----

#include <Arduino.h>

const int SENSOR_PIN  = A0;     // Capteur humidité
const int RELAY_PIN   = 2;      // Relais pompe (LOW = ON)

// ---- Calibration (à remplir après étape 0) ----
// Mesurez ADC en sol complètement sec et sol saturé d'eau
int ADC_SEC = 574;              // ← valeur à mesurer (sol sec)
int ADC_SAT = 243;              // ← valeur à mesurer (sol saturé)
float THETA_MAX = 0.6;         // Humidité volumique max du sol (m³/m³)
                                //   ≈ 0.40–0.50 pour un terreau standard

// ---- Paramètres du protocole d'identification ----
const unsigned long PUMP_DURATION    = 60000; // Durée pompe ON (ms)
const unsigned long RECORD_DURATION  = 90000; // Durée totale enregistrement (ms)
const unsigned long SAMPLE_INTERVAL  = 5000;  // Intervalle entre mesures (ms)

// ---- Variables globales ----
unsigned long startTime;
bool pumpOn = false;
bool headerPrinted = false;

// ============================================================
//  Conversion ADC → humidité volumique θ (0.0 à THETA_MAX)
// ============================================================
float adcToTheta(int adc) {
  // Relation linéaire inverse : plus ADC est bas → plus c'est humide
  float ratio = (float)(ADC_SEC - adc) / (float)(ADC_SEC - ADC_SAT);
  ratio = constrain(ratio, 0.0, 1.0);
  return ratio * THETA_MAX;
}

// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Pompe OFF au démarrage

  // Attendre le signal de l'utilisateur avant de lancer
  Serial.println("=================================================");
  Serial.println("  IDENTIFICATION DE H(p) — TIPE Irrigation");
  Serial.println("=================================================");
  Serial.println("Assurez-vous que le sol est sec (theta proche de 0).");
  Serial.println("Envoyez n'importe quel caractere pour demarrer...");

  while (Serial.available() == 0) { delay(100); } // attendre
  while (Serial.available() > 0)  { Serial.read(); } // vider buffer

  Serial.println();
  Serial.println("Démarrage de l'identification !");
  Serial.println();

  // En-tête CSV pour export tableur
  Serial.println("t_s,ADC_brut,theta_pct,pompe");
  headerPrinted = true;

  startTime = millis();
  pumpOn = true;
  digitalWrite(RELAY_PIN, LOW); // Pompe ON
}

// ============================================================
//  LOOP
// ============================================================
void loop() {
  unsigned long elapsed = millis() - startTime;

  // Couper la pompe après PUMP_DURATION
  if (pumpOn && elapsed >= PUMP_DURATION) {
    digitalWrite(RELAY_PIN, HIGH); // Pompe OFF
    pumpOn = false;
    Serial.println("# -- Pompe coupée --");
  }

  // Arrêter l'enregistrement après RECORD_DURATION
  if (elapsed >= RECORD_DURATION) {
    Serial.println("# -- Enregistrement terminé --");
    Serial.println("#");
    Serial.println("# ETAPE SUIVANTE :");
    Serial.println("# 1) Copiez les donnees CSV dans un tableur (Excel / Python)");
    Serial.println("# 2) Tracez theta_pct en fonction de t_s");
    Serial.println("# 3) Ajustez la courbe : theta(t) = R*Q*(1 - exp(-t/T))");
    Serial.println("#    Pour trouver T : theta(T) = 0.632 * theta_max_mesure");
    Serial.println("#    Pour trouver R : R = theta_max_mesure / Q_max");
    Serial.println("# 4) Comparez R_exp et T_exp aux valeurs théoriques :");
    Serial.println("#    R_theo = 1/k    T_theo = L/k");

    while (true) { delay(1000); } // arrêt définitif
  }

  // Mesure et affichage à chaque SAMPLE_INTERVAL
  static unsigned long lastSample = 0;
  if (millis() - lastSample >= SAMPLE_INTERVAL) {
    lastSample = millis();

    int   adc   = analogRead(SENSOR_PIN);
    float theta = adcToTheta(adc);
    float t_sec = elapsed / 1000.0;

    // Format CSV : t_s, ADC brut, theta en %, état pompe
    Serial.print(t_sec, 1);
    Serial.print(",");
    Serial.print(adc);
    Serial.print(",");
    Serial.print(theta * 100.0, 2);   // en pourcentage
    Serial.print(",");
    Serial.println(pumpOn ? 1 : 0);
  }
}

// ============================================================
//  NOTE — Calcul de T graphiquement (méthode des 63%)
//
//  θ(t) = R·Q_max · (1 - e^(-t/T))
//
//  Quand t = T :  θ(T) = 0.632 × θ_∞   (θ_∞ = valeur plateau)
//
//  Donc : repérez θ_∞ sur votre courbe, calculez 0.632×θ_∞,
//  et lisez le temps correspondant → c'est T_exp.
//
//  Calcul de Kp et Ki (méthode Lambda-tuning, 1er ordre) :
//    t_m = T_exp / 3        (constante de réglage = T/3)
//    Kp  = 1.2 × T_exp / (R_exp × t_m)
//    Ki  = Kp / (2 × t_m)
//  ============================================================
