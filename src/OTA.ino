/*
 * OTA (Over-The-Air) Update Module
 *
 * How it works:
 *   The ESP32 starts a Wi-Fi soft-AP (hotspot) and listens for firmware uploads
 *   using the ArduinoOTA protocol (UDP port 3232 by default).  A desktop running
 *   Arduino IDE (or arduino-cli) can then upload a new sketch wirelessly.
 *
 * Security measures:
 *   1. Unique SSID  – The hotspot name is derived from the last two bytes of the
 *      ESP32's soft-AP MAC address (e.g. "EHU32-A1B2"), so multiple units can be
 *      told apart and only the intended device is targeted.
 *   2. Wi-Fi password – The hotspot is password-protected ("ehu32updater").
 *      This prevents neighbours from connecting at all.
 *   3. OTA password  – Even after connecting to the Wi-Fi, the ArduinoOTA
 *      handshake requires the same password before accepting a firmware upload.
 *      This adds a second layer against accidental or malicious flashing.
 *
 * Changing the password:
 *   Update the string passed to both WiFi.softAP() and ArduinoOTA.setPassword()
 *   below, keeping both values in sync.
 *
 * Timeout:
 *   OTA mode times out automatically after 10 minutes with no upload activity
 *   and the device resets back to normal operation.
 *   Pressing button 8 for ≥5 s while in OTA mode aborts and resets immediately.
 */

// Wi-Fi AP credentials – ssid is filled at runtime with the device MAC address
char ssid[20];
const char* password = "ehu32updater";
volatile bool OTA_running=0, OTA_finished=0, OTA_progressing=0;
#ifndef DEBUG
// initialize OTA functionality as a way to update firmware; this disables A2DP functionality!
void OTA_start(){
  //twai_stop();
  a2dp_sink.end(true);
  vTaskDelay(pdMS_TO_TICKS(500));
  // Build a unique SSID from the last two bytes of the soft-AP MAC address
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
  snprintf(ssid, sizeof(ssid), "EHU32-%02X%02X", mac[4], mac[5]);
  if (!WiFi.softAP(ssid, password)) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP.restart();
  } else {
    IPAddress myIP = WiFi.softAPIP();
    // Show the hotspot name and IP on the car display so the user knows what to connect to
    char ota_info[48];
    snprintf(ota_info, sizeof(ota_info), "SSID: %s", ssid);
    writeTextToDisplay(1, "OTA Update Mode", ota_info, "Pass: ehu32updater");
    ArduinoOTA
      .setMdnsEnabled(false)
      .setRebootOnSuccess(true)
      .setPassword("ehu32updater")  // OTA-level auth: second layer on top of Wi-Fi password
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else
          type = "filesystem";
          OTA_progressing=1;
      })
      .onProgress([](unsigned int progress, unsigned int total) {           // gives visual updates on the 
        //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        if(((progress / (total / 100))%10)==0){
          unsigned int progress_val=progress / (total / 100);
          char progress_text[32];
          snprintf(progress_text, sizeof(progress_text), "Updating... %d%%", progress_val);
          writeTextToDisplay(1, nullptr, progress_text, nullptr);
        }
      })
      .onError([](ota_error_t error) {
        char err_reason[32];
        switch(error){
          case OTA_AUTH_ERROR:{
            snprintf(err_reason, sizeof(err_reason), "Not authenticated");
            break;
          }
          case OTA_BEGIN_ERROR:{
            snprintf(err_reason, sizeof(err_reason), "Error starting");
            break;
          }
          case OTA_CONNECT_ERROR:{
            snprintf(err_reason, sizeof(err_reason), "Connection problem");
            break;
          }
          case OTA_RECEIVE_ERROR:{
            snprintf(err_reason, sizeof(err_reason), "Error receiving");
            break;
          }
          case OTA_END_ERROR:{
            snprintf(err_reason, sizeof(err_reason), "Couldn't apply update");
            break;
          }
          default: break;
        }
        writeTextToDisplay(1, "Error updating", err_reason, "Resetting...");
        vTaskDelay(pdMS_TO_TICKS(3000));
        ESP.restart();
      })
      .onEnd([]() {
        prefs_clear();    // reset settings, ensures any new setup changes/optimizations will be useful
        OTA_finished=1;
        OTA_progressing=0;
      });
    ArduinoOTA.begin();
    OTA_running=1;
  }
}

void OTA_Handle(){
  unsigned long time_started=0;
  while(1){
    while(!checkFlag(OTA_begin)){
      vTaskDelay(1000);
    }
    if(!OTA_running){
      OTA_start();
      time_started=millis();
    }
    ArduinoOTA.handle();
    if(!OTA_progressing){                     // timeout after 10 minutes of no OTA start
      if((time_started+600000)<millis()){
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP.restart();
      }
    }
    if(OTA_running && !OTA_progressing && !OTA_finished){ // keeps the internal watchdog happy when not doing anything
      vTaskDelay(1);
    }
    if(checkFlag(OTA_abort) && OTA_running && !OTA_progressing && !OTA_finished){  // provides a way to reset the ESP if stuck in OTA
      vTaskDelay(pdMS_TO_TICKS(1000));
      ESP.restart();
    }
    if(OTA_finished){
      vTaskDelay(pdMS_TO_TICKS(1000));
      ESP.restart();
    }
  }
}
#endif