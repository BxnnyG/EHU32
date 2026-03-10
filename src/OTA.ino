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

/* OTA state machine variables:
 *
 *   OTA_running    — set to true once WiFi.softAP() and ArduinoOTA.begin()
 *                    succeed.  Prevents OTA_start() from being called twice.
 *
 *   OTA_finished   — set to true in the ArduinoOTA onEnd() callback when the
 *                    firmware upload completes successfully.  OTA_Handle() then
 *                    waits 1 s and calls ESP.restart() to boot the new firmware.
 *
 *   OTA_progressing — set to true in the ArduinoOTA onStart() callback and
 *                     cleared in onEnd().  While true, the 10-minute idle
 *                     timeout is suspended so an in-progress upload is never
 *                     interrupted.
 */
volatile bool OTA_running=0, OTA_finished=0, OTA_progressing=0;

#ifndef DEBUG
/* OTA_start() — initialise Wi-Fi AP and ArduinoOTA.
 *
 * Steps:
 *   1. Stop the A2DP sink (Bluetooth and Wi-Fi share the same radio on ESP32;
 *      they cannot run simultaneously).
 *   2. Build a unique SSID ("EHU32-XXYY") from the last two bytes of the
 *      soft-AP MAC address to allow identifying the target unit.
 *   3. Start the Wi-Fi soft-AP with the password "ehu32updater".
 *      On failure, restart the ESP32.
 *   4. Display the SSID and password on the car display.
 *   5. Configure ArduinoOTA:
 *      - mDNS disabled (not useful in a car environment).
 *      - Reboot on success (boot the new firmware automatically).
 *      - OTA password set (second authentication layer beyond the Wi-Fi password).
 *      - onStart: marks OTA_progressing=1.
 *      - onProgress: updates the display every 10% of progress.
 *      - onError: displays the error reason and restarts after 3 s.
 *        Errors: OTA_AUTH_ERROR (wrong password), OTA_BEGIN_ERROR (flash init),
 *                OTA_CONNECT_ERROR (upload client disconnected), OTA_RECEIVE_ERROR,
 *                OTA_END_ERROR (flash finalisation failed).
 *      - onEnd: clears NVS settings (forces fresh CAN setup after update)
 *               and marks OTA_finished=1.
 *   6. Set OTA_running=1.
 */
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

/* OTA_Handle() — main OTA event loop; called from eventHandlerTask when
 *                OTA_begin is set (button 8 held ≥1 s).  Blocks forever.
 *
 * Flow:
 *   1. Wait (polling every 1 s) until OTA_begin is set.
 *   2. If OTA_running is false, call OTA_start() and record the start time.
 *   3. Call ArduinoOTA.handle() repeatedly to process incoming UDP packets.
 *   4. Timeout (10 minutes): if !OTA_progressing and 600 000 ms have elapsed
 *      since OTA_start(), call ESP.restart() to exit OTA mode.
 *   5. Internal watchdog: when OTA is running but idle (!progressing, !finished),
 *      vTaskDelay(1) yields to the FreeRTOS watchdog timer.
 *   6. Abort: if OTA_abort is set (button 8 held ≥5 s) and no upload is in
 *      progress, call ESP.restart() immediately.
 *   7. Success: if OTA_finished is set (upload complete), wait 1 s and restart
 *      to boot the new firmware.
 *
 * Note: `time_started` is initialised to 0 and set to millis() only after
 *       OTA_start() succeeds, so the 10-minute timer begins at AP start time.
 */
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