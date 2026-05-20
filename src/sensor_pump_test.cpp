/**
 * STAGE 2 - Single sensor + pump relay test
 *
 * Reads one moisture sensor on A0 and controls one pump
 * via a relay on D2. Pump turns ON when soil is dry.
 *
 * Hardware:
 *   - 1x Capacitive soil moisture sensor -> A0
 *   - 1x Relay channel IN1              -> D2
 *     (relay VCC -> 5V, relay GND -> GND)
 *
 * Relay is active LOW:
 *   LOW  = relay energised = pump ON
 *   HIGH = relay off       = pump OFF
 *
 * Serial monitor: 9600 baud
 */

#include <Arduino.h>

const int SENSOR_PIN    = A0;
const int RELAY_PIN     = 2;
const int DRY_THRESHOLD = 450; // above this = soil is dry -> pump ON

void setup() {
  Serial.begin(9600);
  while (!Serial) {}

  // Drive HIGH before pinMode to avoid a brief ON pulse at boot
  digitalWrite(RELAY_PIN, HIGH);
  pinMode(RELAY_PIN, OUTPUT);

  pinMode(SENSOR_PIN, INPUT);

  Serial.println(F("=== Sensor + Pump Test ==="));
  Serial.println(F("Pump starts OFF."));
  Serial.println(F("=========================="));
  delay(500);
}

void loop() {
  int raw = analogRead(SENSOR_PIN);
  bool isDry = raw > DRY_THRESHOLD;

  // Active LOW relay: LOW = pump ON
  digitalWrite(RELAY_PIN, isDry ? LOW : HIGH);

  Serial.print(F("A0: "));
  Serial.print(raw);
  Serial.print(F("  ->  "));
  Serial.print(isDry ? F("DRY") : F("WET"));
  Serial.print(F("  |  Pump: "));
  Serial.println(isDry ? F("ON") : F("OFF"));

  delay(1000);
}
