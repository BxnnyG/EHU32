/*
 * EHU32.ino — Main entry point, global state, RTOS task creation and setup routine
 *
 * EHU32 turns a spare ESP32 into a Bluetooth A2DP audio gateway for GM/Vauxhall/Opel
 * vehicles fitted with an MS-CAN (Middle-Speed CAN, 95 kbit/s) radio such as the CD30,
 * CD30MP3, CD40USB, CD70 or DVD90.  The firmware:
 *   - Reads the MS-CAN bus via the ESP-IDF TWAI driver
 *   - Intercepts the radio's own display messages and replaces them with Bluetooth
 *     metadata (artist, title, album) or vehicle sensor data (coolant, speed, voltage)
 *   - Streams audio from any A2DP Bluetooth source to the vehicle's speakers via the
 *     PCM5102A I2S DAC
 *   - Provides over-the-air firmware updates through a Wi-Fi soft AP
 *
 * Dependencies:
 *   - config.h (pin definitions, CAN IDs, OTA password)
 *   - ESP32-AudioTools (I2S streaming)
 *   - ESP32-A2DP (Bluetooth A2DP sink + AVRCP)
 *   - ESP-IDF TWAI driver (bundled with the Arduino ESP32 core)
 *   - Arduino Preferences library (NVS-backed key-value store)
 *   - ArduinoOTA (Wi-Fi firmware update)
 *
 * Author: PNKP237 — https://github.com/PNKP237/EHU32
 */

#include "config.h"
#include "AudioTools.h"
#include "BluetoothA2DPSink.h"
#include "esp_sleep.h"
#include "driver/twai.h"
#include <Preferences.h>

/* Defining DEBUG enables Serial I/O for simulating button presses or faking
 * measurement blocks through a separate RTOS task (CANsimTask).
 * When DEBUG is defined, WiFi/OTA headers are excluded to save flash space. */
//#define DEBUG

#ifndef DEBUG
#include <WiFi.h>
#include <WiFiAP.h>
#include <ArduinoOTA.h>
#endif

/* Debug print macros — compile to nothing in release builds so no flash/RAM
 * is consumed and no serial output is produced on the target hardware. */
#ifdef DEBUG
#define DEBUG_SERIAL(X) Serial.begin(X)
#define DEBUG_PRINT(X) Serial.print(X)
#define DEBUG_PRINTLN(X) Serial.println(X)
#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG_SERIAL(X)
#define DEBUG_PRINT(X)
#define DEBUG_PRINTLN(X)
#define DEBUG_PRINTF(...)
#endif

/* ─────────────────────────── FreeRTOS Event-Group Flags ──────────────────────────
 * A single 32-bit EventGroup is used for inter-task signalling.  Each bit is a flag.
 *
 * Naming convention:
 *   DIS_*        – display / screen output flags
 *   CAN_*        – CAN-bus transmission/reception flags
 *   bt_* / a2dp_ – Bluetooth A2DP / AVRCP flags
 *   md_*         – AVRCP metadata flags
 *   OTA_*        – OTA update flags
 * ──────────────────────────────────────────────────────────────────────────────── */

/* Set by eventHandlerTask or canDisplayTask to request an immediate display
 * refresh.  Cleared once writeTextToDisplay() has been dispatched. */
#define DIS_forceUpdate (1 << 0)

/* Set by canTransmitTask after the ISO 15765-2 first frame is sent successfully,
 * telling canDisplayTask that flow-control (0x30) from the display is expected. */
#define CAN_MessageReady (1 << 1)

/* Set when a CAN transmission fails (queue timeout or TWAI error).  Used by
 * canDisplayTask to decide whether to retry the multi-packet transmission. */
#define CAN_prevTxFail (1 << 2)

/* Set when a new radio first frame (0x6C1 0x10…) arrives while EHU32 is still
 * sending consecutive frames, meaning the in-progress transmission is now stale
 * and must be abandoned. */
#define CAN_abortMultiPacket (1 << 3)

/* Set by canReceiveTask when the blocking message (Msg_PreventDisplayUpdate) could
 * not be transmitted in time.  canDisplayTask then adds a 300 ms penalty wait before
 * re-sending its own first frame to avoid a CAN error-frame collision. */
#define CAN_flowCtlFail (1 << 4)

/* Set by canProcessTask when a valid vehicle-speed measurement block has been
 * received and speed_buffer has been updated. */
#define CAN_speed_recvd (1 << 5)

/* Set by canProcessTask when a valid coolant-temperature measurement block has
 * been received and coolant_buffer has been updated. */
#define CAN_coolant_recvd (1 << 6)

/* Set by canProcessTask once ALL three measurement values (speed, coolant,
 * voltage) have arrived together, signalling eventHandlerTask to refresh the
 * display. */
#define CAN_new_dataSet_recvd (1 << 7)

