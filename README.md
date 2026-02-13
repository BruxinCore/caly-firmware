<div align="center">

<img src="./docs/banner.png" alt="CalyFirmware Banner" width="100%">

# 🟣 CalyFirmware 🟣
**Ultimate M5StickC Plus2 Experience**

[![License: AGPL v3](https://img.shields.io/badge/License-AGPL_v3-blue.svg)](https://www.gnu.org/licenses/agpl-3.0)
[![Platform](https://img.shields.io/badge/Platform-M5StickC%20Plus2-orange)](https://m5stack.com)
[![Based on](https://img.shields.io/badge/Forked%20from-Bruce%20Firmware-green)](https://github.com/BruceDevices/firmware)

*Performance Otimizada • Protocolos Corrigidos • UX Brasileira*

</div>

---

## 📖 Sobre o Projeto

O **CalyFirmware** é um fork avançado do Bruce Firmware, focado em **precisão técnica** e **estabilidade**. O projeto reescreve componentes críticos como a stack de Infravermelho e o gerenciamento de Bluetooth para garantir que as ferramentas funcionem em hardware real com máxima eficiência.

> *"O anfitrião te aguarda."*

---

## ⚡ Gerenciamento de Energia & Performance

O firmware possui um gerenciamento inteligente de bateria e CPU, alternando dinamicamente entre modos de ataque e preservação.

| Modo | Configuração | Comportamento |
| :--- | :--- | :--- |
| **Economia** | `CPU 80MHz` <br> `Wi-Fi/BLE OFF` | **Modo Furtivo.** Reduz o brilho para 1/3, desliga rádios e força *dimmer* agressivo (apaga tela em +5s). Bloqueia tentativas de scan para evitar erros de leitura. Restaura conexão automaticamente ao sair. |
| **Agressivo** | `Int 80` / `Win 79` | **Scan Contínuo & Max Power.** Prioridade total para descoberta e alcance. Desativa o *Power Save* no boot e força o hardware ao limite. |
| **Balanceado** | `Int 90` / `Win 89` | **Uso Geral.** O ponto ideal entre consumo de bateria e eficácia de descoberta. |

### 🚀 Boost de Alcance (Modo Agressivo)
Quando o perfil Agressivo é ativado, o firmware aplica *patches* em tempo real no driver de rádio:
* **Wi-Fi Turbo:** Força a potência de transmissão (TX) para **20dBm** (máximo suportado), garantindo maior taxa de sucesso em ataques *Deauth* e *Spam* mesmo com obstáculos.
* **BLE Max Output:** Inicializa Scans e Advertising (Beacon Studio) com potência máxima, aumentando drasticamente a visibilidade dos spams Bluetooth.

---

## 🔴 Infravermelho (IR) 2.0
A maior reformulação do firmware. Correção de *bit-order* e protocolos quebrados na versão original.

* **Protocolos Suportados & Corrigidos:**
    * ✅ **NEC / NECext:** Correção crítica de inversão de bytes e envio MSB.
    * ✅ **Sony SIRC:** Montagem correta por *bit-size* (12/15/20) e reversão LSB.
    * ✅ **Kaseikyo/Panasonic:** Cálculo real de paridade, Vendor ID e Checksum.
    * ✅ **Outros:** RC5, RC6, Samsung32.
* **Clonagem Flipper Zero:** A leitura RAW gera arquivos `.ir` compatíveis com o Flipper, salvando protocolo, endereço, comando e dados brutos.
* **RAW Robusto:** Parsing dinâmico com frequência padrão de 38kHz e *duty cycle* 0.33.

---

## 📺 TV-B-Gone & Jammer
Ferramentas de ataque otimizadas para não travar o dispositivo (RTOS Friendly).

* **TV-B-Gone Thread-Safe:** Implementação de *Mutex FreeRTOS* para evitar conflitos no TX. Reordenação inteligente (Prioriza códigos 38kHz e regiões NA/EU).
* **Jammer Avançado:**
    * **Modos:** Basic, Enhanced, Sweep, Random.
    * **Dupla Estratégia:** Alterna entre *Toggle Direto* e *SendRaw* para máxima eficácia.
    * **Densidade Controlável:** Ajuste fino da taxa de envio.
    * **UI Leve:** *Throttling* na atualização da tela para manter a CPU dedicada ao sinal.

---

## 🛡️ Guardas de Sistema & Segurança
Melhorias invisíveis que impedem o dispositivo de travar ou queimar componentes.

* **Proteção de Hardware:** Encerra o SPI do Cartão SD antes de iniciar operações pesadas de IR (evita conflito de pinos).
* **Gestão de 5V (Boost):** O regulador de voltagem (necessário para o IR alcançar longe) só é ativado durante o uso e desligado ao sair.
* **Watchdogs:** Timeout de 10s em leituras headless e avisos de *buffer overflow*.

---

## 🎨 Interface & Usabilidade

Refinamentos visuais para uma experiência mais limpa e informativa.

* **Status Bar Dinâmica:** Novos ícones indicam o perfil de energia atual num relance:
    * 🍃 **Economia** (Folha)
    * ⚖️ **Balanceado** (Balança)
    * ⚡ **Agressivo** (Raio)
* **Visual Minimalista:** O relógio da barra de status agora exibe apenas `HH:MM`, removendo os segundos para reduzir a poluição visual.
* **Idioma:** Tradução completa para **PT-BR**.
* **WebUI Renovada:**
    * Acesso via: `http://caly.local`
    * Senha padrão: `caly` (Usuário: `admin`)

---

## 🛠️ Instalação

Acesse nosso Instalador Web Oficial e conecte seu M5StickC Plus2 via USB. Não é necessário baixar arquivos manualmente.

<div align="center">

[![Web Flasher](https://img.shields.io/badge/🚀_INSTALAR_AGORA-WEB_FLASHER-7e22ce?style=for-the-badge&logo=google-chrome&logoColor=white)](https://bruxincore.github.io/caly-firmware/)

*(Compatível com Google Chrome e Microsoft Edge)*

</div>

---

## ⚖️ Créditos e Licença

Este projeto é um fork orgulhoso do **Bruce Firmware**. Todo o crédito pela arquitetura base e drivers vai para os criadores originais.

* **Base Project:** [BruceDevices/firmware](https://github.com/BruceDevices/firmware)
* **Licença:** GNU Affero General Public License v3.0 (AGPL-3.0).

> **Aviso Legal:** O CalyFirmware é desenvolvido para fins educacionais e de pesquisa de segurança. O autor não se responsabiliza pelo uso indevido das ferramentas aqui fornecidas. Use com responsabilidade.
