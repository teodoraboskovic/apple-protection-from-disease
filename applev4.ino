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


float temperature, humidity, pressure, water;
int prevButtonState = HIGH;                        // Initial state of the previous button state
unsigned long lastButtonCheckTime = 0;
unsigned long lastSendTime = 0;                   // Time of last data send
unsigned long buttonCheckInterval = 100;          // Button check interval in milliseconds

ABPCredentials credentials(DEVADDR, APPSKEY, NWKSKEY);
LoRaModem modem(loraSerial, debugSerial, credentials);
Adafruit_BME280 tph;
CborPayload payload;

void setup() {
    debugSerial.begin(SERIAL_BAUD);
    while ((!debugSerial) && (millis() < 10000)) {} // Wait until the serial bus is available

    loraSerial.begin(modem.getDefaultBaudRate()); // Set baud rate of the serial connection to match the modem
    while ((!loraSerial) && (millis() < 10000)) {} // Wait until the serial bus is available

    while (!modem.init()) { delay(1000); }
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


void loop() {
    unsigned long currentMillis = millis();
      

    // Check button state every 100 milliseconds
    if (currentMillis - lastButtonCheckTime >= buttonCheckInterval) {
        lastButtonCheckTime = currentMillis;
        

        pushButton();
    }

    // Send data every 10 minutes
    if (currentMillis - lastSendTime >= SEND_EVERY) {
        lastSendTime = currentMillis;
        
        readSensors();
        displaySensorValues();
        sendSensorValues();
    }
}

void readSensors() {
    water = digitalRead(WATER_SENSOR);
    temperature = tph.readTemperature();
    humidity = tph.readHumidity();
    pressure = tph.readPressure() / 100.0;
}

void sendSensorValues() {
    payload.reset();
    payload.set("1", water);
    payload.set("5", temperature);
    payload.set("11", humidity);
    payload.set("10", pressure);

    if (modem.send(payload)) {
        debugSerial.println("Data sent successfully!");
    } else {
        debugSerial.println("Failed to send data.");
    }
}
void pushButton() {
    int buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW && prevButtonState == HIGH) { // Button pressed
        debugSerial.println("Button press detected!");
       payload.reset();                                  // Resets the payload in case there's anything left
       payload.set("3", buttonState);                    // Sets the payload with our entity name "3" and the value of our button (true/1)
       modem.send(payload);                              // Sends the payload via LoRaWAN
       prevButtonState = true;             
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