/* Set by canProcessTask when a valid battery-voltage measurement block has been
 * received and voltage_buffer has been updated. */
#define CAN_voltage_recvd (1 << 8)

/* Reserved — not currently used in the main loop. */
#define CAN_measurements_requested (1 << 9)

/* Set by canActionEhuButton2/3 when disp_mode is changed, so eventHandlerTask
 * can show a "No data yet…" status before the first measurement arrives. */
#define disp_mode_changed (1 << 10)

/* When SET:  "Aux" has been detected on the radio's display → EHU32 may
 *            intercept and overwrite the display with its own content.
 * When CLEAR: "Aux" is not active → EHU32 must not touch the display. */
#define CAN_allowAutoRefresh (1 << 11)

/* Set once canProcessTask detects the first 0x6C8 message on the bus, meaning the
 * vehicle has an ECC (Electronic Climate Control) unit.  Used to select the
 * correct measurement-request target (ECC vs. DIS module). */
#define ECC_present (1 << 12)

/* Set after the first 0x6C1 display-update message from the radio is received,
 * meaning the radio has finished booting.  Triggers a2dp_init(). */
#define ehu_started (1 << 13)

/* Set once a2dp_init() has completed successfully. */
#define a2dp_started (1 << 14)

/* Set when the A2DP connection state callback reports state=2 (connected).
 * Cleared on disconnection. */
#define bt_connected (1 << 15)

/* Set by the A2DP connection state callback on any state transition, so
 * A2DP_EventHandler() can update the display with connect/disconnect status. */
#define bt_state_changed (1 << 16)

/* Set when the A2DP audio state callback reports state=2 (playing).
 * Cleared when state=1 (stopped/paused). */
#define bt_audio_playing (1 << 17)

/* Set by the A2DP audio state callback on any state transition. */
#define audio_state_changed (1 << 18)

/* Set by avrc_metadata_callback() when a new album string has been received. */
#define md_album_recvd (1 << 19)

/* Set by avrc_metadata_callback() when a new artist string has been received. */
#define md_artist_recvd (1 << 20)

/* Set by avrc_metadata_callback() when a new track title has been received. */
#define md_title_recvd (1 << 21)

/* Set by canActionEhuButton8() when button 8 is held ≥1 s, starting OTA mode.
 * Cleared implicitly by the OTA handler (which never returns). */
#define OTA_begin (1 << 22)

/* Set by canActionEhuButton8() when button 8 is held ≥5 s while already in OTA
 * mode, causing OTA_Handle() to call ESP.restart() and exit OTA. */
#define OTA_abort (1 << 23)

/* ──────────────────────────────── Pin Definitions ────────────────────────────── */
/* All pin assignments are defined in config.h:
 *   PCM_MUTE_CTL (23) — PCM5102A soft-mute control: HIGH = unmuted, LOW = muted
 *   PCM_ENABLE   (27) — PCM5102A power enable (active LOW); also releases SN65HVD230
 *   CAN_TX_PIN   (5)  — CAN transceiver TX
 *   CAN_RX_PIN   (4)  — CAN transceiver RX (also ext0 deep-sleep wake-up source)  */

/* ──────────────────────────── FreeRTOS Handles ───────────────────────────────────
 * RTOS Task Architecture — two cores:
 *
 *  Core 0 (PRO_CPU):
 *    canTransmitTask    (priority 1)  — dequeues messages from canTxQueue and
 *                                       transmits them via the TWAI driver.
 *    canProcessTask     (priority 2)  — decodes filtered messages from canRxQueue
 *                                       and updates data buffers / flags.
 *    canWatchdogTask    (idle prio)   — resets the device if the radio goes quiet
 *                                       for more than 15 seconds.
 *    canMessageDecoder  (idle prio)   — scans display byte-stream for "Aux" string
 *                                       to decide whether EHU32 may update display.
 *    canAirConMacroTask (priority 10) — plays back a sequence of simulated AC
 *                                       panel button presses; suspended unless active.
 *
 *  Core 1 (APP_CPU):
 *    canReceiveTask     (priority 1)  — receives all TWAI frames, filters them, and
 *                                       either acts immediately (0x6C1 blocking) or
 *                                       enqueues them to canRxQueue.
 *    canDisplayTask     (priority 1)  — implements ISO 15765-2 multi-packet TX to
 *                                       the display; suspended between transmissions.
 *    eventHandlerTask   (priority 4)  — drives the display state machine, handles
 *                                       A2DP events, performs CAN bus-off recovery.
 *
 * Inter-task communication:
 *   canRxQueue   (100 items) — raw twai_message_t frames from canReceiveTask
 *                               to canProcessTask.
 *   canTxQueue   (100 items) — outgoing twai_message_t frames queued by any task,
 *                               consumed by canTransmitTask.
 *   canDispQueue (255 items) — raw display payload bytes from canProcessTask
 *                               to canMessageDecoder (Aux detection).
 *   CAN_MsgSemaphore         — mutex protecting CAN_MsgArray[] while it is being
 *                               written by writeTextToDisplay() and read by
 *                               canDisplayTask.
 *   BufferSemaphore          — mutex protecting the text buffers (title_buffer etc.)
 *                               while they are written by callbacks or read by
 *                               writeTextToDisplay().
 *   eventGroup               — 32-bit flags for all other signalling (see above).
 * ──────────────────────────────────────────────────────────────────────────────── */
