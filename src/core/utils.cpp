#include "utils.h"
#include "core/wifi/wifi_common.h" //to return MAC addr
#include "scrollableTextArea.h"
#include <vector>
#include <globals.h>

/*********************************************************************
**  Function: backToMenu
**  sets the global var to be be used in the options second parameter
**  and returnToMenu will be user do handle the breaks of all loops

when using loopfunctions with an option to "Back to Menu", use:

add this option:
    options.push_back({"Main Menu", [=]() { backToMenu(); }});

while(1) {
    if(returnToMenu) break; // stop this loop and return to the previous loop

    ...
    loopOptions(options);
    ...
}
*/

void backToMenu() { returnToMenu = true; }

void addOptionToMainMenu() {
    returnToMenu = false;
    options.push_back({"Main Menu", backToMenu});
}

/***************************************************************************************
** Function name: getBattery()
** Description:   Returns the battery value from 1-100
***************************************************************************************/
int getBattery() {
#ifdef ANALOG_BAT_PIN
#ifndef ANALOG_BAT_MULTIPLIER
#define ANALOG_BAT_MULTIPLIER 2.0f
#endif
    static bool adcInitialized = false;
    if (!adcInitialized) {
        pinMode(ANALOG_BAT_PIN, INPUT);
        adcInitialized = true;
    }
    uint32_t adcReading = analogReadMilliVolts(ANALOG_BAT_PIN);
    float actualVoltage = (float)adcReading * ANALOG_BAT_MULTIPLIER;
    const float MIN_VOLTAGE = 3300.0f;
    const float MAX_VOLTAGE = 4150.0f;
    float percent = ((actualVoltage - MIN_VOLTAGE) / (MAX_VOLTAGE - (MIN_VOLTAGE + 50.0f))) * 100.0f;

    if (percent < 0) percent = 1;
    if (percent > 100) percent = 100;
    return (int)percent;
#endif
    return 0;
}

void updateClockTimezone() {
    timeClient.begin();
    timeClient.update();

    timeClient.setTimeOffset(bruceConfig.tmz * 3600);

    localTime = timeClient.getEpochTime() + (bruceConfig.dst ? 3600 : 0);

#if defined(HAS_RTC)
    struct tm *timeinfo = localtime(&localTime);
    RTC_TimeTypeDef TimeStruct;
    TimeStruct.Hours = timeinfo->tm_hour;
    TimeStruct.Minutes = timeinfo->tm_min;
    TimeStruct.Seconds = timeinfo->tm_sec;
    _rtc.SetTime(&TimeStruct);
    updateTimeStr(_rtc.getTimeStruct());
#else
    rtc.setTime(localTime);
    updateTimeStr(rtc.getTimeStruct());
    clock_set = true;
#endif
    // Update Internal clock to system time
    struct timeval tv = {.tv_sec = localTime};
    settimeofday(&tv, nullptr);
}

void updateTimeStr(struct tm timeInfo) {
    if (bruceConfig.clock24hr) {
        // Use 24 hour format
        snprintf(
            timeStr, sizeof(timeStr), "%02d:%02d:%02d", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec
        );
    } else {
        // Use 12 hour format with AM/PM
        int hour12 = (timeInfo.tm_hour == 0)   ? 12
                     : (timeInfo.tm_hour > 12) ? timeInfo.tm_hour - 12
                                               : timeInfo.tm_hour;
        const char *ampm = (timeInfo.tm_hour < 12) ? "AM" : "PM";

        snprintf(
            timeStr, sizeof(timeStr), "%02d:%02d:%02d %s", hour12, timeInfo.tm_min, timeInfo.tm_sec, ampm
        );
    }
}

