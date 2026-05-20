/**
 * STAGE 1 - Single sensor test
 *
 * Tests one capacitive moisture sensor on A0.
 * No relay, no WiFi, no AI/ML — just reading and printing.
 *
 * Hardware:
 *   - 1x Capacitive soil moisture sensor -> A0
 *
 * Serial monitor: 9600 baud
 * Moisture range: 0 (wet) to 1023 (dry)
 */

#include <Arduino.h>

const int SENSOR_PIN    = A0;
const int DRY_THRESHOLD = 450; // above this = soil is dry (calibrate later)

void setup() {
  Serial.begin(9600);
  while (!Serial) {}

  pinMode(SENSOR_PIN, INPUT);

  Serial.println(F("=== Single Sensor Test ==="));
  Serial.println(F("Dip sensor in water -> value drops"));
  Serial.println(F("Hold in dry air     -> value rises"));
  Serial.println(F("=========================="));
  delay(500);
}

void loop() {
  int raw = analogRead(SENSOR_PIN);

  Serial.print(F("A0: "));
  Serial.print(raw);
  Serial.print(F("  ->  "));
  Serial.println(raw > DRY_THRESHOLD ? F("DRY") : F("WET"));

  delay(1000);
}
