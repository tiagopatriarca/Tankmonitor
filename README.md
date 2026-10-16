# 🌊 TankMonitor - Monitor de Nível de Caixa d'Água com ESP32

[![ESP32](https://img.shields.io/badge/Hardware-ESP32-blue.svg?logo=espressif)](https://www.espressif.com/)
[![C++](https://img.shields.io/badge/Language-C++-00599C.svg?logo=c%2B%2B)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Status](https://img.shields.io/badge/Status-Concluído%20%26%20Testado-brightgreen.svg)]()

Sistema IoT de monitoramento residencial para caixa d'água desenvolvido com **ESP32** e **sensores magnéticos de nível (boias reed switch)**. O sistema monitora 3 patamares de água (**25%**, **50%** e **100%**), possui servidor web embarcado com animação fluida em tempo real, portal de configuração Wi-Fi automático (Captive Portal) e área administrativa com autenticação e troca de senha.

---

## 📸 Interface do Sistema Web

![Dashboard do TankMonitor](assets/dashboard_ui.jpg)

---

## ✨ Principais Recursos

- 💧 **Visualização em Tempo Real**: Painel com ilustração cilíndrica translúcida (estilo *glassmorphism*) e animação de onda fluida SVG/CSS que reage instantaneamente aos sensores sem precisar recarregar a página.
- 📶 **Configuração Wi-Fi Automática (Captive Portal)**: No primeiro uso (ou caso o roteador mude), o ESP32 cria sua própria rede Wi-Fi aberta chamada `CaixaDagua-Setup`. Ao conectar com o celular ou PC, a página de configuração abre automaticamente em `192.168.4.1`.
- 🔍 **Varredura Ativa de Redes**: O ESP32 escaneia as redes Wi-Fi 2.4 GHz do local e permite que você selecione sua rede em um menu suspenso e digite a senha.
- 🔐 **Painel Administrativo & Segurança**: Usuário padrão `admin` com senha inicial `admin`, permitindo alteração segura da senha diretamente pelo navegador. As credenciais ficam salvas na memória Flash não-volátil (NVS/Preferences).
- 🔄 **Botão de Reset de Fábrica**: Segurando o botão físico `BOOT` na placa ESP32 por 5 segundos, o dispositivo apaga os dados de rede e volta imediatamente para o modo `CaixaDagua-Setup`.
- ⚡ **Zero Componentes Externos**: O código utiliza o resistor interno do ESP32 (`INPUT_PULLUP`), dispensando transistores ou resistores adicionais na placa.
- 📦 **Zero Dependências Externas**: Utiliza apenas bibliotecas nativas da Espressif (`WiFi.h`, `WebServer.h`, `DNSServer.h`, `Preferences.h`), compilando de primeira sem conflitos de versão.

---

## 🛠️ Materiais Necessários

![Sensores de Nível Tipo Boia Horizontal](assets/sensor_boia.png)

| Item | Qtd | Descrição |
| :--- | :---: | :--- |
| **Módulo ESP32** | 1x | ESP32 DevKit V1 (NodeMCU 30 ou 38 pinos, chip WROOM-32). |
| **Sensores de Nível (Boias)** | 2x | Sensor de nível magnético horizontal articulado (Reed Switch em polipropileno/PP com anel de vedação de silicone). |
| **Fonte de Alimentação** | 1x | Carregador de celular comum 5V (1A ou 2A) com cabo USB. |
| **Cabo de Rede / Fiação** | ~5-15m | Cabo UTP Cat5 ou cabo paralelo fino para levar o sinal da caixa d'água até onde o ESP32 ficar abrigado. |
| **Caixa de Proteção** | 1x | Caixa plástica de passagem (tipo Tigre/Amanco ou padrão IP65) para proteger o ESP32 de umidade e chuva. |

> ### 💡 Por que boias magnéticas em vez de hastes metálicas comuns?
> Hastes metálicas com corrente contínua (DC) passando pela água sofrem **eletrólise severa**: fios de cobre ou parafusos comuns enferrujam, criam limo e esfarelam em questão de poucas semanas. As **boias magnéticas tipo reed switch** possuem o circuito elétrico 100% selado a vácuo dentro do plástico. O ímã permanente é quem aciona o contato magnético, proporcionando **anos de vida útil sem corrosão**.

---

## ⚡ Esquema de Ligação Elétrica

Como ativamos o resistor interno do ESP32 (`INPUT_PULLUP`), a ligação é direta:

```text
                  +-----------------------------------+
                  |         ESP32 DEVKIT V1           |
                  |                                   |
                  |  [GND] ---------------+-----------+
                  |                       |
 (Boia 1 - 50%)   |                       |
   Fio 1 ---------+                       |
   Fio 2 ---------------------------------[GPIO 18]
                                          |
 (Boia 2 - 100%)                          |
   Fio 1 ---------+-----------------------+
   Fio 2 ---------------------------------[GPIO 19]
                  |                                   |
                  |  [Micro-USB] <--- 5V Alimentação  |
                  +-----------------------------------+
```

### 📌 Mapeamento dos Níveis:
* **Boia 1 (Inferior)**: Instalada na altura de **50%**.
* **Boia 2 (Superior)**: Instalada na altura de **100%** (próximo à borda/ladrão).

| Boia 1 (GPIO 18) | Boia 2 (GPIO 19) | Nível na Tela | Situação Real |
| :---: | :---: | :---: | :--- |
| **Seca (Baixa)** | **Seca (Baixa)** | **25%** | Nível Crítico / Baixo |
| **Submersa (Alta)** | **Seca (Baixa)** | **50%** | Metade da Caixa (500 L) |
| **Submersa (Alta)** | **Submersa (Alta)** | **100%** | Caixa Cheia (1.000 L) |
| *Seca* | *Submersa* | *⚠️ Alerta* | Diagnóstico de falha (sensor travado ou fio invertido) |

> ⚠️ **Dica de Instalação**: Instale as boias com a **setinha gravada no corpo plástico apontando para CIMA**. Assim, quando a água subir e levantar a boia, ela fechará contato com o terra (GND), ativando o nível.

---

## 💻 Passo a Passo para Gravar no ESP32

### Método 1: Usando a Arduino IDE (Mais Comum)

1. **Baixe e instale** a [Arduino IDE](https://www.arduino.cc/en/software) (versão 2.x ou 1.8.x).
2. Abra a IDE e vá em **Arquivo > Preferências** (`Ctrl + ,`).
3. No campo **URLs Adicionais para Gerenciadores de Placas**, adicione o link oficial da Espressif:
   ```text
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Vá em **Ferramentas > Placa > Gerenciador de Placas**, procure por `esp32` (por Espressif Systems) e clique em **Instalar**.
5. Abra o arquivo [esp32-water-tank-monitor.ino](esp32-water-tank-monitor.ino) (o arquivo `web_pages.h` abrirá junto em uma aba ao lado).
6. Conecte o ESP32 ao computador usando um **cabo USB de dados**.
7. Em **Ferramentas > Placa**, selecione **DOIT ESP32 DEVKIT V1** (ou *ESP32 Dev Module*).
8. Em **Ferramentas > Porta**, selecione a porta COM correspondente (ex: `COM7`).
9. Clique no botão de **Carregar (Upload)** e aguarde a finalização!

---

## 📲 Primeiro Acesso e Configuração do Wi-Fi

1. Ao ligar a placa pela primeira vez, o LED onboard começará a piscar.
2. Abra a lista de Wi-Fi do seu celular ou computador e conecte-se à rede:
   ```text
   CaixaDagua-Setup
   ```
3. O navegador abrirá automaticamente. Caso não abra, acerte o endereço:
   ```text
   http://192.168.4.1
   ```
4. Na aba **Config Wi-Fi**, clique em **Escanear Redes Wi-Fi Próximas**.
5. Selecione a rede da sua casa, digite a senha e clique em **Salvar e Conectar**.
6. O ESP32 reiniciará e conectará no seu roteador. O LED ficará aceso fixo.
7. Agora, qualquer dispositivo conectado ao Wi-Fi da sua casa poderá acessar o painel digitando o endereço IP local atribuído ao ESP32!

---

## 🧪 Simulador em Navegador (Testes Rápidos)

O projeto inclui o arquivo [`dashboard_preview.html`](dashboard_preview.html). 
Você pode simplesmente dar um **duplo clique** nele para abri-lo em qualquer navegador (Chrome, Edge, Firefox) e testar todos os estados visuais, a física da água, a troca de senha e a simulação de redes sem precisar da placa conectada.

---

## 📂 Estrutura do Projeto

```text
Tankmonitor/
├── assets/
│   ├── dashboard_ui.jpg       # Imagem conceitual do painel
│   └── sensor_boia.png        # Foto das boias magnéticas
├── esp32-water-tank-monitor.ino # Código principal do firmware
├── web_pages.h                # Interface web moderna em PROGMEM
├── dashboard_preview.html     # Simulador visual para navegador
├── GUIA_MONTAGEM.md           # Guia detalhado de bancada e fiação
├── .gitignore                 # Filtro para binários e ferramentas
└── README.md                  # Este documento
```

---

## 👤 Autor

Desenvolvido por **Tiago Patriarca**  
GitHub: [@tiagopatriarca](https://github.com/tiagopatriarca)

---

## 📄 Licença

Este projeto é distribuído sob a licença **MIT**. Consulte o arquivo [LICENSE](LICENSE) para obter mais detalhes.
