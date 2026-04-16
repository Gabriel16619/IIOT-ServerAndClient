
// ──────────────────────────────────────────
// ESP32 Cliente — DHT11 → Envio HTTP
// ──────────────────────────────────────────
//segunda esp

#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

// DHT
#define DHTPIN 14
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// WiFi
const char* ssid     = "IoT-B08";
const char* password = "12345678";

// Servidor (ESP32 #1)
const char* serverIP = "192.168.0.114";

// Controle
unsigned long lastSend = 0;
const long interval = 5000; // 5 segundos

bool wifiConnected = false;

// ──────────────────────────────────────────
// WiFi
// ──────────────────────────────────────────
bool connectWiFi() {

  Serial.println("\n[WiFi] Resetando interface...");

  WiFi.disconnect(true);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("[WiFi] Conectando...");
  Serial.print("[WiFi] SSID: ");
  Serial.println(ssid);

  int tentativas = 0;

  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("\n[WiFi] Conectado!");
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());

    return true;

  } else {

    Serial.println("\n[WiFi] Falha na conexão.");
    return false;

  }
}

// ──────────────────────────────────────────
// Envio HTTP
// ──────────────────────────────────────────
void sendData(float temp, float hum) {

  HTTPClient http;

  String url = "http://" + String(serverIP) +
               "/update?temp=" + String(temp, 1) +
               "&hum=" + String(hum, 1);

  Serial.println("[HTTP] Enviando:");
  Serial.println(url);

  http.begin(url);
  http.setTimeout(3000); // 3 segundos

  int httpCode = http.GET();

  if (httpCode > 0) {

    Serial.print("[HTTP] Código: ");
    Serial.println(httpCode);

    String payload = http.getString();
    Serial.print("[HTTP] Resposta: ");
    Serial.println(payload);

  } else {

    Serial.print("[HTTP] Erro: ");
    Serial.println(httpCode);
  }

  http.end();
}

// ──────────────────────────────────────────
// Setup
// ──────────────────────────────────────────
void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println("\n[BOOT] Inicializando DHT Client...");

  dht.begin();

  wifiConnected = connectWiFi();
}

// ──────────────────────────────────────────
// Loop
// ──────────────────────────────────────────
void loop() {

  // Reconexão WiFi
  if (WiFi.status() != WL_CONNECTED) {

    if (wifiConnected) {
      Serial.println("[WiFi] Conexão perdida!");
      wifiConnected = false;
    }

    delay(2000);
    wifiConnected = connectWiFi();
    return;
  }

  // Envio periódico (sem delay travando)
  unsigned long currentMillis = millis();

  if (currentMillis - lastSend >= interval) {

    lastSend = currentMillis;

    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();

    if (isnan(temp) || isnan(hum)) {
      Serial.println("[DHT] Falha na leitura");
      return;
    }

    Serial.print("[DHT] Temp: ");
    Serial.print(temp);
    Serial.print(" °C | Umidade: ");
    Serial.println(hum);

    sendData(temp, hum);
  }
}