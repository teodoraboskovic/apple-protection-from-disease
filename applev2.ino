#include <AllThingsTalk_LoRaWAN.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include "keys.h"


#define SERIAL_BAUD 57600
#define debugSerial Serial
#define loraSerial Serial1
#define WATER_SENSOR A0
#define SEND_EVERY 300 // Define SEND_EVERY here


float temperature, humidity, pressure, water;
int   buttonPin           = 4;                          // Pin number of the button
bool  prevButtonState     = false;                      // Initial state of the previous button state

ABPCredentials credentials(DEVADDR, APPSKEY, NWKSKEY);
LoRaModem modem(loraSerial,debugSerial, credentials);
Adafruit_BME280 tph;
CborPayload payload;                                   




void setup() {
  
  debugSerial.begin(SERIAL_BAUD);
  while((!debugSerial) && (millis()) < 10000){}  // wait until the serial bus is available

  loraSerial.begin(modem.getDefaultBaudRate());  // set baud rate of the serial connection to match the modem
  while((!loraSerial) && (millis()) < 10000){}   // wait until the serial bus is available

   while (!modem.init()) { delay(1000); } 
  debugSerial.println("Ready to send data");
  
  debugSerial.println();
  debugSerial.println("-- Apple --");
  debugSerial.println();
  pinMode(GROVEPWR, OUTPUT);  // turn on the power for the secondary row of grove connectors
  digitalWrite(GROVEPWR, HIGH);
  pinMode(WATER_SENSOR, INPUT); 
  pinMode(buttonPin, INPUT); 
  tph.begin();
}
void readSensors() {
  water = digitalRead(WATER_SENSOR);
  temperature = tph.readTemperature();
  humidity = tph.readHumidity();
  pressure = tph.readPressure() / 100.0;

}
  void sendSensorValues() {
  
  payload.reset();                                      // Reset the cbor payload (in case there's anything left from the previous one)
  payload.set("12", water);                             // Create a payload containing data from the Sound Sensor to be sent to asset name "12" (Loudness sensor asser on AllThingsTalk Maker)
  payload.set("5", temperature);                        // Create a payload containing data from the Temperature Sensor (TPH) to be sent to asset name "5" (Temperature sensor asset on AllThingsTalk Maker)
  payload.set("11", humidity);                          // Create a payload containing data from the Humidity Sensor (TPH) to be sent to asset name "11" (Humidity sensor asset on AllThingsTalk Maker)
  payload.set("10", pressure);                          // Create a payload containing data from the Pressure Sensor (TPH) to be sent to asset name "10" (Pressure sensor asset on AllThingsTalk Maker)
  
  
  if (modem.send(payload)) {
     debugSerial.println("Data sent successfully!");
  } else {
     debugSerial.println("Failed to send data.");
  }
}
  

void pushButton() {                                     // 
  bool buttonState = digitalRead(buttonPin);            // Read the status of the button and save it in 'buttonState'
  if (buttonState == true) {                            // If the button is pressed
    if (prevButtonState == false) {                     // If the button previously wasn't pressed. This way the code below won't be run multiple times if you press and keep pressing the button.
      debugSerial.println("Button press detected, now sending...");
      payload.reset();                                  // Resets the payload in case there's anything left
      payload.set("3", buttonState);                    // Sets the payload with our entity name "3" and the value of our button (true/1)
      modem.send(payload);                              // Sends the payload via LoRaWAN
      prevButtonState = true;                           // Sets the previous button state to pressed (so the 'if' statement above fails) 
    }
  } else { 
    prevButtonState = false;                            // If the button is not being pressed, set the value of previous button state to False
  }
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
  if ( water == LOW) {
        debugSerial.println("Kisa pada!");
      } else {
        debugSerial.println("Kisa ne pada.");
      }
  
  delay(200);                                           // A necessary delay so the serial output isn't cut off while outputting
}

void loop() {
    readSensors(); 
    displaySensorValues();                               
    sendSensorValues();     
    pushButton();    
    delay(SEND_EVERY*1000);                            
}