void showDeviceInfo() {
    ScrollableTextArea area = ScrollableTextArea(tr("DEVICE INFO"));

    area.addLine(tr("Caly Version: ") + String(BRUCE_VERSION));
    area.addLine(tr("EEPROM size: ") + String(EEPROMSIZE));
    area.addLine("");
    area.addLine(tr("[MEMORY]"));
    area.addLine(tr("Total heap: ") + formatBytes(ESP.getHeapSize()));
    area.addLine(tr("Free heap: ") + formatBytes(ESP.getFreeHeap()));
    if (psramFound()) {
        area.addLine(tr("Total PSRAM: ") + formatBytes(ESP.getPsramSize()));
        area.addLine(tr("Free PSRAM: ") + formatBytes(ESP.getFreePsram()));
    }
    area.addLine("");
    area.addLine(tr("[NETWORK]"));
    area.addLine(tr("MAC addr: ") + String(WiFi.macAddress()));
    String localIP = WiFi.localIP().toString();
    String softAPIP = WiFi.softAPIP().toString();
    String ipStatus = (WiFi.status() == WL_CONNECTED) ? (localIP != "0.0.0.0"    ? localIP
                                                         : softAPIP != "0.0.0.0" ? softAPIP
                                                                                 : "No valid IP")
                                                      : "Not connected";
    area.addLine(tr("IP address: ") + tr(ipStatus));
    area.addLine("");
    area.addLine(tr("[STORAGE]"));
    area.addLine(tr("LittleFS total: ") + formatBytes(LittleFS.totalBytes()));
    area.addLine(tr("LittleFS used: ") + formatBytes(LittleFS.usedBytes()));
    area.addLine(tr("LittleFS free: ") + formatBytes(LittleFS.totalBytes() - LittleFS.usedBytes()));
    area.addLine("");
    area.addLine(tr("SD Card total: ") + formatBytes(SD.totalBytes()));
    area.addLine(tr("SD Card used: ") + formatBytes(SD.usedBytes()));
    area.addLine(tr("SD Card free: ") + formatBytes(SD.totalBytes() - SD.usedBytes()));
    area.addLine("");

#ifdef HAS_SCREEN
    area.addLine(tr("[SCREEN]"));
    area.addLine(tr("Rotation: ") + String(ROTATION));
    area.addLine(tr("Width: ") + String(tftWidth) + "px");
    area.addLine(tr("Height: ") + String(tftHeight) + "px");
    area.addLine(tr("Brightness: ") + String(bruceConfig.bright) + "%");
    area.addLine("");
#endif

    area.addLine(tr("[GPIO]"));
    area.addLine("GROVE_SDA: " + String(bruceConfigPins.i2c_bus.sda));
    area.addLine("GROVE_SCL: " + String(bruceConfigPins.i2c_bus.scl));
    area.addLine("SERIAL TX: " + String(bruceConfigPins.uart_bus.tx));
    area.addLine("SERIAL RX: " + String(bruceConfigPins.uart_bus.rx));
    area.addLine("SPI_SCK_PIN: " + String(SPI_SCK_PIN));
    area.addLine("SPI_MOSI_PIN: " + String(SPI_MOSI_PIN));
    area.addLine("SPI_MISO_PIN: " + String(SPI_MISO_PIN));
    area.addLine("SPI_SS_PIN: " + String(SPI_SS_PIN));
    area.addLine("IR TX: " + String(TXLED));
    area.addLine("IR RX: " + String(RXLED));
    area.addLine("");

    area.addLine("[BAT]");
    area.addLine(tr("Charge: ") + String(getBattery()) + "%");
#ifdef USE_BQ27220_VIA_I2C
    area.addLine("BQ27220 ADDR: " + String(BQ27220_I2C_ADDRESS));
    area.addLine("Curr Capacity: " + String(bq.getRemainCap()) + "mAh");
    area.addLine("Full Capacity: " + String(bq.getFullChargeCap()) + "mAh");
    area.addLine("Design Capacity: " + String(bq.getDesignCap()) + "mAh");
    area.addLine("Charging: " + String(bq.getIsCharging()));
    area.addLine(
        "Charging Voltage: " + String(((double)bq.getVolt(VOLT_MODE::VOLT_CHARGING) / 1000.0)) + "V"
    );
    area.addLine("Charging Current: " + String(bq.getCurr(CURR_MODE::CURR_CHARGING)) + "mA");
    area.addLine(
        "Time to Empty: " + String((bq.getTimeToEmpty() / 1440)) + " days " +
        String(((bq.getTimeToEmpty() % 1440) / 60)) + " hrs " + String(((bq.getTimeToEmpty() % 1440) % 60)) +
        " mins"
    );
    area.addLine("Avg Power Use: " + String(bq.getAvgPower()) + "mW");
    area.addLine("Voltage: " + String(((double)bq.getVolt(VOLT_MODE::VOLT) / 1000.0)) + "V");
    area.addLine("Raw Voltage: " + String(bq.getVolt(VOLT_MODE::VOLT_RWA)) + "mV");
    area.addLine("Curr Current: " + String(bq.getCurr(CURR_INSTANT)) + "mA");
    area.addLine("Avg Current: " + String(bq.getCurr(CURR_MODE::CURR_AVERAGE)) + "mA");
    area.addLine("Raw Current: " + String(bq.getCurr(CURR_MODE::CURR_RAW)) + "mA");
#endif

    area.show();
}

