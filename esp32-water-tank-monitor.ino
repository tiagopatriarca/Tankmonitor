/*
 * =================================================================================
 * Projeto: Monitor de Nível de Caixa d'Água Inteligente com ESP32
 * Descrição: Sistema IoT para leitura de 3 níveis d'água (25%, 50%, 100%) usando 
 *            duas boias horizontais de contato magnético (Reed Switch).
 * Recursos:
 *   - Portal de Configuração Wi-Fi Automático (Modo AP + Captive Portal)
 *   - Dashboard Web com gráficos modernos e animação em tempo real
 *   - Autenticação e troca de senha do administrador (padrão: admin / admin)
 *   - Armazenamento em memória Flash não-volátil (NVS/Preferences)
 *   - Botão físico de Reset (Segurar BOOT por 5s para resetar Wi-Fi)
 * =================================================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include "web_pages.h"

// --- MAPEAMENTO DE PINOS (GPIO) ---
#define PIN_FLOAT_MID   18   // Boia 1 (Nível 50%) -> Ligar no GND e GPIO 18
#define PIN_FLOAT_TOP   19   // Boia 2 (Nível 100%) -> Ligar no GND e GPIO 19
#define PIN_RESET_BTN   0    // Botão físico BOOT da placa ESP32
#define PIN_LED_STATUS  2    // LED onboard (indicação de status)

// --- OBJETOS E CONSTANTES ---
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

DNSServer dnsServer;
WebServer server(80);
Preferences preferences;

// --- VARIÁVEIS DE ESTADO ---
bool isAPMode = false;
String wifiSSID = "";
String wifiPass = "";
String adminPassword = "admin";

unsigned long btnPressStartTime = 0;
bool isBtnPressed = false;
unsigned long lastLedBlinkTime = 0;
bool ledState = false;

// Reinicialização agendada
bool shouldReboot = false;
unsigned long rebootTime = 0;

// --- FUNÇÃO PARA LER SENSORES ---
struct TankStatus {
  bool midWet;
  bool topWet;
  int level;
  int liters;
  bool isError;
};

TankStatus readSensors() {
  TankStatus s;
  // Com INPUT_PULLUP e boia com seta para cima:
  // Submersa (boia sobe) = Contato fecha com GND -> LOW (true)
  // Seca (boia desce)    = Contato abre -> HIGH (false)
  s.midWet = (digitalRead(PIN_FLOAT_MID) == LOW);
  s.topWet = (digitalRead(PIN_FLOAT_TOP) == LOW);
  s.isError = false;

  if (s.topWet && s.midWet) {
    s.level = 100;
    s.liters = 1000;
  } else if (!s.topWet && s.midWet) {
    s.level = 50;
    s.liters = 500;
  } else if (!s.topWet && !s.midWet) {
    s.level = 25;
    s.liters = 250;
  } else {
    // Inconsistência: Boia de 100% acionada sem a de 50% estar acionada
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

// 2. Rota para Captive Portal (redireciona qualquer chamada no modo AP)
void handleCaptivePortal() {
  server.sendHeader("Location", String("http://192.168.4.1/"), true);
  server.send(302, "text/plain", "");
}

// 3. API: Status atual dos sensores e do sistema
void handleApiStatus() {
  TankStatus s = readSensors();
  unsigned long uptimeSec = millis() / 1000;
  int rssi = WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0;
  String ip = isAPMode ? WiFi.softAPIP().toString() : WiFi.localIP().toString();

  String json = "{";
  json += "\"level\":" + String(s.level) + ",";
  json += "\"liters\":" + String(s.liters) + ",";
  json += "\"mid\":" + String(s.midWet ? "true" : "false") + ",";
  json += "\"top\":" + String(s.topWet ? "true" : "false") + ",";
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

// 4. API: Escanear redes Wi-Fi próximas
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
  WiFi.scanDelete(); // Libera memória da varredura

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

// 5. API: Salvar novas credenciais Wi-Fi
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

// 6. API: Alterar senha do administrador
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

// 7. API: Reiniciar ESP32
void handleApiReboot() {
  server.send(200, "text/plain", "Reiniciando ESP32...");
  shouldReboot = true;
  rebootTime = millis() + 1500;
}

// 8. API: Reset de Fábrica (Limpar credenciais)
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
  Serial.println("\n==========================================");
  Serial.println("   Monitor de Caixa d'Agua ESP32 Iniciado  ");
  Serial.println("==========================================");

  // Configuração dos Pinos com Pullup Interno
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
  preferences.end();

  // Tentar conectar se houver Wi-Fi salvo
  if (wifiSSID.length() > 0) {
    Serial.print("[WI-FI] Conectando a rede: ");
    Serial.println(wifiSSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID.c_str(), wifiPass.c_str());

    unsigned long startAttemptTime = millis();
    // Tenta conectar por 15 segundos
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
      delay(300);
      digitalWrite(PIN_LED_STATUS, !digitalRead(PIN_LED_STATUS));
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n[WI-FI] Conectado com sucesso!");
      Serial.print("[WI-FI] IP na sua rede: http://");
      Serial.println(WiFi.localIP());
      digitalWrite(PIN_LED_STATUS, HIGH); // LED aceso fixo
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
  server.on("/api/scan", HTTP_GET, handleApiScan);
  server.on("/api/wifi", HTTP_POST, handleApiSaveWifi);
  server.on("/api/admin", HTTP_POST, handleApiAdminPassword);
  server.on("/api/reboot", HTTP_POST, handleApiReboot);
  server.on("/api/factory-reset", HTTP_POST, handleApiFactoryReset);

  // Tratamento de Captive Portal no modo AP (Android, Apple, Windows)
  server.on("/generate_204", HTTP_GET, handleCaptivePortal);
  server.on("/hotspot-detect.html", HTTP_GET, handleCaptivePortal);
  server.on("/ncsi.txt", HTTP_GET, handleCaptivePortal);
  server.on("/connecttest.txt", HTTP_GET, handleCaptivePortal);
  server.onNotFound(handleCaptivePortal);

  server.begin();
  Serial.println("[HTTP] Servidor Web iniciado com sucesso na porta 80.");
}

// --- LOOP PRINCIPAL ---
void loop() {
  // 1. Processar DNS Captive Portal se estiver em modo AP
  if (isAPMode) {
    dnsServer.processNextRequest();

    // Pisca o LED mais lento no modo AP
    if (millis() - lastLedBlinkTime > 500) {
      lastLedBlinkTime = millis();
      ledState = !ledState;
      digitalWrite(PIN_LED_STATUS, ledState);
    }
  } else {
    // Modo Conectado: LED fica sempre aceso
    digitalWrite(PIN_LED_STATUS, HIGH);
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
        
        // Pisca rapido para avisar o usuario
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

  // 4. Executar reboot agendado se solicitado via Web
  if (shouldReboot && millis() >= rebootTime) {
    Serial.println("[SISTEMA] Reiniciando agora...");
    ESP.restart();
  }
}
