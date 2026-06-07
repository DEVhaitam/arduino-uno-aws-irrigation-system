#include <Arduino.h>

// ============================================================
//  TIPE IRRIGATION — Régulateur PI avec PWM temporel
//  La pompe est ON/OFF mais on simule un débit proportionnel
//  en découpant des fenêtres de PWM_WINDOW_MS millisecondes
// ============================================================

// ---- Calibration (étape 0) ----
const int   ADC_SEC   = 574;
const int   ADC_SAT   = 243;
const float THETA_MAX = 0.60;

// ---- Consigne ----
const float THETA_CONSIGNE = 0.50;   // 50%

// ---- Gains PI (calculés depuis T_exp=5.7s, R_exp=38461 s/m²) ----
// t_m = T_exp/3 = 1.9s
// Kp  = 1.2 * T_exp / (R_exp * t_m) = 9.4e-5 m³/s
// Ki  = Kp / (2*t_m) = 2.5e-5 m³/s²
// On travaille en unités normalisées (theta entre 0 et 1)
// donc on ramène Kp et Ki à des fractions de Q_max
const float KP = 2.0;    // à ajuster si la régulation oscille
const float KI = 0.05;   // à ajuster si erreur statique persiste

// ---- Matériel ----
const int SENSOR_PIN = A0;
const int RELAY_PIN  = 7;

// ---- Paramètres expérience ----
const float         QMAX_ML_S        = 15.6;
const unsigned long TEST_DURATION    = 1200000UL;  // 20 min
const unsigned long SAMPLE_INTERVAL  = 5000UL;     // mesure toutes les 5s
const unsigned long PWM_WINDOW_MS    = 3000UL;     // fenêtre PWM temporel 3s

// ---- Variables PI ----
float integrale   = 0.0;
float lastTheta   = 0.0;
unsigned long lastPItime = 0;

// ---- Variables mesures ----
unsigned long startTime;
unsigned long pompeTotalMs = 0;
unsigned long pompeOnSince = 0;
bool pompeOn = false;

// ---- Variables PWM temporel ----
unsigned long windowStart  = 0;
float         dutyCycle    = 0.0;  // 0.0 à 1.0

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
  Serial.println("  REGULATEUR PI — TIPE Irrigation");
  Serial.println("  theta_consigne=50%  Kp=2.0  Ki=0.05");
  Serial.println("=================================================");
  Serial.println("Envoyez n'importe quel caractere pour demarrer...");
  while (Serial.available() == 0) { delay(100); }
  while (Serial.available() > 0)  { Serial.read(); }

  Serial.println();
  Serial.println("t_s,theta_pct,commande_pct,pompe,volume_cumule_mL");

  startTime    = millis();
  lastPItime   = millis();
  windowStart  = millis();
}

// ============================================================
void loop() {
  unsigned long now     = millis();
  unsigned long elapsed = now - startTime;

  // Fin de l'expérience
  if (elapsed >= TEST_DURATION) {
    digitalWrite(RELAY_PIN, HIGH);
    if (pompeOn) pompeTotalMs += now - pompeOnSince;

    float volumeTotal = (pompeTotalMs / 1000.0) * QMAX_ML_S;

    Serial.println();
    Serial.println("# ======= RÉSULTATS FINAUX =======");
    Serial.print("# Durée pompage total (s) : ");
    Serial.println(pompeTotalMs / 1000.0, 1);
    Serial.print("# Volume eau consommé (mL) : ");
    Serial.println(volumeTotal, 1);
    Serial.println("# ================================");
    while (true) { delay(1000); }
  }

  // ---- Calcul PI ----
  // Recalculer toutes les secondes
  static unsigned long lastPIcalc = 0;
  if (now - lastPIcalc >= 1000UL) {
    float dt = (now - lastPIcalc) / 1000.0; // en secondes
    lastPIcalc = now;

    int   adc   = analogRead(SENSOR_PIN);
    float theta = adcToTheta(adc);
    float erreur = THETA_CONSIGNE - theta;

    // Terme intégral avec anti-windup
    integrale += erreur * dt;
    integrale  = constrain(integrale, 0.0, 10.0);

    // Commande PI (entre 0 et 1)
    float commande = KP * erreur + KI * integrale;
    dutyCycle = constrain(commande, 0.0, 1.0);
  }

  // ---- PWM temporel ----
  unsigned long posInWindow = (now - windowStart) % PWM_WINDOW_MS;
  unsigned long onDuration  = (unsigned long)(dutyCycle * PWM_WINDOW_MS);

  bool devraitEtreOn = (posInWindow < onDuration);

  if (devraitEtreOn && !pompeOn) {
    digitalWrite(RELAY_PIN, LOW); // pompe ON
    pompeOnSince = now;
    pompeOn = true;
  }
  else if (!devraitEtreOn && pompeOn) {
    digitalWrite(RELAY_PIN, HIGH); // pompe OFF
    pompeTotalMs += now - pompeOnSince;
    pompeOn = false;
  }

  // ---- Enregistrement toutes les 5s ----
  static unsigned long lastSample = 0;
  if (now - lastSample >= SAMPLE_INTERVAL) {
    lastSample = now;

    int   adc   = analogRead(SENSOR_PIN);
    float theta = adcToTheta(adc);

    unsigned long pompeMs = pompeTotalMs;
    if (pompeOn) pompeMs += now - pompeOnSince;
    float volumeCumule = (pompeMs / 1000.0) * QMAX_ML_S;

    Serial.print(elapsed / 1000.0, 1); Serial.print(",");
    Serial.print(theta * 100.0, 2);    Serial.print(",");
    Serial.print(dutyCycle * 100.0, 1);Serial.print(",");
    Serial.print(pompeOn ? 1 : 0);     Serial.print(",");
    Serial.println(volumeCumule, 1);
  }
}
