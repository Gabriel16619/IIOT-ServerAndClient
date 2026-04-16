// ──────────────────────────────────────────
// ESP32 Web Server — LED + DHT11 Dashboard
// ──────────────────────────────────────────
//primeira esp32

#include <WiFi.h>
#include <WebServer.h>

// WiFi
const char* ssid     = "IoT-B08";
const char* password = "12345678";

// Login HTTP
const char* http_user = "admin";
const char* http_pass = "1234";

// Servidor
WebServer server(80);

// LED
const int LED_PIN = 32;
bool ledState = false;

// DHT (dados recebidos do outro ESP)
float temperatura = 0;
float umidade = 0;

// Estado WiFi
bool wifiConnected = false;

// ──────────────────────────────────────────
// Autenticação
// ──────────────────────────────────────────
bool isAuthenticated() {

  if (!server.authenticate(http_user, http_pass)) {
    server.requestAuthentication();
    return false;
  }

  return true;
}

// ──────────────────────────────────────────
// HTML
// ──────────────────────────────────────────
String buildPage() {

 

  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width'>";
  html += "<title>ESP32 Dashboard</title>";
  html += "<meta http-equiv='refresh' content='5'>";

  html += "<style>";
  html += "body{font-family:sans-serif;text-align:center;padding:40px;background:#f4f4f4}";
  html += "button{padding:16px 32px;font-size:18px;border:none;border-radius:8px;margin:8px}";
  html += ".on{background:#009d00;color:#fff}";
  html += ".off{background:#F4320B;color:#fff}";
  html += "</style>";

  html += "</head><body>";

  html += "<h1>Controle de LED</h1>";

  html += "<p>Status: <strong>";
  html += (ledState ? "LIGADO" : "DESLIGADO");
  html += "</strong></p>";

  html += "<a href='/on'><button class='on'>LIGAR</button></a>";
  html += "<a href='/off'><button class='off'>DESLIGAR</button></a>";

  html += "<hr>";

  html += "<h2>Sensor DHT11</h2>";

  html += "<p>Temperatura: ";
  html += String(temperatura);
  html += " °C</p>";

  html += "<p>Umidade: ";
  html += String(umidade);
  html += " %</p>";

  html += "</body></html>";

  return html;
}

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
// Rotas
// ──────────────────────────────────────────
void setupRoutes() {

  // Página principal
  server.on("/", []() {

    if (!isAuthenticated()) return;

    server.send(200, "text/html", buildPage());
  });

  // Ligar LED
  server.on("/on", []() {

    if (!isAuthenticated()) return;

    ledState = true;
    digitalWrite(LED_PIN, HIGH);

    Serial.println("[LED] Ligado");

    server.sendHeader("Location", "/");
    server.send(303);
  });

  // Desligar LED
  server.on("/off", []() {

    if (!isAuthenticated()) return;

    ledState = false;
    digitalWrite(LED_PIN, LOW);

    Serial.println("[LED] Desligado");

    server.sendHeader("Location", "/");
    server.send(303);
  });

  // Receber dados do ESP DHT
  server.on("/update", []() {

    // SEM autenticação aqui

    if (server.hasArg("temp")) {
      temperatura = server.arg("temp").toFloat();
    }

    if (server.hasArg("hum")) {
      umidade = server.arg("hum").toFloat();
    }

    Serial.println("[DHT] Dados recebidos:");
    Serial.print("Temp: ");
    Serial.println(temperatura);
    Serial.print("Umidade: ");
    Serial.println(umidade);

    server.send(200, "text/plain", "OK");
  });
}

// ──────────────────────────────────────────
// Setup
// ──────────────────────────────────────────
void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println("\n[BOOT] Inicializando ESP32...");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  wifiConnected = connectWiFi();

  setupRoutes();

  if (wifiConnected) {

    server.begin();
    Serial.println("[HTTP] Servidor iniciado!");

  } else {

    Serial.println("[HTTP] Servidor NÃO iniciado (sem WiFi)");
  }
}

// ──────────────────────────────────────────
// Loop
// ──────────────────────────────────────────
void loop() {

  if (WiFi.status() != WL_CONNECTED) {

    if (wifiConnected) {
      Serial.println("[WiFi] Conexão perdida!");
      wifiConnected = false;
    }

    delay(2000);
    wifiConnected = connectWiFi();

    if (wifiConnected) {
      server.begin();
      Serial.println("[HTTP] Servidor reiniciado!");
    }

    return;
  }

  server.handleClient();
}
