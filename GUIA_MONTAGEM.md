# 🛠️ Guia de Montagem e Gravação: Sensor de Caixa d'Água com ESP32

---

## 1. 🧪 Teste Imediato da Interface no seu Computador

Antes mesmo de passar o código para a placa, você pode testar o visual e a dinâmica da interface web agora mesmo:
1. Abra o arquivo [dashboard_preview.html](file:///C:/Users/DATEN/.gemini/antigravity-ide/scratch/esp32-water-tank-monitor/dashboard_preview.html) no seu navegador (Edge, Chrome, etc.).
2. No rodapé da tela, use o **"Simulador de Bancada"** para clicar em:
   * **Ambas Secas (25%)**: A água desce e fica laranja indicando nível crítico.
   * **Boia Meio Ativa (50%)**: A água sobe para 500 Litros em azul.
   * **Ambas Ativas (100%)**: A caixa enche e brilha em ciano.
   * **Erro / Travada**: Mostra o diagnóstico inteligente caso a boia de cima ative sem a de baixo.
3. Teste a aba **"Config Wi-Fi"** (escanear redes) e a aba **"Admin"** (troca de senha do admin).

---

## 2. ⚡ Esquema de Ligação Elétrica (Fiação Simples)

Graças ao resistor de **Pull-Up interno** do ESP32, **não é necessário nenhum resistor ou componente externo**.

```
                +---------------------+
                |      ESP32 DEVKIT   |
                |                     |
                | [GND] ---------+----+
                |                |
(Boia 1 - 50%)  |                |
  Fio 1 --------+----------------+
  Fio 2 -------------------------[GPIO 18]
                                 |
(Boia 2 - 100%)                  |
  Fio 1 --------+----------------+
  Fio 2 -------------------------[GPIO 19]
                +---------------------+
```

### 📌 Resumo dos Pinos:
* **Boia 1 (Meio / 50%)**:
  * Fio 1 -> **GND** do ESP32
  * Fio 2 -> **GPIO 18** do ESP32
* **Boia 2 (Topo / 100%)**:
  * Fio 1 -> **GND** do ESP32
  * Fio 2 -> **GPIO 19** do ESP32
* **Alimentação**: Cabo USB conectado a um carregador de celular de 5V (1A ou 2A).

> [!NOTE]
> **Orientação da Boia**: Instale as boias com a **setinha gravada no corpo plástico apontando para CIMA**. Assim, quando a água subir e levantar a haste, o contato interno fechará com o terra (GND), acionando o nível.

---

## 3. 💻 Como Gravar o Código no ESP32 (Arduino IDE)

### Passo 1: Instalar o Suporte ao ESP32 (se ainda não tiver)
1. Abra o **Arduino IDE** (versão 1.8.x ou 2.x).
2. Vá em **Arquivo -> Preferências** (`Ctrl + ,`).
3. No campo **URLs Adicionais para Gerenciadores de Placas**, cole a URL oficial da Espressif:
   ```text
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Vá em **Ferramentas -> Placa -> Gerenciador de Placas**, pesquise por `esp32` (por Espressif Systems) e clique em **Instalar**.

### Passo 2: Abrir o Projeto
1. Abra o arquivo principal do projeto:
   [esp32-water-tank-monitor.ino](file:///C:/Users/DATEN/.gemini/antigravity-ide/scratch/esp32-water-tank-monitor/esp32-water-tank-monitor.ino).
2. O arquivo auxiliar [web_pages.h](file:///C:/Users/DATEN/.gemini/antigravity-ide/scratch/esp32-water-tank-monitor/web_pages.h) deve estar na mesma pasta (ele abrirá automaticamente em uma aba ao lado).

### Passo 3: Configurar a Placa e Porta
1. Em **Ferramentas -> Placa -> ESP32 Arduino**, selecione:
   * **DOIT ESP32 DEVKIT V1** (ou *ESP32 Dev Module*).
2. Em **Ferramentas -> Porta**, selecione a porta COM do seu ESP32 (ex: `COM7`, `COM8`, etc.).
3. Clique na seta de **Upload / Carregar** (`Ctrl + U`).

*(Se durante a gravação aparecer `Connecting........_____`, dê um toque rápido no botão físico `BOOT` do ESP32 para liberar a gravação).*

---

## 4. 📲 Primeiro Acesso e Configuração do Wi-Fi

1. **Ligue o ESP32**:
   * Como é a primeira vez, ele não tem rede salva e o LED onboard começará a piscar.
   * Ele criará uma rede Wi-Fi aberta com o nome: `CaixaDagua-Setup`.
2. **Conecte pelo Celular ou Notebook**:
   * Procure pelas redes Wi-Fi e conecte-se na rede `CaixaDagua-Setup`.
   * Na maioria dos celulares, abrirá imediatamente a notificação *"Fazer login na rede Wi-Fi"* (Captive Portal). Se não abrir, abra o navegador e acesse:
     ```
     http://192.168.4.1
     ```
3. **Selecione o Wi-Fi da sua Casa**:
   * Vá na aba **Config Wi-Fi** e clique em **Escanear Redes**.
   * Escolha o Wi-Fi da sua casa, digite a senha e clique em **Salvar e Conectar**.
   * O ESP32 salvará os dados na memória permanente (NVS) e reiniciará.
4. **Pronto! Acesso na Rede Local**:
   * Conecte seu celular de volta ao Wi-Fi da sua casa.
   * Abra o navegador e digite o IP atribuído ao ESP32 pelo seu roteador (você pode ver pelo aplicativo do roteador ou pelo Monitor Serial da Arduino IDE).
   * O painel agora atualizará os níveis da caixa em tempo real a cada 2 segundos!

---

## 5. 🔒 Segurança & Recuperação de Senha

* **Usuário Padrão**: `admin`
* **Senha Inicial Padrão**: `admin`
* Para trocar a senha, vá até a aba **Admin** do painel e insira sua nova senha.
* **Trocou de roteador ou esqueceu a senha?**
  * Pressione e segure o botão físico **`BOOT`** na placa ESP32 por **5 segundos**.
  * O LED piscará rapidamente e o ESP32 apagará as credenciais de rede, voltando a criar a rede `CaixaDagua-Setup` para nova configuração!
