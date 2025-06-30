#include "HX711.h"

#define DOUT D3  // DT (Daten)
#define CLK  D2  // SCK (Takt)

HX711 scale;
float known_weight = 0.0;
bool calibrated = false;
float calibration_factor = 1.0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("------ HX711 Kalibrierungs-Tool ------");

  scale.begin(DOUT, CLK);
   delay(1000);
  if (!scale.is_ready()) {
    Serial.println("❌ HX711 NICHT bereit. Überprüfe Verkabelung!");
  } else {
    Serial.println("✅ HX711 bereit.");
    scale.set_scale(calibration_factor);
    scale.tare();
    Serial.println("Waage tariert.");
  }

  Serial.println("✅ HX711 bereit.");
  Serial.println("Gib 'tare' ein, um die Waage zu nullen.");
  Serial.println("Dann lege ein bekanntes Gewicht auf und gib z.B. 'set 500' ein.");
  Serial.println("Zur Wiederholung einfach 'tare' und erneut 'set XYZ'.");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.equalsIgnoreCase("tare")) {
      Serial.println("Tariere... Bitte entferne alle Gewichte.");
      scale.set_scale();
      scale.tare();
      delay(1000);
      Serial.println("✅ Nullpunkt gesetzt. Jetzt Gewicht auflegen.");
    }

    if (input.startsWith("set ")) {
      String valueStr = input.substring(4);
      known_weight = valueStr.toFloat();

      if (known_weight <= 0) {
        Serial.println("⚠️ Ungültiger Wert. Beispiel: 'set 1000' für 1000g");
        return;
      }

      long reading = scale.get_units(10);
      calibration_factor = (float)reading / known_weight;
      scale.set_scale(calibration_factor);
      calibrated = true;

      Serial.println("✅ Kalibrierung abgeschlossen!");
      Serial.print("Bekanntes Gewicht: ");
      Serial.print(known_weight);
      Serial.println(" g");
      Serial.print("HX711 Rohwert: ");
      Serial.println(reading);
      Serial.print("🔢 → Kalibrierfaktor: ");
      Serial.println(calibration_factor, 5);
      Serial.println("❗ Diesen Wert kannst du in deinen Hauptcode übernehmen.");
    }
  }

  if (calibrated) {
    float weight = scale.get_units(5);
    Serial.print("Gewicht [g]: ");
    Serial.println(weight);
    delay(1000);
  }
}