TaskHandle_t canReceiveTaskHandle, canDisplayTaskHandle, canProcessTaskHandle, canTransmitTaskHandle, canWatchdogTaskHandle, canAirConMacroTaskHandle, canMessageDecoderTaskHandle, eventHandlerTaskHandle;
QueueHandle_t canRxQueue, canTxQueue, canDispQueue;
SemaphoreHandle_t CAN_MsgSemaphore=NULL, BufferSemaphore=NULL;
EventGroupHandle_t eventGroup;

/* TWAI driver state — shared between eventHandlerTask (bus-off recovery) and
 * canTransmitTask (alert monitoring). */
uint32_t alerts_triggered;
twai_status_info_t status_info;

/* CAN identifier used for ISO 15765-2 transmissions to the display (e.g. 0x6C0,
 * 0x6C2 …).  Determined at first boot by testing which unused ID the display
 * responds to, then persisted in NVS flash.  The corresponding flow-control
 * response comes from (displayMsgIdentifier − 0x400). */
uint32_t displayMsgIdentifier=0;

/* ─────────────────────────────── Data Buffers ────────────────────────────────────
 * DisplayMsg[1024]   — Assembled ISO 15765-2 payload before it is split into
 *                      8-byte CAN frames.  1024 bytes covers the worst-case
 *                      UTF-16 expansion of all three display lines.
 * CAN_MsgArray[128][8] — 128 pre-labelled 8-byte CAN frames ready for sequential
 *                        transmission.  Index 0 is always the ISO 15765-2 first
 *                        frame (0x10 / 0x11); subsequent frames are consecutive
 *                        frames (0x21 … 0x2F, wrapping back to 0x20).
 * title_buffer[64]   — Current track title from AVRCP metadata (UTF-8, max 63 chars).
 * artist_buffer[64]  — Current artist name from AVRCP metadata.
 * album_buffer[64]   — Current album name from AVRCP metadata.
 * coolant_buffer[32] — Formatted coolant temperature string (e.g. "Coolant temp: 87°C").
 * speed_buffer[32]   — Formatted speed/RPM string (e.g. "80 km/h 2000 RPM").
 * voltage_buffer[32] — Formatted battery voltage string (e.g. "Voltage: 13.8 V").
 * ──────────────────────────────────────────────────────────────────────────────── */
char DisplayMsg[1024], CAN_MsgArray[128][8], title_buffer[64], artist_buffer[64], album_buffer[64];
char coolant_buffer[32], speed_buffer[32], voltage_buffer[32];

/* Display operating mode — controls what writeTextToDisplay() renders:
 *   -1  Screen updates disabled (button 9 held ≥500 ms); EHU32 stops
 *       intercepting the radio's display messages entirely.
 *    0  Audio metadata mode: show Bluetooth track title, artist and album.
 *       This is also the default mode shown while waiting for a connection.
 *    1  Three-line vehicle data: coolant temperature, speed/RPM and voltage.
 *    2  Single-line coolant temperature only (low-priority background display). */
volatile int disp_mode=-1;

/* Timestamp variables used to rate-limit periodic actions:
 *   last_millis      — general-purpose last-event timestamp (unused in main loop).
 *   last_millis_req  — time of the last measurement-block request to the ECU.
 *   last_millis_disp — time of the last display update (throttles refresh rate).
 *   last_millis_aux  — time the "Aux" string was last seen on the display bus;
 *                      used to implement the 6-second Aux auto-refresh timeout. */
unsigned long last_millis=0, last_millis_req=0, last_millis_disp=0, last_millis_aux=0;

/* Vehicle configuration flags loaded from (or saved to) NVS at startup:
 *   vehicle_ECC_present — true when 0x6C8 (ECC module) was detected on the bus.
 *                         Selects the ECC-specific measurement request messages.
 *   vehicle_UHP_present — true when 0x6C7 (UHP, factory Bluetooth hands-free) was
 *                         detected.  Disables EHU32's play/pause shortcut to avoid
 *                         conflicting with the factory unit. */
bool vehicle_ECC_present, vehicle_UHP_present;

void canReceiveTask(void* pvParameters);
void canTransmitTask(void* pvParameters);
void canProcessTask(void* pvParameters);
void canDisplayTask(void* pvParameters);
void canWatchdogTask(void* pvParameters);
void canAirConMacroTask(void* pvParameters);
void OTAhandleTask(void* pvParameters);
void prepareMultiPacket(int bytes_processed, char* buffer_to_read);
int processDisplayMessage(char* upper_line_buffer, char* middle_line_buffer, char* lower_line_buffer);