#if defined(HAS_TOUCH)
/*********************************************************************
** Function: touchHeatMap
** Touchscreen Mapping, include this function after reading the touchPoint
**********************************************************************/
void touchHeatMap(struct TouchPoint t) {
    int third_x = tftWidth / 3;
    int third_y = tftHeight / 3;

    if (t.x > third_x * 0 && t.x < third_x * 1 && t.y > third_y) PrevPress = true;
    if (t.x > third_x * 1 && t.x < third_x * 2 && ((t.y > third_y && t.y < third_y * 2) || t.y > tftHeight))
        SelPress = true;
    if (t.x > third_x * 2 && t.x < third_x * 3) NextPress = true;
    if (t.x > third_x * 0 && t.x < third_x * 1 && t.y < third_y) EscPress = true;
    if (t.x > third_x * 1 && t.x < third_x * 2 && t.y < third_y) UpPress = true;
    if (t.x > third_x * 1 && t.x < third_x * 2 && t.y > third_y * 2 && t.y < third_y * 3) DownPress = true;
    /*
                        Touch area Map
                ________________________________ 0
                |   Esc   |   UP    |         |
                |_________|_________|         |_> third_y
                |         |   Sel   |         |
                |         |_________|  Next   |_> third_y*2
                |  Prev   |  Down   |         |
                |_________|_________|_________|_> third_y*3
                |__Prev___|___Sel___|__Next___| 20 pixel touch area where the touchFooter is drawn
                0         L third_x |         |
                                    Lthird_x*2|
                                              Lthird_x*3
    */
}

#endif

String getOptionsJSON() {
    String menutype = "regular_menu";
    if (menuOptionType == 0) menutype = "main_menu";
    else if (menuOptionType == 1) menutype = "sub_menu";

    String response = "{\"width\":" + String(tftWidth) + ", \"height\":" + String(tftHeight) +
                      ",\"menu\":\"" + menutype + "\",\"menu_title\":\"" + tr(menuOptionLabel) +
                      "\", \"options\":[";
    int i = 0;
    int sel = 0;
    for (auto opt : options) {
        response += "{\"n\":" + String(i) + ",\"label\":\"" + tr(opt.label) + "\"}";
        if (opt.hovered) sel = i;
        i++;
        if (i < options.size()) response += ",";
    }
    response += "], \"active\":" + String(sel) + "}";
    return response;
}

