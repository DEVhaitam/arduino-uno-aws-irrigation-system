#include <Arduino.h>

// ============================================================
//  TIPE IRRIGATION — Régulateur ON/OFF avec hystérésis
//  Métriques enregistrées : theta(t), état pompe, volume cumulé
// ============================================================

// ---- Calibration (étape 0) ----
const int   ADC_SEC   = 574;
const int   ADC_SAT   = 243;
const float THETA_MAX = 0.60;

// ---- Paramètres ON/OFF ----
const float THETA_CONSIGNE = 0.50;   // 50%
const float THETA_MIN      = 0.40;   // 40% → pompe ON
const float THETA_MAX_SEUIL= 0.60;   // 60% → pompe OFF

// ---- Matériel ----
const int SENSOR_PIN = A0;
const int RELAY_PIN  = 7;

// ---- Paramètres expérience ----
const float          QMAX_ML_S       = 15.6;         // débit pompe mL/s
const unsigned long  TEST_DURATION   = 1200000UL;    // 20 min en ms
const unsigned long  SAMPLE_INTERVAL = 5000UL;       // mesure toutes les 5s

// ---- Variables ----
unsigned long startTime;
unsigned long pompeTotalMs = 0;      // durée cumulée pompe ON
unsigned long pompeOnSince = 0;      // instant où pompe s'est allumée
bool pompeOn = false;
int  nbCommutations = 0;

// ============================================================
float adcToTheta(int adc) {
  float ratio = (float)(ADC_SEC - adc) / (float)(ADC_SEC - ADC_SAT);
  return constrain(ratio, 0.0, 1.0) * THETA_MAX;
}

// ============================================================
void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // pompe OFF

  Serial.println("=================================================");
  Serial.println("  REGULATEUR ON/OFF — TIPE Irrigation");
  Serial.println("  theta_consigne=50%  theta_min=40%  theta_max=60%");
  Serial.println("=================================================");
  Serial.println("Envoyez n'importe quel caractere pour demarrer...");
  while (Serial.available() == 0) { delay(100); }
  while (Serial.available() > 0)  { Serial.read(); }

  Serial.println();
  Serial.println("t_s,theta_pct,pompe,volume_cumule_mL,nb_commutations");

  startTime = millis();
}

// ============================================================
void loop() {
  unsigned long now     = millis();
  unsigned long elapsed = now - startTime;

  // Fin de l'expérience
  if (elapsed >= TEST_DURATION) {
    digitalWrite(RELAY_PIN, HIGH); // pompe OFF
    if (pompeOn) pompeTotalMs += millis() - pompeOnSince;

    float volumeTotal = (pompeTotalMs / 1000.0) * QMAX_ML_S;

    Serial.println();
    Serial.println("# ======= RÉSULTATS FINAUX =======");
    Serial.print("# Durée pompage total (s) : ");
    Serial.println(pompeTotalMs / 1000.0, 1);
    Serial.print("# Volume eau consommé (mL) : ");
    Serial.println(volumeTotal, 1);
    Serial.print("# Nombre de commutations : ");
    Serial.println(nbCommutations);
    Serial.println("# ================================");
    while (true) { delay(1000); }
  }

  // Lecture capteur
  int   adc   = analogRead(SENSOR_PIN);
  float theta = adcToTheta(adc);

  // Logique ON/OFF avec hystérésis
  if (!pompeOn && theta <= THETA_MIN) {
    digitalWrite(RELAY_PIN, LOW); // pompe ON
    pompeOnSince = millis();
    pompeOn = true;
    nbCommutations++;
  }
  else if (pompeOn && theta >= THETA_MAX_SEUIL) {
    digitalWrite(RELAY_PIN, HIGH); // pompe OFF
    pompeTotalMs += millis() - pompeOnSince;
    pompeOn = false;
    nbCommutations++;
  }

  // Enregistrement toutes les 5s
  static unsigned long lastSample = 0;
  if (now - lastSample >= SAMPLE_INTERVAL) {
    lastSample = now;

    // Volume cumulé à cet instant
    unsigned long pompeMs = pompeTotalMs;
    if (pompeOn) pompeMs += millis() - pompeOnSince;
    float volumeCumule = (pompeMs / 1000.0) * QMAX_ML_S;

    Serial.print(elapsed / 1000.0, 1); Serial.print(",");
    Serial.print(theta * 100.0, 2);    Serial.print(",");
    Serial.print(pompeOn ? 1 : 0);     Serial.print(",");
    Serial.print(volumeCumule, 1);     Serial.print(",");
    Serial.println(nbCommutations);
  }
}
