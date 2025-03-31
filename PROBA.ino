/*    _   _ _ _____ _    _              _____     _ _     ___ ___  _  __
 *   /_\ | | |_   _| |_ (_)_ _  __ _ __|_   _|_ _| | |__ / __|   \| |/ /
 *  / _ \| | | | | | ' \| | ' \/ _` (_-< | |/ _` | | / / \__ \ |) | ' <
 * /_/ \_\_|_| |_| |_||_|_|_||_\__, /__/ |_|\__,_|_|_\_\ |___/___/|_|\_\
 *                             |___/
 *
 * Copyright 2020 AllThingsTalk
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * ------- About This Example -------
 * If you didn't read the guide on AllThingsTalk Knowledge Center,
 * please do so at: https://www.allthingstalk.com/faq/getting-started-with-the-lorawan-rapid-development-kit
 * 
 * When the button is pressed, a value "true" is sent to AllThingsTalk.
 * The asset name for the button is "3", which is the "Push Button" asset.
 */

#include <AllThingsTalk_LoRaWAN.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include "keys.h"

#define debugSerial Serial
#define loraSerial Serial1
#define SERIAL_BAUD 57600
#define WATER_SENSOR 20
#define SEND_EVERY 600000 // Send data every 10 minutes
#define BUTTON_PIN 4



Adafruit_BME280 tph;
float temperature, humidity, pressure, water;
int prevButtonState = HIGH;                        // Initial state of the previous button state
unsigned long lastButtonCheckTime = 0;
unsigned long lastSendTime = 0;                   // Time of last data send
unsigned long buttonCheckInterval = 500;          // Button check interval in milliseconds

void setup() {
  Serial.begin(57600);                             // Starts the serial output 
   debugSerial.println("Ready to send data");
    debugSerial.println();
    debugSerial.println("-- Apple --");

    pinMode(WATER_SENSOR, INPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Use internal pull-up resistor for button pin

    if (!tph.begin()) {
        debugSerial.println("Could not find a valid BME280 sensor, check wiring!");
    }
    

    delay(500);
}


void readSensors() {
    water = digitalRead(WATER_SENSOR);
    temperature = tph.readTemperature();
    humidity = tph.readHumidity();
    pressure = tph.readPressure() / 100.0;
}


void pushButton() {
    int buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW && prevButtonState == HIGH) { // Button pressed
        debugSerial.println("Button press detected!");
//       payload.reset();                                  // Resets the payload in case there's anything left
//      payload.set("3", buttonState);                    // Sets the payload with our entity name "3" and the value of our button (true/1)
//      modem.send(payload);                              // Sends the payload via LoRaWAN
//      prevButtonState = true;             
    }
    prevButtonState = buttonState; // Update the previous button state
}
void displaySensorValues() {                            
  debugSerial.println("-----------------------");       // A simple line to distinguish between old and new data easily
  
 
  debugSerial.print("Temperatura: ");
  debugSerial.print(temperature);
  debugSerial.println(" °C");

  debugSerial.print("Vlaznost: ");
  debugSerial.print(humidity);
  debugSerial.println(" %");

  debugSerial.print("Pritisak: ");
  debugSerial.print(pressure);
  debugSerial.println(" hPa");

  debugSerial.print("Water_level: ");
  if (water) {
        debugSerial.println("Kisa ne pada!");
      } else {
        debugSerial.println("Kisa  pada.");
      }
  
  delay(200);                                           // A necessary delay so the serial output isn't cut off while outputting
}

void loop() {
    unsigned long currentMillis = millis();
     

    // Check button state every 500 milliseconds
    if (currentMillis - lastButtonCheckTime >= buttonCheckInterval) {
        lastButtonCheckTime = currentMillis;
        

        pushButton();
    }

    // Send data every 10 minutes
    if (currentMillis - lastSendTime >= SEND_EVERY) {
        lastSendTime = currentMillis;
        
        readSensors();
        displaySensorValues();
//        sendSensorValues();
    }
}
