
extern "C" {
  void app_loop();
  void eraseMcuConfig();
  void restartMCU();
}

#include "Settings.h"
#include <BlynkSimpleEsp32_SSL.h>

#ifndef BLYNK_NEW_LIBRARY
#error "Old version of Blynk library is in use. Please replace it with the new one."
#endif

#if !defined(BLYNK_TEMPLATE_ID) || !defined(BLYNK_DEVICE_NAME)
#error "Please specify your BLYNK_TEMPLATE_ID and BLYNK_DEVICE_NAME"
#endif

#include "BlynkState.h"
#include "ConfigStore.h"
#include "ResetButton.h"
#include "ConfigMode.h"
#include "Indicator.h"
#include "OTA.h"
#include "Console.h"
void manual_control();
inline
void BlynkState::set(State m) {
  if (state != m && m < MODE_MAX_VALUE) {
    DEBUG_PRINT(String(StateStr[state]) + " => " + StateStr[m]);
    state = m;

    // You can put your state handling here,
    // i.e. implement custom indication
  }
}

void printDeviceBanner()
{
  Blynk.printBanner();
  DEBUG_PRINT("--------------------------");
  DEBUG_PRINT(String("Product:  ") + BLYNK_DEVICE_NAME);
  DEBUG_PRINT(String("Hardware: ") + BOARD_HARDWARE_VERSION);
  DEBUG_PRINT(String("Firmware: ") + BLYNK_FIRMWARE_VERSION " (build " __DATE__ " " __TIME__ ")");
  if (configStore.getFlag(CONFIG_FLAG_VALID)) {
    DEBUG_PRINT(String("Token:    ...") + (configStore.cloudToken+28));
  }
  DEBUG_PRINT(String("Device:   ") + BLYNK_INFO_DEVICE + " @ " + ESP.getCpuFreqMHz() + "MHz");
  DEBUG_PRINT(String("MAC:      ") + WiFi.macAddress());
  DEBUG_PRINT(String("Flash:    ") + ESP.getFlashChipSize() / 1024 + "K");
  DEBUG_PRINT(String("ESP sdk:  ") + ESP.getSdkVersion());
  DEBUG_PRINT(String("Chip rev: ") + ESP.getChipRevision());
  DEBUG_PRINT(String("Free mem: ") + ESP.getFreeHeap());
  DEBUG_PRINT("--------------------------");
}

void runBlynkWithChecks() {
  Blynk.run();
  if (BlynkState::get() == MODE_RUNNING) {
    if (!Blynk.connected()) {
      if (WiFi.status() == WL_CONNECTED) {
        BlynkState::set(MODE_CONNECTING_CLOUD);
      } else {
        BlynkState::set(MODE_CONNECTING_NET);
      }
    }
  }
}

class Edgent {

public:
  void begin()
  {
    indicator_init();
    button_init();
    config_init();

    WiFi.persistent(false);
    WiFi.enableSTA(true);   // Needed to get MAC

    printDeviceBanner();

    if (configStore.getFlag(CONFIG_FLAG_VALID)) {
      BlynkState::set(MODE_CONNECTING_NET);
    } else if (config_load_blnkopt()) {
      DEBUG_PRINT("Firmware is preprovisioned");
      BlynkState::set(MODE_CONNECTING_NET);
    } else {
      BlynkState::set(MODE_WAIT_CONFIG);
    }
  }

  void run() {
    app_loop();
    switch (BlynkState::get()) {
    case MODE_WAIT_CONFIG:       
    case MODE_CONFIGURING:       enterConfigMode();    break;
    case MODE_CONNECTING_NET:    enterConnectNet();    break;
    case MODE_CONNECTING_CLOUD:  enterConnectCloud();  break;
    case MODE_RUNNING:           runBlynkWithChecks(); break;
    case MODE_OTA_UPGRADE:       enterOTA();           break;
    case MODE_SWITCH_TO_STA:     enterSwitchToSTA();   break;
    case MODE_RESET_CONFIG:      enterResetConfig();   break;
    default:                     enterError();         break;
    }
  }
};

Edgent BlynkEdgent;
BlynkTimer edgentTimer;

void app_loop() {
    edgentTimer.run();
    edgentConsole.run();
    manual_control();
}

void manual_control()
{
  if (digitalRead(SwitchPin1) == LOW) {
    digitalWrite(RelayPin1, toggleState_1);
    toggleState_1 = !toggleState_1;
    Blynk.virtualWrite(VPIN_BUTTON_1, toggleState_1);
    delay(300); 
  }
  if (digitalRead(SwitchPin2) == LOW) {
    digitalWrite(RelayPin2, toggleState_2);
    toggleState_2 = !toggleState_2;
    Blynk.virtualWrite(VPIN_BUTTON_2, toggleState_2);
    delay(300);
  }
  if (digitalRead(SwitchPin3) == LOW) {
    digitalWrite(RelayPin3, toggleState_3);
    toggleState_3 = !toggleState_3;
    Blynk.virtualWrite(VPIN_BUTTON_3, toggleState_3);
    delay(300);
  }
  if (digitalRead(SwitchPin4) == LOW) {
    digitalWrite(RelayPin4, toggleState_4);
    toggleState_4 = !toggleState_4;
    Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_4);
    delay(300); 
  }
  if (digitalRead(SwitchPin5) == LOW) {
    digitalWrite(RelayPin5, toggleState_5);
    toggleState_5 = !toggleState_5;
    Blynk.virtualWrite(VPIN_BUTTON_5, toggleState_5);
    delay(300); 
  }
  if (digitalRead(SwitchPin6) == LOW) {
    digitalWrite(RelayPin6, toggleState_6);
    toggleState_6 = !toggleState_6;
    Blynk.virtualWrite(VPIN_BUTTON_6, toggleState_6);
    delay(300);
  }
  if (digitalRead(SwitchPin7) == LOW) {
    digitalWrite(RelayPin7, toggleState_7);
    toggleState_7 = !toggleState_7;
    Blynk.virtualWrite(VPIN_BUTTON_7, toggleState_7);
    delay(300);
  }
  if (digitalRead(SwitchPin8) == LOW) {
    digitalWrite(RelayPin8, toggleState_8);
    toggleState_8 = !toggleState_8;
    Blynk.virtualWrite(VPIN_BUTTON_8, toggleState_8);
    delay(300); 
  }
}
