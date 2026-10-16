# 🛠️ Guia de Montagem e Gravação: TankMonitor ESP32 (3 Sensores)

---

## 1. 🧪 Teste Imediato da Nova Interface

Você pode testar a nova física dos **3 Sensores**, o **Alerta Crítico (< 25%)** e a **Edição de Litros** no simulador web:
1. Abra o arquivo [dashboard_preview.html](file:///C:/Users/DATEN/.gemini/antigravity-ide/scratch/esp32-water-tank-monitor/dashboard_preview.html) no seu navegador.
2. Use o **"Simulador de Bancada"** para testar:
   * **🚨 Abaixo de 25%**: O banner vermelho de alerta crítico pisca na tela e a água fica vermelha alertando risco de desabastecimento.
   * **Boia 1 Ativa (25% Reserva)**: Nível sobe para 25% (250 L em tanque padrão de 1.000 L).
   * **Boias 1 e 2 Ativas (50% Médio)**: Nível sobe para 50% (500 L) em azul.
   * **Todas Ativas (100% Cheia)**: Nível sobe para 100% (1.000 L) em ciano.
   * **Erro / Inconsistência**: Diagnóstico inteligente caso uma boia superior ative sem a inferior.
3. Teste alterar a **Capacidade do Reservatório** na aba *Config & Admin* (ex: digite 2.000 ou 500 Litros) e veja os cálculos recalcularem automaticamente!

---

## 2. ⚡ Esquema de Ligação Elétrica (3 Boias)

Com o pull-up interno ativado pelo código (`INPUT_PULLUP`), a fiação é direta e sem resistores adicionais:

```text
                +---------------------+
                |      ESP32 DEVKIT   |
                |                     |
                | [GND] ---------+----+
                |                |
(Boia 1 - 25%)  |                |
  Fio 1 --------+----------------+
  Fio 2 -------------------------[GPIO 17]
                                 |
(Boia 2 - 50%)                   |
  Fio 1 --------+----------------+
  Fio 2 -------------------------[GPIO 18]
                                 |
(Boia 3 - 100%)                  |
  Fio 1 --------+----------------+
  Fio 2 -------------------------[GPIO 19]
                +---------------------+
```

### 📌 Pinos de Ligação:
* **Boia 1 (Reserva / 25%)**:
  * Fio 1 -> **GND**
  * Fio 2 -> **GPIO 17**
* **Boia 2 (Meio / 50%)**:
  * Fio 1 -> **GND**
  * Fio 2 -> **GPIO 18**
* **Boia 3 (Topo / 100%)**:
  * Fio 1 -> **GND**
  * Fio 2 -> **GPIO 19**

> [!NOTE]
> **Orientação das Boias**: Instale com a **setinha para CIMA**. Ao subir com a água, o contato fecha no terra (GND), acionando o patamar.

---

## 3. 📲 Configurando a Capacidade do seu Tanque

1. Acesse o painel pelo IP da sua rede ou por `192.168.4.1`.
2. Vá até a aba **Config & Admin**.
3. No campo **Capacidade Total do Reservatório (em Litros)**, insira o volume real da sua caixa d'água (ex: 500, 1000, 1500, 2000, 5000).
4. Clique em **Salvar Capacidade em Litros**.
5. O valor fica gravado permanentemente na memória Flash do microcontrolador e persistirá mesmo se faltar energia!