/*********************************************************************
** Function: i2c_bulk_write
** Sends múltiple registers via I2C using a compact table.
   bulk_data example..
   const uint8_t bulk_data[] = {
      2, 0x00, 0x00,       // <- datalen = 2, reg = 0x00, data = 0x00
      3, 0x01, 0x00, 0x02, // <- datalen = 3, reg = 0x01, data = 0x00, 0x02
      0 };                 // <- datalen 0 is end of data.
**********************************************************************/
void i2c_bulk_write(TwoWire *wire, uint8_t addr, const uint8_t *bulk_data) {
    const uint8_t *p = bulk_data;
    while (true) {
        uint8_t datalen = *p++;
        if (datalen == 0) { break; } // --- end of table ---
        uint8_t reg = *p++;
        wire->beginTransmission(addr);
        wire->write(reg);
        for (uint8_t i = 0; i < datalen - 1; i++) { wire->write(*p++); }
        uint8_t error = wire->endTransmission();
        if (error != 0) { log_e("I2C Write error %d", error); }
        delay(1);
    }
}

String formatTimeDecimal(uint32_t totalMillis) {
    uint16_t minutes = totalMillis / 60000;
    float seconds = (totalMillis % 60000) / 1000.0;

    char buffer[10];
    sprintf(buffer, "%02d:%06.3f", minutes, seconds);
    return String(buffer);
}

void printMemoryUsage(const char *msg) {
    Serial.printf(
        "%s:\nPSRAM: [Free: %lu, max alloc: %lu],\nRAM: [Free: %lu, "
        "max alloc: %lu]\n\n",
        msg,
        ESP.getFreePsram(),
        ESP.getMaxAllocPsram(),
        ESP.getFreeHeap(),
        ESP.getMaxAllocHeap()
    );
}

String repeatString(int length, String character) {
    String result = "";
    for (int i = 0; i < length; i++) { result += character; }
    return result;
}

String formatBytes(uint64_t bytes) {
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    float size = bytes;

    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        unitIndex++;
    }

    if (unitIndex == 0) {
        return String(bytes) + " " + units[unitIndex];
    } else {
        return String(size, 2) + " " + units[unitIndex];
    }
}

static String _sanitize(const String &in) {
    String out;
    out.reserve(in.length());
    for (size_t i = 0; i < in.length(); i++) {
        char c = in[i];
        switch ((unsigned char)c) {
        case (char)0xC3: // UTF-8 lead byte for many Latin-1 chars; naive strip
            if (i + 1 < in.length()) {
                unsigned char n = (unsigned char)in[i + 1];
                if (n == 0xA1 || n == 0x81) { out += 'a'; }      // á/Á
                else if (n == 0xA2 || n == 0x82) { out += 'a'; } // â/Â
                else if (n == 0xA3 || n == 0x83) { out += 'a'; } // ã/Ã
                else if (n == 0xA4 || n == 0x84) { out += 'a'; } // ä/Ä
                else if (n == 0xA9 || n == 0x89) { out += 'e'; } // é/É
                else if (n == 0xAA || n == 0x8A) { out += 'e'; } // ê/Ê
                else if (n == 0xA8 || n == 0x88) { out += 'e'; } // è/È
                else if (n == 0xAD || n == 0x8D) { out += 'i'; } // í/Í
                else if (n == 0xB4 || n == 0x94) { out += 'o'; } // ó/Ó
                else if (n == 0xB5 || n == 0x95) { out += 'o'; } // ô/Ô
                else if (n == 0xB5 || n == 0x95) { out += 'o'; } // ô/Ô
                else if (n == 0xB5 || n == 0x95) { out += 'o'; }
                else if (n == 0xB5 || n == 0x95) { out += 'o'; }
                else if (n == 0xB5 || n == 0x95) { out += 'o'; }
                else if (n == 0xB5 || n == 0x95) { out += 'o'; }
                else if (n == 0xB5 || n == 0x95) { out += 'o'; }
                else if (n == 0xB5 || n == 0x95) { out += 'o'; }
                else if (n == 0xB5 || n == 0x95) { out += 'o'; }
                else if (n == 0xB5 || n == 0x95) { out += 'o'; }
                else if (n == 0xB5 || n == 0x95) { out += 'o'; }
                else if (n == 0xB5 || n == 0x95) { out += 'o'; }
                else if (n == 0xBA || n == 0x9A) { out += 'u'; } // ú/Ú
                else if (n == 0xA7 || n == 0x87) { out += 'c'; } // ç/Ç
                else if (n == 0xB1 || n == 0x91) { out += 'n'; } // ñ/Ñ
                else { out += '?'; }
                i++;
            } else out += '?';
            break;
        default:
            if ((unsigned char)c < 32 || (unsigned char)c > 126) out += '?';
            else out += c;
        }
    }
    return out;
}

