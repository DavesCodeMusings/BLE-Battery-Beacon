/*
 * Bluetooth Low Energy (BLE) Battery Beacon
 *
 * A very simple example of using Bluetooth Low Energy (BLE) to give the
 * status of a fictitious ESP32 attached battery. This is only an example.
 * It does not actually read a battery, because there isn't one. It simply 
 * reports 100% whenever it is polled.
 *
 * Copyright (c) 2024 David Horton
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define OPERATING_TIME_SECS 60
#define SLEEPING_TIME_SECS 120
#define LED_BUILTIN 2
#include <ArduinoBLE.h>

// Used for microsecond delay calculations.
const uint64_t uS_in_Sec = 1000000;

// Configure an interrupt timer for tracking time until disconnect or deep sleep.
hw_timer_t *timer = NULL;
volatile int sleepCountdown = OPERATING_TIME_SECS;
void IRAM_ATTR onTimer() {
  sleepCountdown--;
}

// Declare Bluetooth service name, and characteristic for battery status.
// Reference: https://www.bluetooth.com/specifications/assigned-numbers/
BLEService batteryService("180F");
BLEUnsignedCharCharacteristic batteryLevelCharacteristic("2A19", BLERead); // 8-bit unsigned percent value 0 - 100, no decimal places.

// Configure Bluetooth and serial debugging.
void setup() {

  // Blink LED to indicate startup and leave it on. 
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i=0; i<3; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
  }

  // Initialize serial for debug output.
  Serial.begin(9600);

  // Initialize Bluetooth communication.
  Serial.println("Initializing Bluetooth communication.");
  if (!BLE.begin()) {
    Serial.println("Failed.");
    while (1); // Loop forever. This has the effect of leaving the LED on, indicating a failure.
  }

  // Set up Bluetooth service.
  Serial.println("Setting up battery service with characteristic for battery level.");
  BLE.setLocalName("ESP32DevkitV1");
  BLE.setAdvertisedService(batteryService);

  // Add battery level characteristic.
  batteryService.addCharacteristic(batteryLevelCharacteristic);
  batteryLevelCharacteristic.writeValue(100); // Make it 100% since it's wall powered.

  // Manufacturer Data field must include the company ID as the first two octets. Data follows.
  // It needs to be in little endian format, so 0x09A3, registered to Arduino SA is 0xA3, 0x09.
  // Company ID source: https://www.bluetooth.com/specifications/assigned-numbers/
  // Below is the Arduino company ID followed by "BATT:100%" in ASCII.
  byte data[11] = { 0xA3, 0x09, 0x42, 0x41, 0x54, 0x54, 0x3A, 0x31, 0x30, 0x30, 0x25};
  BLE.setManufacturerData(data, 11);

  // Make the service available.
  Serial.println("Advertising services.");
  BLE.addService(batteryService);
  BLE.setConnectable(true);
  BLE.advertise();

  // Start deep sleep timer to help mitigate self-heating of attached sensors.
  Serial.print("Countdown to deep sleep: ");
  Serial.println(OPERATING_TIME_SECS);
  timer = timerBegin(uS_in_Sec);  // Freq needs to match timerAlarm() below to be in units of seconds.
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, uS_in_Sec, true, 0);

  // Turn off LED to indicate startup has completed successfully.
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {

  // Wait for a connection from a central.
  BLEDevice central = BLE.central();

  // When a connection is made, activate LED and write address to serial for debug.
  if (central) {
    sleepCountdown = OPERATING_TIME_SECS;
    Serial.print("Countdown to deep sleep was reset: ");
    Serial.println(OPERATING_TIME_SECS);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print("Servicing connection from: ");
    Serial.println(central.address());

    while (central.connected()) {

      // If there were an actual battery, read its capacity and report with:
      // batteryLevelCharacteristic.writeValue(<batteryPercent>);
      // where <batteryPercent> is an unsigned char value between 0 and 100.

      // Delay between updates. (Don't make too long or connections start to timeout.)
      delay(500);
      if (sleepCountdown <= 0) {
        Serial.println("Forcing disconnect after time limit.");
        BLE.disconnect();
      }
    }

    // Turn off LED when connection is dropped. 
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("Connection terminated.");
  }

  // Deep sleep saves power and helps mitigate self-heating sensors.
  if (sleepCountdown <= 0) {
    Serial.print("Entering deep sleep. Countdown to wake-up: ");
    Serial.println(SLEEPING_TIME_SECS);
    Serial.flush();
    esp_sleep_enable_timer_wakeup((uint64_t) SLEEPING_TIME_SECS * uS_in_Sec);
    esp_deep_sleep_start();
  }
}
