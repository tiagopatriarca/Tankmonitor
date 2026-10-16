/*
 * =================================================================================
 * Projeto: TankMonitor - Monitor de Nível de Caixa d'Água Inteligente com ESP32
 * Descrição: Sistema IoT para leitura de 3 sensores de nível d'água (25%, 50%, 100%)
 *            usando boias magnéticas reed switch e alerta crítico para nível < 25%.
 * Recursos:
 *   - 3 Sensores Físicos (GPIO 17: 25%, GPIO 18: 50%, GPIO 19: 100%)
 *   - Alerta visual e de sistema quando o nível está abaixo de 25% (todas as boias secas)
 *   - Edição e salvamento da capacidade do tanque em Litros na memória Flash NVS
 *   - Portal de Configuração Wi-Fi Automático (Modo AP + Captive Portal)
 *   - Dashboard Web moderno com animação fluida em tempo real
 *   - Autenticação e troca de senha do administrador (padrão: admin / admin)
 *   - Botão físico de Reset (Segurar BOOT por 5s para resetar Wi-Fi)
 * =================================================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include "web_pages.h"

// --- MAPEAMENTO DE PINOS (GPIO) ---
#define PIN_FLOAT_LOW   17   // Boia 1 (Nível 25% / Reserva) -> Ligar no GND e GPIO 17
#define PIN_FLOAT_MID   18   // Boia 2 (Nível 50% / Meio)    -> Ligar no GND e GPIO 18
#define PIN_FLOAT_TOP   19   // Boia 3 (Nível 100% / Topo)   -> Ligar no GND e GPIO 19
#define PIN_RESET_BTN   0    // Botão físico BOOT da placa ESP32
#define PIN_LED_STATUS  2    // LED onboard (indicação de status)

// --- OBJETOS E CONSTANTES ---
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

DNSServer dnsServer;
WebServer server(80);
Preferences preferences;

// --- VARIÁVEIS DE CONFIGURAÇÃO E ESTADO ---
bool isAPMode = false;
String wifiSSID = "";
String wifiPass = "";
String adminPassword = "admin";
uint32_t tankCapacity = 1000; // Capacidade configurável em Litros (padrão 1000L)

unsigned long btnPressStartTime = 0;
bool isBtnPressed = false;
unsigned long lastLedBlinkTime = 0;
bool ledState = false;

// Reinicialização agendada
bool shouldReboot = false;
unsigned long rebootTime = 0;

// --- ESTRUTURA E LEITURA DOS 3 SENSORES ---
struct TankStatus {
  bool lowWet;
  bool midWet;
  bool topWet;
  int level;
  uint32_t liters;
  bool isAlert;
  bool isError;
};

TankStatus readSensors() {
  TankStatus s;
  // Com INPUT_PULLUP e boia com seta para cima:
  // Submersa (boia sobe) = Contato fecha com GND -> LOW (true)
  // Seca (boia desce)    = Contato abre -> HIGH (false)
  s.lowWet = (digitalRead(PIN_FLOAT_LOW) == LOW);
  s.midWet = (digitalRead(PIN_FLOAT_MID) == LOW);
  s.topWet = (digitalRead(PIN_FLOAT_TOP) == LOW);
  s.isAlert = false;
  s.isError = false;

  if (!s.lowWet && !s.midWet && !s.topWet) {
    // TODAS AS BOIAS SECAS: ABAIXO DE 25% -> ALERTA CRÍTICO
    s.level = 0;
    s.liters = 0;
    s.isAlert = true;
  } else if (s.lowWet && !s.midWet && !s.topWet) {
    // NÍVEL DE RESERVA: 25%
    s.level = 25;
    s.liters = (tankCapacity * 25) / 100;
  } else if (s.lowWet && s.midWet && !s.topWet) {
    // NÍVEL MÉDIO: 50%
    s.level = 50;
    s.liters = (tankCapacity * 50) / 100;
  } else if (s.lowWet && s.midWet && s.topWet) {
    // CAIXA CHEIA: 100%
    s.level = 100;
    s.liters = tankCapacity;
  } else {
    // Inconsistência nos sensores (ex: topo ativo sem o fundo estar ativo)
    s.level = -1;
    s.liters = 0;
    s.isError = true;
  }
  return s;
}

// --- ROTAS DO SERVIDOR WEB ---

// 1. Rota Principal (Dashboard / Portal)
void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

// 2. Rota para Captive Portal (redireciona chamadas no modo AP)
void handleCaptivePortal() {
  server.sendHeader("Location", String("http://192.168.4.1/"), true);
  server.send(302, "text/plain", "");
}

// 3. API: Status atual dos sensores, volume e sistema
void handleApiStatus() {
  TankStatus s = readSensors();
  unsigned long uptimeSec = millis() / 1000;
  int rssi = WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0;
  String ip = isAPMode ? WiFi.softAPIP().toString() : WiFi.localIP().toString();

  String json = "{";
  json += "\"level\":" + String(s.level) + ",";
  json += "\"liters\":" + String(s.liters) + ",";
  json += "\"capacity\":" + String(tankCapacity) + ",";
  json += "\"low\":" + String(s.lowWet ? "true" : "false") + ",";
  json += "\"mid\":" + String(s.midWet ? "true" : "false") + ",";
  json += "\"top\":" + String(s.topWet ? "true" : "false") + ",";
  json += "\"isAlert\":" + String(s.isAlert ? "true" : "false") + ",";
  json += "\"isError\":" + String(s.isError ? "true" : "false") + ",";
  json += "\"ssid\":\"" + (isAPMode ? "Modo Ponto de Acesso" : WiFi.SSID()) + "\",";
  json += "\"ip\":\"" + ip + "\",";
  json += "\"rssi\":" + String(rssi) + ",";
  json += "\"uptime\":" + String(uptimeSec) + ",";
  json += "\"isAP\":" + String(isAPMode ? "true" : "false");
  json += "}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

// 4. API: Atualizar a Capacidade do Tanque (Litros)
void handleApiCapacity() {
  if (server.hasArg("capacity")) {
    int cap = server.arg("capacity").toInt();
    if (cap >= 50 && cap <= 100000) {
      tankCapacity = cap;

      preferences.begin("tank_mon", false);
      preferences.putUInt("capacity", tankCapacity);
      preferences.end();

      server.send(200, "application/json", "{\"success\":true,\"message\":\"Capacidade atualizada para " + String(tankCapacity) + " Litros!\"}");
    } else {
      server.send(400, "application/json", "{\"success\":false,\"message\":\"Valor de capacidade invalido (minimo 50L).\"}");
    }
  } else {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Parametro capacity ausente.\"}");
  }
}

// 5. API: Escanear redes Wi-Fi próximas
void handleApiScan() {
  int n = WiFi.scanNetworks();
  String json = "[";
  for (int i = 0; i < n; ++i) {
    if (i > 0) json += ",";
    json += "{";
    json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI(i));
    json += "}";
  }
  json += "]";
  WiFi.scanDelete();

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

// 6. API: Salvar novas credenciais Wi-Fi
void handleApiSaveWifi() {
  if (server.hasArg("ssid") && server.hasArg("pass")) {
    String newSsid = server.arg("ssid");
    String newPass = server.arg("pass");

    preferences.begin("tank_mon", false);
    preferences.putString("ssid", newSsid);
    preferences.putString("pass", newPass);
    preferences.end();

    server.send(200, "text/plain", "Dados de Wi-Fi salvos com sucesso! O ESP32 ira reiniciar em 3 segundos.");
    
    shouldReboot = true;
    rebootTime = millis() + 3000;
  } else {
    server.send(400, "text/plain", "Parametros invalidos.");
  }
}

// 7. API: Alterar senha do administrador
void handleApiAdminPassword() {
  if (server.hasArg("currentPass") && server.hasArg("newPass")) {
    String curr = server.arg("currentPass");
    String newP = server.arg("newPass");

    if (curr == adminPassword) {
      adminPassword = newP;
      preferences.begin("tank_mon", false);
      preferences.putString("admin_pass", adminPassword);
      preferences.end();

      server.send(200, "application/json", "{\"success\":true,\"message\":\"Senha alterada com sucesso!\"}");
    } else {
      server.send(200, "application/json", "{\"success\":false,\"message\":\"Senha atual incorreta!\"}");
    }
  } else {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Campos obrigatorios ausentes.\"}");
  }
}

// 8. API: Reiniciar ESP32
void handleApiReboot() {
  server.send(200, "text/plain", "Reiniciando ESP32...");
  shouldReboot = true;
  rebootTime = millis() + 1500;
}

// 9. API: Reset de Fábrica (Limpar credenciais)
void handleApiFactoryReset() {
  preferences.begin("tank_mon", false);
  preferences.remove("ssid");
  preferences.remove("pass");
  preferences.end();

  server.send(200, "text/plain", "Configuracoes resetadas! Reiniciando em modo AP...");
  shouldReboot = true;
  rebootTime = millis() + 2000;
}

// --- INICIALIZAÇÃO DO MODO AP (SETUP) ---
void startAccessPoint() {
  isAPMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP("CaixaDagua-Setup");

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  Serial.println("\n[WI-FI] Modo Ponto de Acesso ativado!");
  Serial.print("[WI-FI] Conecte na rede 'CaixaDagua-Setup' e acesse: http://");
  Serial.println(WiFi.softAPIP());
}

// --- SETUP PRINCIPAL ---
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=======================================================");
  Serial.println("   TankMonitor ESP32 Iniciado (3 Sensores + Alerta)    ");
  Serial.println("=======================================================");

  // Configuração dos 3 Pinos das Boias com Pull-Up Interno
  pinMode(PIN_FLOAT_LOW, INPUT_PULLUP);
  pinMode(PIN_FLOAT_MID, INPUT_PULLUP);
  pinMode(PIN_FLOAT_TOP, INPUT_PULLUP);
  pinMode(PIN_RESET_BTN, INPUT_PULLUP);
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, LOW);

  // Carregar configurações da memória Flash NVS
  preferences.begin("tank_mon", false);
  wifiSSID = preferences.getString("ssid", "");
  wifiPass = preferences.getString("pass", "");
  adminPassword = preferences.getString("admin_pass", "admin");
  tankCapacity = preferences.getUInt("capacity", 1000);
  preferences.end();

  Serial.print("[CONFIG] Capacidade configurada: ");
  Serial.print(tankCapacity);
  Serial.println(" Litros.");

  // Tentar conectar se houver Wi-Fi salvo
  if (wifiSSID.length() > 0) {
    Serial.print("[WI-FI] Conectando a rede: ");
    Serial.println(wifiSSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID.c_str(), wifiPass.c_str());

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
      delay(300);
      digitalWrite(PIN_LED_STATUS, !digitalRead(PIN_LED_STATUS));
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n[WI-FI] Conectado com sucesso!");
      Serial.print("[WI-FI] IP na sua rede: http://");
      Serial.println(WiFi.localIP());
      digitalWrite(PIN_LED_STATUS, HIGH);
      isAPMode = false;
    } else {
      Serial.println("\n[WI-FI] Falha ao conectar. Iniciando Modo AP...");
      startAccessPoint();
    }
  } else {
    Serial.println("[WI-FI] Nenhuma rede configurada. Iniciando Modo AP...");
    startAccessPoint();
  }

  // Registro das Rotas do Web Server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleApiStatus);
  server.on("/api/capacity", HTTP_POST, handleApiCapacity);
  server.on("/api/scan", HTTP_GET, handleApiScan);
  server.on("/api/wifi", HTTP_POST, handleApiSaveWifi);
  server.on("/api/admin", HTTP_POST, handleApiAdminPassword);
  server.on("/api/reboot", HTTP_POST, handleApiReboot);
  server.on("/api/factory-reset", HTTP_POST, handleApiFactoryReset);

  // Tratamento de Captive Portal
  server.on("/generate_204", HTTP_GET, handleCaptivePortal);
  server.on("/hotspot-detect.html", HTTP_GET, handleCaptivePortal);
  server.on("/ncsi.txt", HTTP_GET, handleCaptivePortal);
  server.on("/connecttest.txt", HTTP_GET, handleCaptivePortal);
  server.onNotFound(handleCaptivePortal);

  server.begin();
  Serial.println("[HTTP] Servidor Web iniciado na porta 80.");
}

// --- LOOP PRINCIPAL ---
void loop() {
  // 1. Processar DNS Captive Portal se estiver em modo AP
  if (isAPMode) {
    dnsServer.processNextRequest();

    if (millis() - lastLedBlinkTime > 500) {
      lastLedBlinkTime = millis();
      ledState = !ledState;
      digitalWrite(PIN_LED_STATUS, ledState);
    }
  } else {
    // Em modo conectado, se estiver em estado de alerta (< 25%), pisca para avisar visualmente
    TankStatus s = readSensors();
    if (s.isAlert) {
      if (millis() - lastLedBlinkTime > 200) {
        lastLedBlinkTime = millis();
        ledState = !ledState;
        digitalWrite(PIN_LED_STATUS, ledState);
      }
    } else {
      digitalWrite(PIN_LED_STATUS, HIGH);
    }
  }

  // 2. Processar requisições do Servidor Web
  server.handleClient();

  // 3. Checagem do Botão Físico BOOT (Reset de Wi-Fi segurando por 5 segundos)
  if (digitalRead(PIN_RESET_BTN) == LOW) {
    if (!isBtnPressed) {
      isBtnPressed = true;
      btnPressStartTime = millis();
    } else {
      if (millis() - btnPressStartTime > 5000) {
        Serial.println("\n[RESET] Botao BOOT pressionado por 5s! Limpando configuracoes...");
        for (int i = 0; i < 10; i++) {
          digitalWrite(PIN_LED_STATUS, !digitalRead(PIN_LED_STATUS));
          delay(100);
        }
        preferences.begin("tank_mon", false);
        preferences.remove("ssid");
        preferences.remove("pass");
        preferences.end();
        ESP.restart();
      }
    }
  } else {
    isBtnPressed = false;
  }

  // 4. Executar reboot agendado
  if (shouldReboot && millis() >= rebootTime) {
    Serial.println("[SISTEMA] Reiniciando agora...");
    ESP.restart();
  }
}