/* setup() — Arduino entry point; runs once on Core 1 before loop() starts.
 *
 * Step 1: Configure ext0 deep-sleep wake-up on GPIO 4 (CAN RX, active LOW).
 *         This means any CAN activity while sleeping will wake the ESP32.
 * Step 2: Configure PCM5102A control pins; start with audio muted and DAC off
 *         to avoid pops while the CAN bus is not yet confirmed active.
 * Step 3: Initialise the TWAI (CAN) driver at 95 kbit/s.
 * Step 4: Attempt a single CAN receive with a 100 ms timeout.  If no message
 *         arrives, assume the vehicle is off and enter deep sleep immediately.
 * Step 5: Enable the PCM5102A and release the SN65HVD230 CAN transceiver from
 *         standby now that bus activity has been confirmed.
 * Step 6: First-boot CAN setup (only when NVS key "setupcomplete" == 0):
 *         a) Wait for the radio's 0x6C1 boot message (0x40 or 0xC0 pattern).
 *         b) Scan 20 seconds of traffic for IDs in the 0x6C0–0x6CF range to
 *            find which ones are already in use.
 *         c) Try each unused ID as a display transmission source (ISO 15765-2
 *            first frame) and look for a flow-control response (0x30) on the
 *            corresponding 0x2C0–0x2CF response ID.
 *         d) Persist the working displayMsgIdentifier to NVS flash.
 * Step 7: On subsequent boots, load displayMsgIdentifier and vehicle config
 *         flags directly from NVS flash.
 * Step 8: Create FreeRTOS synchronisation primitives (mutexes, queues,
 *         event group).
 * Step 9: Create and pin all RTOS tasks to their respective cores.
 */
