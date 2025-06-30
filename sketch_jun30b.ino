#define BLYNK_PRINT Serial
#include "HX711.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Blynk Template Info
#define BLYNK_TEMPLATE_ID "TMPL40a_UQmxb"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "8RKKh4GDeJbzh2DHPZuK4p7tBIXJtdrh"

// HX711 Pins
#define DOUT D3
#define CLK  D2
HX711 scale;

// WLAN & Blynk Auth
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Apartment SD68a-012";
char pass[] = "08423168465717724532";

// Blynk V-Pins
#define V_GRAPH            V0
#define V_CALIBRATE_BUTTON V1
#define V_TARGET_WEIGHT    V2
#define V_SOUND_TRIGGER    V3

// Gewicht & Kalibrierung
float calibration_factor = 56.74519; // Kalibrierung anpassen!
float targetWeight = 0.0;
bool targetReached = false;
unsigned long targetHoldStart = 0;
const unsigned long holdTime = 1500; // in ms
float lastWeightSent = -999;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("🔧 Setup startet...");

  scale.begin(DOUT, CLK);
  if (!scale.is_ready()) {
    Serial.println("❌ HX711 NICHT bereit. Verkabelung prüfen.");
  } else {
    Serial.println("✅ HX711 bereit.");
    scale.set_scale(calibration_factor);
    scale.tare();
    Serial.println("🔄 Waage tariert.");
  }

  WiFi.begin(ssid, pass);
  Serial.println("🌐 WLAN-Verbindung gestartet...");
  Blynk.config(auth);
  Serial.println("📲 Blynk konfiguriert.");
}

void loop() {
  // WLAN & Blynk verbinden
  if (WiFi.status() == WL_CONNECTED) {
    if (!Blynk.connected()) {
      Serial.println("🔌 Verbinde mit Blynk...");
      if (Blynk.connect()) {
        Serial.println("✅ Blynk verbunden.");
      }
    } else {
      Blynk.run();
    }
  }

  // Gewicht messen
  if (scale.is_ready()) {
    float weightRaw = scale.get_units(5);
    float weightRounded = round(weightRaw * 10.0) / 10.0;

    // Debug-Ausgabe beider Werte
    Serial.print("RAW Gewicht: ");
    Serial.print(weightRaw, 3);
    Serial.print(" kg | Gerundet: ");
    Serial.print(weightRounded, 1);
    Serial.println(" kg");

    // Optional: auf 0-100 kg beschränken (nur wenn es absolut nötig ist!)
    if (weightRounded < 0) weightRounded = 0;
    if (weightRounded > 100) weightRounded = 100;

    if (abs(weightRounded - lastWeightSent) >= 0.1) {
      if (Blynk.connected()) {
        Blynk.virtualWrite(V_GRAPH, weightRounded);
      }
      lastWeightSent = weightRounded;
    }

    // Zielgewicht prüfen
    if (weightRounded >= targetWeight && targetWeight > 0.0) {
      if (!targetReached) {
        targetHoldStart = millis();
        targetReached = true;
        Serial.println("🎯 Zielgewicht erreicht – Haltezeit startet...");
      } else if (millis() - targetHoldStart >= holdTime) {
        Serial.println("✅ Zielgewicht gehalten – Trigger aktiv!");
        Blynk.virtualWrite(V_SOUND_TRIGGER, 1);
        targetReached = false;
      }
    } else {
      if (targetReached) {
        Serial.println("🔁 Zielgewicht nicht mehr gehalten – zurücksetzen.");
      }
      targetReached = false;
      Blynk.virtualWrite(V_SOUND_TRIGGER, 0);
    }

  } else {
    Serial.println("⚠️ HX711 nicht bereit.");
  }

  delay(200);
}

// === BLYNK CALLBACKS ===

// Kalibrieren (tare)
BLYNK_WRITE(V_CALIBRATE_BUTTON) {
  if (param.asInt() == 1) {
    scale.tare();
    Serial.println("🔄 Waage neu tariert.");
  }
}

// Zielgewicht setzen
BLYNK_WRITE(V_TARGET_WEIGHT) {
  targetWeight = param.asFloat();
  Serial.print("🎯 Neues Zielgewicht: ");
  Serial.println(targetWeight, 1);
}

// Kalibrierfaktor ändern (optional)
BLYNK_WRITE(V3) {
  calibration_factor = param.asFloat();
  scale.set_scale(calibration_factor);
  Serial.print("⚙️ Neuer Kalibrierfaktor: ");
  Serial.println(calibration_factor, 5);
}
