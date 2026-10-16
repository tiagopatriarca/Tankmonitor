#ifndef WEB_PAGES_H
#define WEB_PAGES_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Tank Monitor - Caixa d'Água ESP32</title>
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Plus+Jakarta+Sans:wght@400;500;600;700;800&display=swap" rel="stylesheet">
  <style>
    :root {
      --bg-gradient: radial-gradient(circle at 50% 0%, #151f38 0%, #0a0d17 100%);
      --card-bg: rgba(22, 30, 52, 0.7);
      --card-border: rgba(56, 189, 248, 0.15);
      --text-main: #f8fafc;
      --text-muted: #94a3b8;
      --accent-cyan: #38bdf8;
      --accent-blue: #2563eb;
      --accent-emerald: #10b981;
      --accent-amber: #f59e0b;
      --accent-rose: #f43f5e;
      --water-color: linear-gradient(180deg, rgba(56, 189, 248, 0.85) 0%, rgba(37, 99, 235, 0.95) 100%);
      --font-main: 'Plus Jakarta Sans', system-ui, -apple-system, sans-serif;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: var(--font-main); }
    body { background: var(--bg-gradient); color: var(--text-main); min-height: 100vh; display: flex; flex-direction: column; overflow-x: hidden; }

    #criticalAlertBanner {
      background: linear-gradient(90deg, #dc2626, #b91c1c);
      color: #fff; padding: 12px 20px; font-weight: 800; font-size: 0.95rem; text-align: center;
      display: none; align-items: center; justify-content: center; gap: 12px;
      box-shadow: 0 4px 20px rgba(220, 38, 38, 0.6); animation: pulseAlert 1.5s infinite; z-index: 100;
    }
    @keyframes pulseAlert { 0%, 100% { opacity: 1; transform: scale(1); } 50% { opacity: 0.88; transform: scale(0.995); } }

    header {
      background: rgba(13, 18, 33, 0.9); backdrop-filter: blur(16px); border-bottom: 1px solid rgba(255, 255, 255, 0.08);
      padding: 16px 28px; display: flex; align-items: center; justify-content: space-between; position: sticky; top: 0; z-index: 50;
    }
    .brand { display: flex; align-items: center; gap: 12px; }
    .brand-icon {
      width: 40px; height: 40px; background: linear-gradient(135deg, #0284c7, #38bdf8); border-radius: 12px;
      display: flex; align-items: center; justify-content: center; box-shadow: 0 0 20px rgba(56, 189, 248, 0.4);
    }
    .brand h1 { font-size: 1.25rem; font-weight: 800; background: linear-gradient(90deg, #ffffff, #93c5fd); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }
    .brand span { font-size: 0.75rem; background: rgba(56, 189, 248, 0.15); color: var(--accent-cyan); padding: 2px 8px; border-radius: 20px; font-weight: 600; border: 1px solid rgba(56, 189, 248, 0.3); }

    nav { display: flex; gap: 8px; background: rgba(15, 23, 42, 0.6); padding: 5px; border-radius: 12px; border: 1px solid rgba(255, 255, 255, 0.06); }
    .nav-btn {
      background: transparent; border: none; color: var(--text-muted); padding: 8px 18px; border-radius: 8px;
      font-size: 0.9rem; font-weight: 600; cursor: pointer; transition: all 0.2s ease; display: flex; align-items: center; gap: 8px;
    }
    .nav-btn:hover { color: var(--text-main); background: rgba(255, 255, 255, 0.05); }
    .nav-btn.active { color: #ffffff; background: linear-gradient(135deg, #0284c7, #2563eb); box-shadow: 0 4px 12px rgba(37, 99, 235, 0.35); }

    .user-tag { display: flex; align-items: center; gap: 10px; background: rgba(255, 255, 255, 0.04); padding: 6px 14px; border-radius: 24px; border: 1px solid rgba(255, 255, 255, 0.08); font-size: 0.85rem; color: var(--text-muted); }
    .user-avatar { width: 26px; height: 26px; border-radius: 50%; background: linear-gradient(135deg, #10b981, #059669); display: flex; align-items: center; justify-content: center; font-weight: 700; color: #fff; font-size: 0.75rem; }

    main { max-width: 1200px; margin: 0 auto; padding: 28px 20px; flex: 1; width: 100%; }
    .tab-content { display: none; animation: fadeIn 0.3s ease; }
    .tab-content.active { display: block; }
    @keyframes fadeIn { from { opacity: 0; transform: translateY(8px); } to { opacity: 1; transform: translateY(0); } }

    .dashboard-grid { display: grid; grid-template-columns: 1fr 380px 1.1fr; gap: 28px; align-items: stretch; }
    @media (max-width: 980px) { .dashboard-grid { grid-template-columns: 1fr; } }

    .card {
      background: var(--card-bg); backdrop-filter: blur(16px); border: 1px solid var(--card-border); border-radius: 20px;
      padding: 24px; position: relative; overflow: hidden; box-shadow: 0 10px 30px rgba(0, 0, 0, 0.25);
    }
    .card::before { content: ''; position: absolute; top: 0; left: 0; right: 0; height: 2px; background: linear-gradient(90deg, transparent, var(--accent-cyan), transparent); }
    .card-title { font-size: 0.85rem; font-weight: 700; text-transform: uppercase; letter-spacing: 0.06em; color: var(--text-muted); margin-bottom: 16px; display: flex; align-items: center; justify-content: space-between; }

    .reservoir-card { display: flex; flex-direction: column; align-items: center; justify-content: center; padding: 36px 20px; }
    .tank-container {
      width: 220px; height: 380px; border: 4px solid rgba(255, 255, 255, 0.2); border-radius: 40px; position: relative;
      background: rgba(15, 23, 42, 0.6); backdrop-filter: blur(10px); box-shadow: 0 0 40px rgba(56, 189, 248, 0.15), inset 0 0 20px rgba(0, 0, 0, 0.5); overflow: hidden; margin: 10px 0;
    }
    .marker { position: absolute; left: 0; right: 0; height: 1px; border-top: 2px dashed rgba(255, 255, 255, 0.3); display: flex; justify-content: flex-end; align-items: center; padding-right: 12px; z-index: 10; }
    .marker-label { font-size: 0.72rem; font-weight: 700; color: rgba(255, 255, 255, 0.75); background: rgba(15, 23, 42, 0.85); padding: 2px 6px; border-radius: 6px; }
    .marker-100 { top: 12%; }
    .marker-50 { top: 50%; }
    .marker-25 { top: 75%; }

    .water { position: absolute; bottom: 0; left: 0; right: 0; height: 25%; background: var(--water-color); transition: height 1s cubic-bezier(0.4, 0, 0.2, 1); box-shadow: 0 -4px 20px rgba(56, 189, 248, 0.5); }
    .water.critical {
      background: linear-gradient(180deg, rgba(244, 63, 94, 0.85) 0%, rgba(190, 18, 60, 0.95) 100%);
      box-shadow: 0 -4px 20px rgba(244, 63, 94, 0.6);
    }
    .water-wave {
      position: absolute; top: -12px; left: 0; width: 200%; height: 24px;
      background: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 1200 120' preserveAspectRatio='none'%3E%3Cpath d='M0,0 C150,90 350,-40 500,50 C650,140 900,10 1200,40 L1200,120 L0,120 Z' fill='%2338bdf8' opacity='0.7'/%3E%3C/svg%3E");
      background-size: 600px 24px; animation: waveMove 4s linear infinite;
    }
    .water.critical .water-wave {
      background: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 1200 120' preserveAspectRatio='none'%3E%3Cpath d='M0,0 C150,90 350,-40 500,50 C650,140 900,10 1200,40 L1200,120 L0,120 Z' fill='%23f43f5e' opacity='0.8'/%3E%3C/svg%3E");
      background-size: 600px 24px;
    }
    @keyframes waveMove { 0% { transform: translateX(0); } 100% { transform: translateX(-50%); } }

    .level-badge { font-size: 2.8rem; font-weight: 800; color: #fff; text-shadow: 0 0 25px rgba(56, 189, 248, 0.6); margin-top: 14px; }
    .level-status-text { font-size: 0.95rem; font-weight: 700; color: var(--accent-cyan); letter-spacing: 0.05em; text-transform: uppercase; text-align: center; }

    .stat-box { margin-bottom: 20px; }
    .stat-val { font-size: 1.8rem; font-weight: 700; color: #fff; display: flex; align-items: baseline; gap: 6px; }
    .stat-unit { font-size: 0.9rem; color: var(--text-muted); font-weight: 500; }

    .sensor-item { display: flex; align-items: center; justify-content: space-between; padding: 10px 14px; background: rgba(15, 23, 42, 0.4); border-radius: 12px; margin-bottom: 8px; border: 1px solid rgba(255, 255, 255, 0.04); }
    .sensor-info { display: flex; flex-direction: column; gap: 2px; }
    .sensor-name { font-size: 0.85rem; font-weight: 600; color: #fff; }
    .sensor-desc { font-size: 0.72rem; color: var(--text-muted); }

    .badge-pill { font-size: 0.75rem; font-weight: 700; padding: 4px 10px; border-radius: 20px; display: flex; align-items: center; gap: 6px; }
    .badge-pill.active { background: rgba(16, 185, 129, 0.15); color: var(--accent-emerald); border: 1px solid rgba(16, 185, 129, 0.3); }
    .badge-pill.inactive { background: rgba(148, 163, 184, 0.12); color: var(--text-muted); border: 1px solid rgba(148, 163, 184, 0.2); }
    .dot { width: 7px; height: 7px; border-radius: 50%; }
    .badge-pill.active .dot { background: var(--accent-emerald); box-shadow: 0 0 8px var(--accent-emerald); }
    .badge-pill.inactive .dot { background: var(--text-muted); }

    .form-group { margin-bottom: 18px; }
    label { display: block; font-size: 0.85rem; font-weight: 600; color: var(--text-muted); margin-bottom: 8px; }
    input, select { width: 100%; background: rgba(15, 23, 42, 0.7); border: 1px solid rgba(255, 255, 255, 0.12); border-radius: 10px; padding: 12px 16px; color: #fff; font-size: 0.95rem; outline: none; }
    input:focus, select:focus { border-color: var(--accent-cyan); box-shadow: 0 0 10px rgba(56, 189, 248, 0.2); }

    .btn-action {
      background: linear-gradient(135deg, #0284c7, #2563eb); color: #fff; border: none; border-radius: 10px; padding: 12px 24px;
      font-size: 0.95rem; font-weight: 700; cursor: pointer; display: inline-flex; align-items: center; gap: 10px;
      box-shadow: 0 4px 14px rgba(37, 99, 235, 0.4); transition: all 0.2s;
    }
    .btn-action:hover { transform: translateY(-2px); box-shadow: 0 6px 20px rgba(37, 99, 235, 0.5); }
    .btn-danger { background: linear-gradient(135deg, #e11d48, #be123c); box-shadow: 0 4px 14px rgba(225, 29, 72, 0.4); }
    .btn-secondary { background: rgba(255, 255, 255, 0.08); border: 1px solid rgba(255, 255, 255, 0.15); box-shadow: none; }
    .btn-secondary:hover { background: rgba(255, 255, 255, 0.15); }

    .wifi-list { display: flex; flex-direction: column; gap: 10px; margin-top: 15px; max-height: 250px; overflow-y: auto; }
    .wifi-item { display: flex; align-items: center; justify-content: space-between; padding: 12px 16px; background: rgba(15, 23, 42, 0.5); border-radius: 10px; border: 1px solid rgba(255, 255, 255, 0.08); cursor: pointer; }
    .wifi-item:hover, .wifi-item.selected { border-color: var(--accent-cyan); background: rgba(56, 189, 248, 0.1); }
    .wifi-ssid { font-size: 0.9rem; font-weight: 600; color: #fff; }
    .wifi-signal { font-size: 0.8rem; color: var(--text-muted); }

    #toast { position: fixed; bottom: 24px; right: 24px; background: #1e293b; color: #fff; padding: 14px 22px; border-radius: 12px; border: 1px solid var(--accent-cyan); box-shadow: 0 10px 25px rgba(0, 0, 0, 0.5); transform: translateY(100px); opacity: 0; transition: all 0.3s; z-index: 100; font-size: 0.9rem; font-weight: 600; }
    #toast.show { transform: translateY(0); opacity: 1; }
    .ap-banner { background: linear-gradient(90deg, #f59e0b, #d97706); color: #000; font-weight: 700; text-align: center; padding: 8px 12px; font-size: 0.85rem; display: none; }
  </style>
</head>
<body>

  <!-- Critical Alert Banner (Visible when < 25%) -->
  <div id="criticalAlertBanner">
    <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="#fff" stroke-width="2.5"><path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z"></path><line x1="12" y1="9" x2="12" y2="13"></line><line x1="12" y1="17" x2="12.01" y2="17"></line></svg>
    <span>🚨 ALERTA CRÍTICO: NÍVEL DA CAIXA D'ÁGUA ABAIXO DE 25%! Risco de desabastecimento!</span>
  </div>

  <div id="apBanner" class="ap-banner">⚠️ MODO DE CONFIGURAÇÃO (PONTO DE ACESSO): Conecte o ESP32 ao Wi-Fi residencial na aba "Config Wi-Fi".</div>

  <header>
    <div class="brand">
      <div class="brand-icon">
        <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="#fff" stroke-width="2.5"><path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"></path></svg>
      </div>
      <div><h1>Tank Monitor</h1></div>
      <span>3 Sensores IoT</span>
    </div>

    <nav>
      <button class="nav-btn active" onclick="switchTab('dashboard')">Painel</button>
      <button class="nav-btn" onclick="switchTab('wifi')">Config Wi-Fi</button>
      <button class="nav-btn" onclick="switchTab('admin')">Config & Admin</button>
    </nav>

    <div class="user-tag">
      <div class="user-avatar">A</div>
      <span id="headerUser">admin</span>
    </div>
  </header>

  <main>
    <!-- TAB 1: DASHBOARD -->
    <section id="tab-dashboard" class="tab-content active">
      <div class="dashboard-grid">
        
        <!-- Left Stats -->
        <div class="card">
          <div class="card-title">Telemetria da Caixa</div>
          <div class="stat-box">
            <label>Volume Atual / Capacidade</label>
            <div class="stat-val">
              <span id="litersDisplay">--</span> 
              <span class="stat-unit">L / <span id="capacityStatDisplay">--</span> L</span>
            </div>
          </div>
          <div class="stat-box">
            <label>Status Operacional</label>
            <div id="quickStatusBadge" style="font-size: 1.1rem; font-weight: 700; color: var(--accent-emerald);">Lendo sensores...</div>
          </div>
          <div class="stat-box">
            <label>Tempo de Atividade</label>
            <div class="stat-val" style="font-size: 1.25rem;" id="uptimeDisplay">00h 00m 00s</div>
          </div>
          <div class="stat-box">
            <label>Última Leitura</label>
            <div style="font-size: 0.9rem; color: #cbd5e1;" id="lastReadTime">Conectando...</div>
          </div>
        </div>

        <!-- Center Tank Graphic -->
        <div class="card reservoir-card">
          <div class="card-title" style="width: 100%;">Nível da Caixa d'Água</div>
          
          <div class="tank-container">
            <div class="marker marker-100"><span class="marker-label">100% (Cheia)</span></div>
            <div class="marker marker-50"><span class="marker-label">50% (Média)</span></div>
            <div class="marker marker-25"><span class="marker-label">25% (Reserva)</span></div>
            <div class="water" id="waterLevel">
              <div class="water-wave"></div>
            </div>
          </div>

          <div class="level-badge" id="levelPercent">--%</div>
          <div class="level-status-text" id="levelStatus">Iniciando...</div>
        </div>

        <!-- Right Stats: 3 SENSORS -->
        <div class="card">
          <div class="card-title">Status dos 3 Sensores Físicos</div>
          
          <div class="sensor-item">
            <div class="sensor-info">
              <span class="sensor-name">Boia 3 (Topo / 100%)</span>
              <span class="sensor-desc">GPIO 19</span>
            </div>
            <div class="badge-pill inactive" id="sensorTopBadge">
              <span class="dot"></span>
              <span id="sensorTopText">Seca</span>
            </div>
          </div>

          <div class="sensor-item">
            <div class="sensor-info">
              <span class="sensor-name">Boia 2 (Meio / 50%)</span>
              <span class="sensor-desc">GPIO 18</span>
            </div>
            <div class="badge-pill inactive" id="sensorMidBadge">
              <span class="dot"></span>
              <span id="sensorMidText">Seca</span>
            </div>
          </div>

          <div class="sensor-item">
            <div class="sensor-info">
              <span class="sensor-name">Boia 1 (Baixo / 25%)</span>
              <span class="sensor-desc">GPIO 17</span>
            </div>
            <div class="badge-pill inactive" id="sensorLowBadge">
              <span class="dot"></span>
              <span id="sensorLowText">Seca</span>
            </div>
          </div>

          <div class="card-title" style="margin-top: 20px;">Conexão</div>
          <div class="sensor-item">
            <div class="sensor-info">
              <span class="sensor-name">Rede Wi-Fi</span>
              <span class="sensor-desc" id="wifiSsidName">--</span>
            </div>
            <span id="wifiConnBadge" style="font-size: 0.8rem; color: var(--accent-emerald); font-weight: 700;">OK</span>
          </div>

          <div class="sensor-item">
            <div class="sensor-info">
              <span class="sensor-name">Endereço IP</span>
              <span class="sensor-desc" id="ipAddress">--</span>
            </div>
            <span id="rssiDisplay" style="font-size: 0.8rem; color: var(--accent-cyan); font-weight: 700;">-- dBm</span>
          </div>
        </div>

      </div>
    </section>

    <!-- TAB 2: WIFI CONFIG -->
    <section id="tab-wifi" class="tab-content">
      <div style="max-width: 600px; margin: 0 auto;" class="card">
        <div class="card-title">Configuração de Conexão Wi-Fi</div>
        <p style="font-size: 0.85rem; color: var(--text-muted); margin-bottom: 20px;">
          Escaneie as redes 2.4 GHz disponíveis ao redor do ESP32 ou selecione uma manualmente para salvar na memória flash NVS.
        </p>

        <button class="btn-action btn-secondary" onclick="scanWifi()" id="btnScan" style="margin-bottom: 20px; width: 100%; justify-content: center;">
          Escanear Redes Wi-Fi Próximas
        </button>

        <div id="wifiScanResults" class="wifi-list" style="display: none;"></div>

        <form id="wifiForm" onsubmit="saveWifi(event)" style="margin-top: 20px;">
          <div class="form-group">
            <label for="ssidInput">Nome da Rede Wi-Fi (SSID)</label>
            <input type="text" id="ssidInput" placeholder="Selecione acima ou digite..." required>
          </div>

          <div class="form-group">
            <label for="passInput">Senha da Rede Wi-Fi</label>
            <input type="password" id="passInput" placeholder="Digite a senha..." required>
          </div>

          <button type="submit" class="btn-action" style="width: 100%; justify-content: center;">
            Salvar e Conectar
          </button>
        </form>
      </div>
    </section>

    <!-- TAB 3: ADMIN & CONFIG -->
    <section id="tab-admin" class="tab-content">
      <div style="max-width: 600px; margin: 0 auto; display: flex; flex-direction: column; gap: 24px;">
        
        <!-- Configuração da Capacidade do Tanque -->
        <div class="card">
          <div class="card-title">Configuração da Caixa d'Água</div>
          <form onsubmit="saveTankCapacity(event)">
            <div class="form-group">
              <label for="tankCapacityInput">Capacidade Total do Reservatório (em Litros)</label>
              <input type="number" id="tankCapacityInput" min="50" max="50000" step="50" required>
              <span style="font-size: 0.75rem; color: var(--text-muted); display: block; margin-top: 6px;">
                Exemplos comuns: 500 L, 1000 L, 1500 L, 2000 L, 5000 L. O sistema calculará o volume em litros automaticamente com base neste valor.
              </span>
            </div>

            <button type="submit" class="btn-action" style="width: 100%; justify-content: center;">
              Salvar Capacidade em Litros
            </button>
          </form>
        </div>

        <!-- Alterar Senha -->
        <div class="card">
          <div class="card-title">Alterar Senha do Administrador</div>
          <form onsubmit="changeAdminPassword(event)">
            <div class="form-group">
              <label>Usuário</label>
              <input type="text" value="admin" disabled style="opacity: 0.6; cursor: not-allowed;">
            </div>

            <div class="form-group">
              <label for="currentPass">Senha Atual</label>
              <input type="password" id="currentPass" placeholder="Senha padrão: admin" required>
            </div>

            <div class="form-group">
              <label for="newPass">Nova Senha</label>
              <input type="password" id="newPass" placeholder="Mínimo 4 caracteres" required>
            </div>

            <div class="form-group">
              <label for="confirmPass">Confirmar Nova Senha</label>
              <input type="password" id="confirmPass" placeholder="Repita a nova senha" required>
            </div>

            <button type="submit" class="btn-action" style="width: 100%; justify-content: center;">
              Atualizar Senha
            </button>
          </form>
        </div>

        <!-- System Controls Card -->
        <div class="card">
          <div class="card-title">Manutenção e Reinicialização</div>
          <div style="display: flex; gap: 12px; flex-wrap: wrap;">
            <button class="btn-action btn-secondary" onclick="rebootEsp()">Reiniciar ESP32</button>
            <button class="btn-action btn-danger" onclick="factoryReset()">Resetar Wi-Fi (Modo Setup)</button>
          </div>
        </div>

      </div>
    </section>
  </main>

  <div id="toast">Mensagem do sistema</div>

  <script>
    function switchTab(tabId) {
      document.querySelectorAll('.tab-content').forEach(el => el.classList.remove('active'));
      document.querySelectorAll('.nav-btn').forEach(el => el.classList.remove('active'));
      document.getElementById('tab-' + tabId).classList.add('active');
      const targetBtn = Array.from(document.querySelectorAll('.nav-btn')).find(b => b.getAttribute('onclick').includes(tabId));
      if (targetBtn) targetBtn.classList.add('active');
    }

    function showToast(msg) {
      const toast = document.getElementById('toast');
      toast.innerText = msg;
      toast.classList.add('show');
      setTimeout(() => toast.classList.remove('show'), 3500);
    }

    function formatUptime(seconds) {
      const h = Math.floor(seconds / 3600);
      const m = Math.floor((seconds % 3600) / 60);
      const s = seconds % 60;
      return `${String(h).padStart(2,'0')}h ${String(m).padStart(2,'0')}m ${String(s).padStart(2,'0')}s`;
    }

    function updateBadge(badge, textEl, isActive) {
      if (isActive) {
        badge.className = 'badge-pill active';
        textEl.innerText = 'Submersa';
      } else {
        badge.className = 'badge-pill inactive';
        textEl.innerText = 'Seca';
      }
    }

    let loadedCapacity = false;

    function fetchStatus() {
      fetch('/api/status')
        .then(res => res.json())
        .then(data => {
          const water = document.getElementById('waterLevel');
          const levelPercent = document.getElementById('levelPercent');
          const levelStatus = document.getElementById('levelStatus');
          const liters = document.getElementById('litersDisplay');
          const alertBanner = document.getElementById('criticalAlertBanner');
          const quickStatus = document.getElementById('quickStatusBadge');

          const lowBadge = document.getElementById('sensorLowBadge');
          const lowText = document.getElementById('sensorLowText');
          const midBadge = document.getElementById('sensorMidBadge');
          const midText = document.getElementById('sensorMidText');
          const topBadge = document.getElementById('sensorTopBadge');
          const topText = document.getElementById('sensorTopText');

          // Update 3 Sensors
          updateBadge(lowBadge, lowText, data.low);
          updateBadge(midBadge, midText, data.mid);
          updateBadge(topBadge, topText, data.top);

          // Level Display
          if (data.level === 100) {
            water.style.height = '100%';
            water.className = 'water';
            levelPercent.innerText = '100%';
            levelPercent.style.color = '#38bdf8';
            levelStatus.innerText = 'Caixa Cheia (100%)';
            levelStatus.style.color = '#38bdf8';
            liters.innerText = data.liters.toLocaleString('pt-BR');
            alertBanner.style.display = 'none';
            quickStatus.innerText = 'Reservatório Completo';
            quickStatus.style.color = '#38bdf8';
          } else if (data.level === 50) {
            water.style.height = '50%';
            water.className = 'water';
            levelPercent.innerText = '50%';
            levelPercent.style.color = '#60a5fa';
            levelStatus.innerText = 'Nível Médio (50%)';
            levelStatus.style.color = '#60a5fa';
            liters.innerText = data.liters.toLocaleString('pt-BR');
            alertBanner.style.display = 'none';
            quickStatus.innerText = 'Abastecimento Normal';
            quickStatus.style.color = '#10b981';
          } else if (data.level === 25) {
            water.style.height = '25%';
            water.className = 'water';
            levelPercent.innerText = '25%';
            levelPercent.style.color = '#f59e0b';
            levelStatus.innerText = 'Nível de Reserva (25%)';
            levelStatus.style.color = '#f59e0b';
            liters.innerText = data.liters.toLocaleString('pt-BR');
            alertBanner.style.display = 'none';
            quickStatus.innerText = 'Nível Baixo (Atenção)';
            quickStatus.style.color = '#f59e0b';
          } else if (data.level === 0) {
            // CRITICAL ALERT: Below 25%
            water.style.height = '10%';
            water.className = 'water critical';
            levelPercent.innerText = '< 25%';
            levelPercent.style.color = '#f43f5e';
            levelStatus.innerText = '🚨 ALERTA CRÍTICO: ABAIXO DE 25%';
            levelStatus.style.color = '#f43f5e';
            liters.innerText = `< ${(data.capacity * 0.25).toLocaleString('pt-BR')}`;
            alertBanner.style.display = 'flex';
            quickStatus.innerText = '⚠️ Risco de Desabastecimento';
            quickStatus.style.color = '#f43f5e';
          } else {
            water.style.height = '20%';
            water.className = 'water critical';
            levelPercent.innerText = '⚠️ Erro';
            levelPercent.style.color = '#f43f5e';
            levelStatus.innerText = 'Inconsistência nos Sensores (Checar Boias)';
            levelStatus.style.color = '#f43f5e';
            liters.innerText = '--';
            alertBanner.style.display = 'none';
            quickStatus.innerText = 'Falha de Sensor';
            quickStatus.style.color = '#f43f5e';
          }

          document.getElementById('capacityStatDisplay').innerText = data.capacity.toLocaleString('pt-BR');
          if (!loadedCapacity) {
            document.getElementById('tankCapacityInput').value = data.capacity;
            loadedCapacity = true;
          }

          document.getElementById('uptimeDisplay').innerText = formatUptime(data.uptime);
          document.getElementById('wifiSsidName').innerText = data.ssid || 'CaixaDagua-Setup';
          document.getElementById('ipAddress').innerText = data.ip;
          document.getElementById('rssiDisplay').innerText = (data.rssi ? data.rssi + ' dBm' : 'Modo AP');
          document.getElementById('lastReadTime').innerText = new Date().toLocaleTimeString('pt-BR');

          const apBanner = document.getElementById('apBanner');
          if (data.isAP) {
            apBanner.style.display = 'block';
          } else {
            apBanner.style.display = 'none';
          }
        })
        .catch(err => {
          document.getElementById('lastReadTime').innerText = 'Falha na conexão...';
        });
    }

    function saveTankCapacity(e) {
      e.preventDefault();
      const val = parseInt(document.getElementById('tankCapacityInput').value, 10);
      if (isNaN(val) || val < 50) {
        showToast('❌ Capacidade mínima é de 50 Litros.');
        return;
      }

      const formData = new FormData();
      formData.append('capacity', val);

      fetch('/api/capacity', {
        method: 'POST',
        body: formData
      })
      .then(res => res.json())
      .then(data => {
        if (data.success) {
          showToast('✅ ' + data.message);
          fetchStatus();
        } else {
          showToast('❌ ' + data.message);
        }
      })
      .catch(err => {
        showToast('❌ Erro de comunicação com o ESP32.');
      });
    }

    function scanWifi() {
      const btn = document.getElementById('btnScan');
      const list = document.getElementById('wifiScanResults');
      btn.innerText = 'Escaneando redes 2.4 GHz...';
      btn.disabled = true;

      fetch('/api/scan')
        .then(res => res.json())
        .then(networks => {
          btn.innerText = 'Escanear Redes Wi-Fi Próximas';
          btn.disabled = false;
          list.style.display = 'flex';
          list.innerHTML = '';

          if (networks.length === 0) {
            list.innerHTML = '<div style="color: #94a3b8; padding: 12px; text-align: center;">Nenhuma rede 2.4 GHz encontrada. Tente novamente.</div>';
            return;
          }

          networks.forEach(net => {
            const div = document.createElement('div');
            div.className = 'wifi-item';
            div.innerHTML = `
              <span class="wifi-ssid">${net.ssid}</span>
              <span class="wifi-signal">${net.rssi} dBm</span>
            `;
            div.onclick = () => {
              document.querySelectorAll('.wifi-item').forEach(i => i.classList.remove('selected'));
              div.classList.add('selected');
              document.getElementById('ssidInput').value = net.ssid;
              document.getElementById('passInput').focus();
            };
            list.appendChild(div);
          });
          showToast(`${networks.length} redes encontradas!`);
        })
        .catch(err => {
          btn.innerText = 'Escanear Redes Wi-Fi Próximas';
          btn.disabled = false;
          showToast('Erro ao escanear redes.');
        });
    }

    function saveWifi(e) {
      e.preventDefault();
      const ssid = document.getElementById('ssidInput').value;
      const pass = document.getElementById('passInput').value;

      showToast(`Enviando credenciais... O ESP32 conectará na rede "${ssid}"!`);
      const formData = new FormData();
      formData.append('ssid', ssid);
      formData.append('pass', pass);

      fetch('/api/wifi', { method: 'POST', body: formData })
        .then(res => res.text())
        .then(msg => showToast(msg))
        .catch(() => showToast('Credenciais enviadas! Aguarde o ESP32 conectar.'));
    }

    function changeAdminPassword(e) {
      e.preventDefault();
      const curr = document.getElementById('currentPass').value;
      const newP = document.getElementById('newPass').value;
      const conf = document.getElementById('confirmPass').value;

      if (newP !== conf) {
        showToast('❌ A nova senha e confirmação não coincidem!');
        return;
      }
      if (newP.length < 4) {
        showToast('❌ A nova senha deve ter no mínimo 4 caracteres!');
        return;
      }

      const formData = new FormData();
      formData.append('currentPass', curr);
      formData.append('newPass', newP);

      fetch('/api/admin', { method: 'POST', body: formData })
        .then(res => res.json())
        .then(data => {
          if (data.success) {
            showToast('✅ ' + data.message);
            e.target.reset();
          } else {
            showToast('❌ ' + data.message);
          }
        })
        .catch(() => showToast('❌ Erro de comunicação com o ESP32.'));
    }

    function rebootEsp() {
      if (confirm('Deseja realmente reiniciar o ESP32 agora?')) {
        fetch('/api/reboot', { method: 'POST' });
        showToast('Reiniciando microcontrolador... Aguarde 10 segundos.');
      }
    }

    function factoryReset() {
      if (confirm('ATENÇÃO: Deseja apagar a rede salva e voltar para o modo CaixaDagua-Setup?')) {
        fetch('/api/factory-reset', { method: 'POST' });
        showToast('Redefinindo padrões de fábrica...');
      }
    }

    fetchStatus();
    setInterval(fetchStatus, 2000);
  </script>
</body>
</html>
)rawliteral";

#endif