void setup(){
  /* Step 1 — Wake-up source: CAN_RX_PIN (GPIO 4) is the CAN RX line; a LOW level
   * (dominant CAN bit) will wake the ESP32 from deep sleep. */
  esp_sleep_enable_ext0_wakeup(CAN_RX_PIN, 0);
  pinMode(PCM_MUTE_CTL, OUTPUT);
  pinMode(PCM_ENABLE, OUTPUT);          // control PCM5102 power setting
  digitalWrite(PCM_MUTE_CTL, HIGH);    // unmuted — audio path ready
  digitalWrite(PCM_ENABLE, HIGH);      // DAC + transceiver off until CAN confirmed
  DEBUG_SERIAL(921600);                 // serial comms for debug

  /* Step 3 — Initialise TWAI at 95 kbit/s (MS-CAN bus speed). */
  twai_init();
  twai_message_t testMsg;
  /* Step 4 — If no CAN frame arrives within 100 ms, assume ignition is off;
   * go back to deep sleep and wait for the next bus wake-up event. */
  if(twai_receive(&testMsg, pdMS_TO_TICKS(100))!=ESP_OK){
    DEBUG_PRINTLN("CAN inactive. Back to sleep!");
    #ifdef DEBUG
    vTaskDelay(pdMS_TO_TICKS(10));  // wait for a bit for the buffer to be transmitted when debugging
    #endif
    esp_deep_sleep_start();                     // enter deep sleep
  }

  /* Step 5 — Bus is active: enable PCM5102A (LOW = active) and release the
   * SN65HVD230 CAN transceiver from standby (active-low STANDBY pin on KF50BD). */
  digitalWrite(PCM_ENABLE, LOW);

  /* Step 6 — First-boot CAN setup stored in NVS under namespace "my-app". */
  Preferences settings;
  settings.begin("my-app", false);
  if(!settings.isKey("setupcomplete")){
    /* NVS is blank (fresh flash) — create all required keys with defaults. */
    DEBUG_PRINTLN("CAN SETUP: Key does not exist! Creating keys");
    settings.clear();
    settings.putBool("setupcomplete", 0);
    settings.putBool("uhppresent", 0);
    settings.putBool("eccpresent", 0);
    settings.putBool("vectra", 0);
    settings.putUInt("identifier", 0);
  }
  bool init_setupComplete=settings.getBool("setupcomplete", 0);
  if(!init_setupComplete){
    /* Step 6a — Wait for the radio's CAN_ID_RADIO_DISPLAY (0x6C1) boot message
     * with data[2]==0x40 or 0xC0 (the standard CD30/CD40/CD70 init pattern). */
    while(twai_receive(&testMsg, portMAX_DELAY)!=ESP_OK && testMsg.identifier!=CAN_ID_RADIO_DISPLAY && (testMsg.data[2]!=0x40 || testMsg.data[2]!=0xC0)) {}
    unsigned long millis_init_start=millis();       // got 0x6C1, assume radio started now
    /* Step 6b — Scan 20 s of traffic and record which IDs in the 0x6C0–0x6CF
     * range are already active.  Index i maps to ID (0x6C0 + i). */
    bool init_usedCANids[16];     // represents 0x6C0 to 0x6CF
    while(twai_receive(&testMsg, portMAX_DELAY)==ESP_OK && (millis_init_start+20000>millis())){
      if((testMsg.identifier & 0xFF0)==0x6C0 && !init_usedCANids[testMsg.identifier-0x6c0]){
        init_usedCANids[testMsg.identifier-0x6c0]=1;
        if(testMsg.identifier==CAN_ID_UHP_PRESENCE){
          /* CAN_ID_UHP_PRESENCE (0x6C7) — UHP (factory Bluetooth hands-free) module. */
          settings.putBool("uhppresent", 1);
          vehicle_UHP_present=1;
        }
        if(testMsg.identifier==CAN_ID_ECC_PRESENCE){
          /* CAN_ID_ECC_PRESENCE (0x6C8) — ECC (Electronic Climate Control) module.
           * Note: ECC only transmits after key-at-ignition. */
          settings.putBool("eccpresent", 1);
          vehicle_ECC_present=1;
        }
        DEBUG_PRINTF("CAN SETUP: Marking 0x%03X as a CAN ID in use\n", testMsg.identifier);
      }
    }
    /* Step 6c — Try each unused 0x6C0–0x6CF ID as a transmit source.
     * Send a dummy ISO 15765-2 first frame; if the display replies with a
     * flow-control frame (0x30) on the corresponding 0x2Cx address, that ID
     * is valid and is saved as displayMsgIdentifier.
     * Fallback 1: 0x6C8 (ECC ID) if ECC is present but no unused ID works.
     * Fallback 2: 0x6C1 (stock radio ID) with extra overwrite logic. */
    DEBUG_PRINT("CAN SETUP: Attempting to test display responses to tested identifiers: ");
    twai_message_t testMsgTx={ .identifier=0x6C0, .data_length_code=8, .data={0x10, 0xA7, 0x40, 0x00, 0xA4, 0x03, 0x10, 0x13}};
    unsigned long millis_transmitted;
    for(int i=0; i<16 && displayMsgIdentifier==0; i++){
      if(init_usedCANids[i])  i++;  // skip IDs in active use
      testMsgTx.identifier=(0x6C0+i);
      DEBUG_PRINTF("0x%03X... ", testMsgTx.identifier);
      twai_transmit(&testMsgTx, pdMS_TO_TICKS(300));
      millis_transmitted=millis();
      while((millis_transmitted+1000>millis()) && displayMsgIdentifier==0){
        if(twai_receive(&testMsg, pdMS_TO_TICKS(300))==ESP_OK){
          /* A 0x30 flow-control response on (txID − 0x400) confirms the display
           * accepted our first frame — this is the ID to use. */
          if(testMsg.identifier==(testMsgTx.identifier-0x400) && testMsg.data[0]==0x30){
            displayMsgIdentifier=testMsgTx.identifier;
            DEBUG_PRINTF("got a response on 0x%03X!", testMsg.identifier);
          }
        }
      }
      vTaskDelay(pdMS_TO_TICKS(100));
    }
    if(displayMsgIdentifier==0){
      if(init_usedCANids[8]==1){
        displayMsgIdentifier=CAN_ID_ECC_PRESENCE;
        DEBUG_PRINTLN("\nCAN SETUP: Unable to find a valid unused CAN ID, but detected ECC -> using 0x6C8");
      } else {
        displayMsgIdentifier=CAN_ID_RADIO_DISPLAY;
        DEBUG_PRINTLN("\nCAN SETUP: Unable to find a valid unused CAN ID. Falling back to stock -> using 0x6C1");
      }
    }
    /* Step 6d — Persist the discovered ID so subsequent boots skip this scan. */
    DEBUG_PRINTLN("\nCAN SETUP: Saving the display message identifier to flash...");
    settings.putUInt("identifier", displayMsgIdentifier);
    settings.putBool("setupcomplete", 1);
  } else {
    /* Step 7 — Not first boot: load previously stored configuration from NVS. */
    displayMsgIdentifier=settings.getUInt("identifier", 0);
    DEBUG_PRINTF("CAN SETUP: Get the display identifier from flash -> 0x%03X\n", displayMsgIdentifier);
    vehicle_ECC_present=settings.getBool("eccpresent", 0);
    vehicle_UHP_present=settings.getBool("uhppresent", 0);
  }
  if(displayMsgIdentifier==0){
    /* Safety net: a zero identifier would break all display transmissions.
     * Clear the setup flag so the full discovery runs again on the next boot. */
    DEBUG_PRINTLN("CAN SETUP: identifier can't be 0, rerunning CAN setup...");
    settings.putBool("setupcomplete", 0);
    settings.end();
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP.restart();
  } else {
    settings.end();
  }

  /* Step 8 — Create FreeRTOS synchronisation primitives.
   *   CAN_MsgSemaphore: taken while CAN_MsgArray[] is being written (by
   *                     writeTextToDisplay) or read (by canDisplayTask).
   *   BufferSemaphore:  taken while the text buffers (title, artist, …) are
   *                     read or written; prevents a partial read mid-update. */
  CAN_MsgSemaphore=xSemaphoreCreateMutex();
  BufferSemaphore=xSemaphoreCreateMutex();
  canRxQueue=xQueueCreate(100, sizeof(twai_message_t));        // incoming CAN frames → canProcessTask
  canTxQueue=xQueueCreate(100, sizeof(twai_message_t));        // outgoing CAN frames → canTransmitTask
  canDispQueue=xQueueCreate(255, sizeof(uint8_t));             // raw display bytes → canMessageDecoder (Aux detection)
  eventGroup=xEventGroupCreate();

  /* Step 9 — Create all RTOS tasks, pinned to the appropriate core. */
  xTaskCreatePinnedToCore(canReceiveTask, "CANbusReceiveTask", 4096, NULL, 1, &canReceiveTaskHandle, 1);   // Core 1, prio 1
  xTaskCreatePinnedToCore(canTransmitTask, "CANbusTransmitTask", 4096, NULL, 1, &canTransmitTaskHandle, 0); // Core 0, prio 1
  xTaskCreatePinnedToCore(canProcessTask, "CANbusMessageProcessor", 8192, NULL, 2, &canProcessTaskHandle, 0); // Core 0, prio 2
  xTaskCreatePinnedToCore(canDisplayTask, "DisplayUpdateTask", 8192, NULL, 1, &canDisplayTaskHandle, 1);    // Core 1, prio 1
  vTaskSuspend(canDisplayTaskHandle);  // suspended until there is something to send
  xTaskCreatePinnedToCore(canWatchdogTask, "WatchdogTask", 2048, NULL, tskIDLE_PRIORITY, &canWatchdogTaskHandle, 0); // Core 0, idle
  xTaskCreatePinnedToCore(canMessageDecoder, "MessageDecoder", 2048, NULL, tskIDLE_PRIORITY, &canMessageDecoderTaskHandle, 0); // Core 0, idle
  vTaskSuspend(canMessageDecoderTaskHandle);  // suspended until ehu_started is set
  #ifdef DEBUG 
  xTaskCreate(CANsimTask, "CANbusSimulateEvents", 2048, NULL, 21, NULL);  // allows to simulate button presses through serial
  #endif
  xTaskCreatePinnedToCore(canAirConMacroTask, "AirConMacroTask", 2048, NULL, 10, &canAirConMacroTaskHandle, 0); // Core 0, prio 10
  vTaskSuspend(canAirConMacroTaskHandle);  // only resumed on AC button long-press
  xTaskCreatePinnedToCore(eventHandlerTask, "eventHandler", 8192, NULL, 4, &eventHandlerTaskHandle, 1); // Core 1, prio 4
}

