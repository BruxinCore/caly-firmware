#include "powerSave.h"
#include "display.h"
#include "settings.h"
#include <NimBLEDevice.h>
#include "wifi/wifi_common.h"
#include <globals.h>

/* Check if it's time to put the device to sleep */
#define SCREEN_OFF_DELAY 5000

static wifi_mode_t ps_prev_wifi_mode = WIFI_MODE_NULL;
static bool ps_prev_wifi_connected = false;

void fadeOutScreen(int startValue) {
    for (int brightValue = startValue; brightValue >= 0; brightValue -= 1) {
        setBrightness(max(brightValue, 0), false);
        delay(5);
    }
    turnOffDisplay();
}

void checkPowerSaveTime() {
    if (bruceConfig.dimmerSet == 0) return;

    unsigned long elapsed = millis() - previousMillis;
    int startDimmerBright = bruceConfig.bright / 3;
    int dimmerSetMs = bruceConfig.dimmerSet * 1000;

    if (elapsed >= dimmerSetMs && !dimmer && !isSleeping) {
        dimmer = true;
        setBrightness(startDimmerBright, false);
    } else if (elapsed >= (dimmerSetMs + SCREEN_OFF_DELAY) && !isScreenOff && !isSleeping) {
        isScreenOff = true;
        fadeOutScreen(startDimmerBright);
    }
}

void sleepModeOn() {
    isSleeping = true;
    setCpuFrequencyMhz(80);

    int startDimmerBright = bruceConfig.bright / 3;

    fadeOutScreen(startDimmerBright);

    panelSleep(true); //  power down screen

    disableCore0WDT();
#if SOC_CPU_CORES_NUM > 1
    disableCore1WDT();
#endif
    disableLoopWDT();
    delay(200);
}

void sleepModeOff() {
    isSleeping = false;
    setCpuFrequencyMhz(CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ);

    panelSleep(false); // wake the screen back up

    if (bruceConfig.powerSaveEnabled) {
        uint8_t base = (uint8_t)bruceConfig.bright;
        uint8_t target = base > 15 ? (uint8_t)max(3, (int)(base / 3)) : base;
        setBrightness(target, false);
    } else {
        getBrightness();
    }
    enableCore0WDT();
#if SOC_CPU_CORES_NUM > 1
    enableCore1WDT();
#endif
    enableLoopWDT();
    feedLoopWDT();
    delay(200);
}

void powerSaveOn() {
    bruceConfig.setPowerSaveEnabled(1);
    setCpuFrequencyMhz(80);
    uint8_t current = (uint8_t)bruceConfig.bright;
    uint8_t target = current > 15 ? (uint8_t)max<uint8_t>(3, current / 3) : current;
    setBrightness(target, false);

    ps_prev_wifi_mode = WiFi.getMode();
    ps_prev_wifi_connected = (WiFi.status() == WL_CONNECTED) || (WiFi.softAPSSID().length() > 0);
    if (ps_prev_wifi_mode != WIFI_MODE_NULL) wifiDisconnect();

    if (BLEConnected) {
#if defined(CONFIG_IDF_TARGET_ESP32C5)
        esp_bt_controller_deinit();
#else
        BLEDevice::deinit();
#endif
        BLEConnected = false;
    }

    // BLE desativado para economizar energia
}

void powerSaveOff() {
    bruceConfig.setPowerSaveEnabled(0);
    setCpuFrequencyMhz(CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ);
    setBrightness((uint8_t)bruceConfig.bright, false);

    if (ps_prev_wifi_mode != WIFI_MODE_NULL) {
        WiFi.mode(ps_prev_wifi_mode);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        switch (ps_prev_wifi_mode) {
            case WIFI_AP:
                _setupAP();
                break;
            case WIFI_MODE_STA:
                if (ps_prev_wifi_connected) wifiConnecttoKnownNet();
                break;
            case WIFI_MODE_APSTA:
                _setupAP();
                if (ps_prev_wifi_connected) wifiConnecttoKnownNet();
                break;
            default:
                break;
        }
    }
}

void prepareForDeepSleep() {
    if (WiFi.getMode() != WIFI_MODE_NULL) wifiDisconnect();
    if (BLEConnected) {
#if defined(CONFIG_IDF_TARGET_ESP32C5)
        esp_bt_controller_deinit();
#else
        BLEDevice::deinit();
#endif
        BLEConnected = false;
    }
}