String tr(const String &s) {
    if (bruceConfig.language == 0) return s;
    struct P { const char *en; const char *pt; };
    static const P map[] = {
        {"Main Menu", "Menu Principal"},
        {"Config", "Config"},
        {"Display & UI", "Tela & UI"},
        {"Brightness", "Brilho"},
        {"Dim Time", "Tempo de Dim"},
        {"Orientation", "Orientacao"},
        {"UI Color", "Cor da UI"},
        {"UI Theme", "Tema da UI"},
        {"LED Config", "LED"},
        {"LED Color", "Cor LED"},
        {"LED Effect", "Efeito LED"},
        {"LED Brightness", "Brilho LED"},
        {"Audio Config", "Audio"},
        {"Sound Volume", "Volume"},
        {"System Config", "Sistema"},
        {"Power", "Energia"},
        {"Dev Mode", "Modo Dev"},
        {"Install App Store", "Instalar Loja"},
        {"Back", "Voltar"},
        {"WiFi", "WiFi"},
        {"JS Interpreter", "JS Interp"},
        {"RF", "RF"},
        {"RFID", "RFID"},
        {"Others", "Outros"},
        {"others", "Outros"},
        {"NRF24", "NRF24"},
        {"LoRa", "LoRa"},
        {"IR", "IR"},
        {"GPS", "GPS"},
        {"Files", "Arquivos"},
        {"FM", "FM"},
        {"Ethernet", "Ethernet"},
        {"Connect", "Conectar"},
        {"connect", "Conectar"},
        {"Clock", "Relogio"},
        {"clock", "Relogio"},
        {"BLE", "BLE"},
        {"Hide/Show Apps", "Ocultar/Mostrar Apps"},
        {"Startup App", "App Inicial"},
        {"Advanced", "Avancado"},
        {"Power Menu", "Energia"},
        {"Power Save:", "Economia:"},
        {"Power Save", "Economia"},
        {"Deep Sleep", "Sono Profundo"},
        {"Sleep", "Sono"},
        {"Restart", "Reiniciar"},
        {"Power Off", "Desligar"},
        {"Send File", "Enviar Arquivo"},
        {"Recv File", "Receber Arquivo"},
        {"Send Cmds", "Enviar Cmds"},
        {"Recv Cmds", "Receber Cmds"},
        {"WiFi AP Creds", "Creds WiFi AP"},
        {"Network Creds", "Creds Rede"},
        {"Keyboard Layout", "Layout Teclado"},
        {"Key Delay", "Delay Teclas"},
        {"Show Output", "Mostrar Saida"},
        {"Toggle BLE API", "Alternar BLE API"},
        {"BadUSB/BLE", "BadUSB/BLE"},
        {"Factory Reset", "Resetar Fabrica"},
        {"I2C Finder", "I2C Finder"},
        {"CC1101 Pins", "Pins CC1101"},
        {"NRF24  Pins", "Pins NRF24"},
        {"LoRa Pins", "Pins LoRa"},
        {"W5500 Pins", "Pins W5500"},
        {"SDCard Pins", "Pins SDCard"},
        {"I2C Pins", "Pins I2C"},
        {"UART Pins", "Pins UART"},
        {"GPS Pins", "Pins GPS"},
        {"Serial USB", "Serial USB"},
        {"Serial UART", "Serial UART"},
        {"Disable DevMode", "Desativar DevMode"},
        {"Dev Mode Enabled", "Modo Dev Ativado"},
        {"PREV", "ANT"},
        {"SEL", "SEL"},
        {"NEXT", "PROX"},
        {"Exit", "Sair"},
        {"UP", "Cima"},
        {"DOWN", "Baixo"},
        {"Language", "Idioma"},
        {"English", "Ingles"},
        {"Português (BR)", "Portugues (BR)"},
        {"About", "Sobre"},
        {"DEVICE INFO", "INFO DO DISPOSITIVO"},
        {"Caly Version:", "Caly Versao:"},
        {"EEPROM size:", "EEPROM tamanho:"},
        {"[MEMORY]", "[MEMORIA]"},
        {"Total heap:", "Heap total:"},
        {"Free heap:", "Heap livre:"},
        {"Total PSRAM:", "PSRAM total:"},
        {"Free PSRAM:", "PSRAM livre:"},
        {"[NETWORK]", "[REDE]"},
        {"MAC addr:", "MAC:"},
        {"IP address:", "IP:"},
        {"[STORAGE]", "[ARMAZENAMENTO]"},
        {"LittleFS total:", "LittleFS total:"},
        {"LittleFS used:", "LittleFS usado:"},
        {"LittleFS free:", "LittleFS livre:"},
        {"SD Card total:", "SD total:"},
        {"SD Card used:", "SD usado:"},
        {"SD Card free:", "SD livre:"},
        {"[SCREEN]", "[TELA]"},
        {"Rotation:", "Rotacao:"},
        {"Width:", "Largura:"},
        {"Height:", "Altura:"},
        {"Brightness:", "Brilho:"},
        {"[GPIO]", "[GPIO]"},
        {"Charge:", "Carga:"},
        {"Power Save ON", "Economia ON"},
        {"Power Save OFF", "Economia OFF"},
        {"Power Save ON - Desative para usar Wi-Fi", "Economia ON - Desative para usar Wi-Fi"},
        {"Are you sure you want\nto Factory Reset?\nAll data will be lost!", "Tem certeza que deseja\nResetar Fabrica?\nTodos os dados serao perdidos!"},
        {"No", "Nao"},
        {"Yes", "Sim"},
        {"Paused", "Pausado"},
        {"Running, Wait", "Executando, Aguarde"},
        {"All codes sent!", "Todos os codigos enviados!"},
        {"User Stopped", "Usuario Parou"},
        {"Chat", "Chat"},
        {"Change username", "Trocar usuario"},
        {"Change Frequency", "Trocar frequencia"},
        {"W5500 not found", "W5500 nao encontrado"},
        {"Power Off Device?", "Desligar dispositivo?"},
        {"WiFi Connect", "Conectar ao WiFi"},
        {"Connect to wifi", "Conectar ao WiFi"},
        {"Connect to Wifi", "Conectar ao WiFi"},
        {"Connect to WiFi", "Conectar ao WiFi"},
        {"Wifi Offline", "WiFi Offline"},
        {"Scanning..", "Escaneando.."},
        {"Scanning Networks..", "Escaneando Redes.."},
        {"Turn off WiFi", "Desligar WiFi"},
        {"WiFi Password Recover", "Recuperar Senha WiFi"},
        {"WiFi Password Cracker", "Quebrar Senha WiFi"},
        {"WiFi Cracker", "Quebrar WiFi"},
        {"WiFi: Beacon SPAM", "WiFi: SPAM de Beacon"},
        {"<Hidden SSID>", "<SSID Oculto>"},
        {"Start WiFi AP", "Iniciar WiFi AP"},
        {"AP info", "Info AP"},
        {"Wifi Atks", "Ataques WiFi"},
        {"Evil Portal", "Portal Maligno"},
        {"Listen TCP", "Escutar TCP"},
        {"Client TCP", "Cliente TCP"},
        {"TelNET", "TelNET"},
        {"SSH", "SSH"},
        {"Sniffers", "Sniffers"},
        {"Scan Hosts", "Escanear Hosts"},
        {"Wireguard", "Wireguard"},
        {"Responder", "Responder"},
        {"Brucegotchi", "Brucegotchi"},
        {"WiFi Pass Recovery", "Recuperar Senha WiFi"},
        {"Change MAC", "Trocar MAC"},
        {"Add Evil Wifi", "Adicionar WiFi Maligno"},
        {"Remove Evil Wifi", "Remover WiFi Maligno"},
        {"Evil Wifi Settings", "Config WiFi Maligno"},
        {"Password Mode", "Modo Senha"},
        {"Rename /creds", "Renomear /creds"},
        {"Allow /creds access", "Permitir acesso /creds"},
        {"Rename /ssid", "Renomear /ssid"},
        {"Allow /ssid access", "Permitir acesso /ssid"},
        {"Display endpoints", "Mostrar endpoints"},
        {"Hidden Networks:", "Redes Ocultas:"},
        {"WiFi Config", "Config WiFi"},
        {"WiFi", "WiFi"},
        {"Starting a Wifi function will probably make the WebUI stop working", "Iniciar funcao WiFi pode parar o WebUI"},
        {"Sel: to continue", "SEL: continuar"},
        {"Any key: to Menu", "Qualquer tecla: menu"},
        {"Deauth Flood", "Inundacao Deauth"},
        {"Handshake Capture", "Captura Handshake"},
        {"Target Deauth", "Deauth Alvo"},
        {"pcap sniffer", "Sniffer pcap"},
        {"PROBE SNIFFER", "SNIFFER PROBE"},
        {"EVIL PORTAL", "PORTAL MALIGNO"},
        {"Station Deauth", "Deauth Estacao"},
        {"TAG-O-MATIC", "TAG-O-MATIC"},
        {"SRIX TOOL", "Ferramenta SRIX"},
        {"PN532 BLE", "PN532 BLE"},
        {"Read EMV Card", "Ler Cartao EMV"},
        {"CHAMELEON", "CHAMELEON"},
        {"AMIIBOLINK", "AMIIBOLINK"},
        {"RF Jammer", "Jammer RF"},
        {"Set a timer", "Definir timer"},
        {"Timer finished!", "Timer finalizado!"},
        {"iButton", "iButton"},
        {"iButton Write", "Escrever iButton"},
        {"iButton ID", "ID iButton"},
        {"Wigle Upload", "Upload Wigle"},
        {"Wardriving", "Wardriving"},
        {"GPS Tracker", "Rastreamento GPS"},
        {"MAC Flooding", "Inundacao MAC"},
        {"DHCP Starvation", "Exaustao DHCP"},
        {"ARP Poisoning", "Envenenamento ARP"},
        {"ARP Spoofing", "Spoofing ARP"},
        {"iBeacon", "iBeacon"},
        {"Spam All Apple", "Spam Apple"},
        {"FILE INFO", "INFO ARQUIVO"},
        {"Mass Storage", "Armaz. Massa"},
        {"RECEIVE COMMANDS", "RECEBER COMANDOS"},
        {"SEND FILE", "ENVIAR ARQUIVO"},
        {"RECEIVE FILE", "RECEBER ARQUIVO"},
        {"No wg.conf file", "Sem arquivo wg.conf"},
        {"Cannot copy Folder", "Nao foi possivel copiar Pasta"},
        {"Couldn't create folder", "Nao foi possivel criar pasta"},
        {"Typing", "Digitando"},
    };
    for (auto &p : map) {
        if (s == p.en) return _sanitize(String(p.pt));
    }
    String x = s;
    x.replace(": ON", ": LIGADO");
    x.replace(": OFF", ": DESLIGADO");
    x.replace(" ON", " LIGADO");
    x.replace(" OFF", " DESLIGADO");
    x.replace("Power Save", "Economia");
    x.replace("Enabled", "Ativado");
    x.replace("Disabled", "Desativado");
    return _sanitize(x);
}
String tr(const char *s) { return tr(String(s)); }
