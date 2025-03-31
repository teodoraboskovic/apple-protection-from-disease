#include <AllThingsTalk_LoRaWAN.h>
#include "keys.h"
#include <Wire.h>
#include "Adafruit_BME280.h"

#define WaterSensorPin A0
#define GROVEPWR       // Define GROVEPWR here
#define SEND_EVERY 300 // Define SEND_EVERY here

bool prevButtonState = false;
float temperature, humidity, pressure, water;
unsigned long waterSensorStartTime = 0;
unsigned long waterSensorDuration = 0;

ABPCredentials credentials(DEVADDR, APPSKEY, NWKSKEY);
LoRaModem modem(Serial1, Serial, credentials);
Adafruit_BME280 tph;
CborPayload payload;                                   




void setup() {
  Serial.begin(57600);
  while ((!Serial) && (millis()) < 10000) {}
  
  while (!modem.init()) {
    delay(1000);
  }

  pinMode(GROVEPWR, OUTPUT);
  digitalWrite(GROVEPWR, HIGH);
  pinMode(WaterSensorPin, INPUT);
  tph.begin();
}

void loop() {
  readSensors();
  displaySensorValues();
  sendSensorValues();

  delay(SEND_EVERY);
}

void readSensors() {
  water = analogRead(WaterSensorPin);
  temperature = tph.readTemperature();
  humidity = tph.readHumidity();
  pressure = tph.readPressure() / 100.0;
}

void displaySensorValues() {
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");

  Serial.print("Water Level: ");
  Serial.print(water);
  Serial.println(" ");
}

  void sendSensorValues() {
   payload.reset();  
  payload.addFloat("temperature", temperature);
  payload.addFloat("humidity", humidity);
  payload.addFloat("pressure", pressure);
  payload.addInteger("water_level", water);
   sendWarning(temperature, getCurrentHour());

  // Send the payload over LoRaWAN
  if (modem.send(&payload)) {
    Serial.println("Data sent successfully!");
  } else {
    Serial.println("Failed to send data.");
  }
}
}


void sendWarning(float temperature, int hour) {
  // ovo sam gledala tabelu 
  if ((hour >= 1 && hour <= 5 && temperature > 35.0) ||
      (hour >= 6 && hour <= 10 && temperature > 20.0) ||
      (hour >= 11 && hour <= 15 && temperature > 10.0) ||
      (hour >= 16 && hour <= 25 && temperature > 5.0) ||
      (hour >= 26 && temperature > 10.0)) {
  
    Payload warningPayload;

    // Add warning data to the payload
    warningPayload.addString("warning", "High temperature and rain!");

    
    if (modem.send(&warningPayload)) {
      Serial.println("Warning sent successfully!");
    } else {
      Serial.println("No warning.");
    }
  }
}

unsigned long waterSensorStartTime = 0;
unsigned long waterSensorDuration = 0;

void updateWaterSensor() {
  
  if (digitalRead(WaterSensorPin) == HIGH) {
    if (waterSensorStartTime == 0) {
      // ako senzor pre toga nije bio kvasan
      waterSensorStartTime = millis();
    }

    // ako kisa pada vec neko vreme
    waterSensorDuration = millis() - waterSensorStartTime;
  } else {
    // ako nema vise vode na senzoru
    waterSensorStartTime = 0;
    waterSensorDuration = 0;
  }
}

int getCurrentHour() {
  // 
  // ovo je da pretvori milisekunde u sate 
  int currentHour = waterSensorDuration / (60 * 60 * 1000); 
  return currentHour;
}

void sendSensorValues() {
  readSensors();
  displaySensorValues();
  updateWaterSensor(); //
  sendWarning(temperature, getCurrentHour()); 
  delay(SEND_EVERY);
}