/* canWatchdogTask — Core 0, idle priority
 *
 * Purpose: reset the device if the radio stops transmitting its periodic
 *          0x6C1 display messages.  The radio sends these roughly every 5 s;
 *          a 15-second timeout provides a generous safety margin.
 *
 * Communication: canProcessTask calls xTaskNotifyGive(canWatchdogTaskHandle)
 *                each time a valid 0x6C1 message is received.
 */
// this task monitors radio messages and resets the program if the radio goes to sleep or CAN dies
void canWatchdogTask(void *pvParameters){
  static BaseType_t notifResult;
  while(1){
    notifResult=xTaskNotifyWait(0, 0, NULL, pdMS_TO_TICKS(15000));           // wait for a notification that display packet from the radio unit has been received
    if(notifResult==pdFAIL){            // if the notification has not been received in the specified timeframe (radio sends its display messages each 5s, specified timeout of 15s for safety) we assume the radio is off
      DEBUG_PRINTLN("WATCHDOG: Triggering software reset...");
      vTaskDelay(pdMS_TO_TICKS(100));
      a2dp_shutdown();      // this or disp_mode=-1?
    } else {
      DEBUG_PRINTLN("WATCHDOG: Reset successful.");
      xTaskNotifyStateClear(NULL);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// reads settings from preferences
bool getPreferencesBool(const char* key){
  Preferences settings;
  bool result;
  settings.begin("my-app", true);
  result=settings.getBool(key, 0);
  settings.end();
  return result;
}

// writes settings to preferences
void setPreferencesBool(const char* key, bool value){
  Preferences settings;
  settings.begin("my-app", false);
  settings.putBool(key, value);
  settings.end();
}

// below functions are used to simplify interaction with freeRTOS eventGroups
void setFlag(uint32_t bit){
  xEventGroupSetBits(eventGroup, bit);
}

// clears an event bit
void clearFlag(uint32_t bit){
  xEventGroupClearBits(eventGroup, bit);
}

// waits for an event bit to be set, blocking indefinitely if 2nd argument not provided
void waitForFlag(uint32_t bit, TickType_t ticksToWait=portMAX_DELAY){
  xEventGroupWaitBits(eventGroup, bit, pdFALSE, pdTRUE, ticksToWait);
}

// Check if a specific event bit is set (without blocking)
bool checkFlag(uint32_t bit){
  EventBits_t bits=xEventGroupGetBits(eventGroup);
  return (bits&bit)!=0;
}

// used to clear saved settings and go through the setup again on next reboot
void prefs_clear(){
  Preferences settings;
  settings.begin("my-app", false);
  settings.clear();
  settings.end();
}

/* writeTextToDisplay() — bridge between text data and CAN display messages.
 *
 * Parameters:
 *   disp_mode_override — when false (default), the current disp_mode determines
 *                        what is displayed (audio metadata, body data, etc.).
 *                        When true, the caller-supplied text arguments are used
 *                        directly, bypassing disp_mode.  Used for one-off status
 *                        messages ("EHU32 v0.9.5", "OTA Started", errors, etc.).
 *   up_line_text    — UTF-8 text for the upper line (album / first info).
 *   mid_line_text   — UTF-8 text for the middle line (title / primary info).
 *   low_line_text   — UTF-8 text for the lower line (artist / secondary info).
 *                     Any of these may be nullptr to leave the line blank.
 *
 * Mode routing (disp_mode_override == false):
 *   disp_mode  0 — audio metadata: upper=album, mid=title, low=artist.
 *                  Only writes if at least one buffer is non-empty.
 *   disp_mode  1 — body data (3-line): upper=coolant, mid=speed, low=voltage.
 *   disp_mode  2 — single-line coolant: mid=coolant only.
 *   disp_mode -1 — screen updates disabled; this function should not be called.
 *
 * Semaphore protocol:
 *   Both CAN_MsgSemaphore and BufferSemaphore are taken before building the
 *   message and released after prepareMultiPacket() fills CAN_MsgArray[].
 *   CAN_MsgSemaphore prevents canDisplayTask from reading a partially-built
 *   frame array.  BufferSemaphore prevents the text buffers being overwritten
 *   by A2DP callbacks while they are being converted to UTF-16.
 *
 * After writing: canDisplayTaskHandle is resumed so the new frame array is
 *               transmitted immediately.
 */
// processes data based on the current value of disp_mode or prints one-off messages by specifying the data in arguments; message is then transmitted right away
// it acts as a bridge between UTF-8 text data and the resulting CAN messages meant to be transmitted to the display
void writeTextToDisplay(bool disp_mode_override=0, char* up_line_text=nullptr, char* mid_line_text=nullptr, char* low_line_text=nullptr){             // disp_mode_override exists as a simple way to print one-off messages (like board status, errors and such)
  DEBUG_PRINTLN("EVENTS: Refreshing buffer...");
  xSemaphoreTake(CAN_MsgSemaphore, portMAX_DELAY);      // take the semaphore as a way to prevent any transmission when the message structure is being written
  xSemaphoreTake(BufferSemaphore, portMAX_DELAY);       // we take both semaphores, since this task specifically interacts with both the internal data buffers and the CAN message buffer
  if(!disp_mode_override){
    if(disp_mode==0 && (album_buffer[0]!='\0' || title_buffer[0]!='\0' || artist_buffer[0]!='\0')){         // audio metadata mode
      prepareMultiPacket(processDisplayMessage(album_buffer, title_buffer, artist_buffer), DisplayMsg);               // prepare a 3-line message (audio Title, Album and Artist)
    } else {
      if(disp_mode==1){                                     // vehicle data mode (3-line)
        prepareMultiPacket(processDisplayMessage(coolant_buffer, speed_buffer, voltage_buffer), DisplayMsg);               // vehicle data buffer
      }
      if(disp_mode==2){                                     // coolant mode (1-line)
        prepareMultiPacket(processDisplayMessage(nullptr, coolant_buffer, nullptr), DisplayMsg);               // vehicle data buffer (single line)
      }
    }
  } else {                                   // overriding buffers
    prepareMultiPacket(processDisplayMessage(up_line_text, mid_line_text, low_line_text), DisplayMsg);
  }
  xSemaphoreGive(CAN_MsgSemaphore);
  xSemaphoreGive(BufferSemaphore);        // releasing semaphores
  vTaskResume(canDisplayTaskHandle);            // buffer has been updated, transmit 
  clearFlag(DIS_forceUpdate);
}

/* eventHandlerTask — Core 1, priority 4
 *
 * Purpose: central event dispatcher.  Runs in a tight 10 ms loop and handles:
 *   1. OTA trigger   — when OTA_begin is set, shows a status message, suspends
 *                      the watchdog, and blocks inside OTA_Handle() forever.
 *   2. Body data mode (disp_mode == 1) — requests all three measurement blocks
 *      (speed/RPM, coolant, voltage) every 400 ms and refreshes the display
 *      once a complete dataset has arrived.
 *   3. Coolant-only mode (disp_mode == 2) — requests only coolant temperature
 *      every 3 seconds (lower frequency since the value changes slowly).
 *   4. CAN bus-off recovery — detects TWAI_STATE_BUS_OFF, suspends all CAN
 *      tasks, uninstalls and reinstalls the TWAI driver, then resumes them.
 *   5. Bluetooth events — delegates to A2DP_EventHandler() for connection,
 *      disconnection, play/pause and metadata events.
 */
// this task handles events and output to display in context of events, such as new data in buffers or A2DP events
void eventHandlerTask(void *pvParameters){
  while(1){
    if(checkFlag(OTA_begin)){
      disp_mode=0;
      writeTextToDisplay(1, "Bluetooth off", "OTA Started", "Waiting for connection...");
      vTaskDelay(1000);
      vTaskSuspend(canWatchdogTaskHandle);    // so I added the watchdog but forgot to suspend it when starting OTA. result? Couldn't update it inside the car and had to take the radio unit out to do it manually
      #ifndef DEBUG
      OTA_Handle();
      #endif
    }

    if(disp_mode==1 && checkFlag(ehu_started)){                           // if running in measurement block mode, check time and if enough time has elapsed ask for new data
      if(checkFlag(disp_mode_changed)){
        clearFlag(disp_mode_changed);
        writeTextToDisplay(1, nullptr, "No data yet...", nullptr);      // print a status message that will stay if display/ecc are not responding
      }
      if((last_millis_req+400)<millis()){
        requestMeasurementBlocks();
        last_millis_req=millis();
      }
      if(((last_millis_disp+400)<millis()) && checkFlag(CAN_new_dataSet_recvd)){               // print new data if it has arrived
        clearFlag(CAN_new_dataSet_recvd);
        writeTextToDisplay();
        last_millis_disp=millis();
      }
    }

    if(disp_mode==2 && checkFlag(ehu_started)){                  // single line measurement mode only provides coolant temp.
      if(checkFlag(disp_mode_changed)){
        clearFlag(disp_mode_changed);
        writeTextToDisplay(1, nullptr, "No data yet...", nullptr);      // print a status message that will stay if display/ecc are not responding
      }
      if((last_millis_req+3000)<millis()){        // since we're only updating coolant data, I'd say that 3 secs is a fair update rate
        requestCoolantTemperature();
        last_millis_req=millis();
      }
      if(((last_millis_disp+3000)<millis()) && checkFlag(CAN_coolant_recvd)){
        clearFlag(CAN_coolant_recvd);
        writeTextToDisplay();
        last_millis_disp=millis();
      }
    }

    twai_get_status_info(&status_info);  
    // this will try to get CAN back up in case it fails
    if((status_info.state==TWAI_STATE_BUS_OFF) || (twai_get_status_info(&status_info)==ESP_ERR_INVALID_STATE)){
      DEBUG_PRINTLN("CAN: DETECTED BUS OFF. TRYING TO RECOVER -> REINSTALLING");
      vTaskSuspend(canReceiveTaskHandle);
      vTaskSuspend(canTransmitTaskHandle);
      vTaskSuspend(canProcessTaskHandle);
      vTaskSuspend(canDisplayTaskHandle);
      vTaskSuspend(canWatchdogTaskHandle);
      //twai_initiate_recovery();                     // twai_initiate_recovery(); leads to hard crashes - it's something that ESP-IDF need to fix
      twai_stop();
      if(twai_driver_uninstall()==ESP_OK){
        DEBUG_PRINTLN("CAN: TWAI DRIVER UNINSTALL OK");
      } else {
        DEBUG_PRINTLN("CAN: TWAI DRIVER UNINSTALL FAIL!!! Rebooting...");          // total fail - just reboot at this point
        vTaskDelay(pdMS_TO_TICKS(100));
        ESP.restart();
      }
      vTaskDelay(100);
      twai_init();
      vTaskDelay(100);
      vTaskResume(canReceiveTaskHandle);
      vTaskResume(canTransmitTaskHandle);
      vTaskResume(canProcessTaskHandle);
      vTaskResume(canDisplayTaskHandle);
      vTaskResume(canWatchdogTaskHandle);
    }

    A2DP_EventHandler();          // process bluetooth and audio flags set by A2DP callbacks
    vTaskDelay(10);
  }
}

// loop will do nothing
void loop(){
  vTaskDelay(pdMS_TO_TICKS(1000));
}