
#include <Wire.h>
#include <ATT_LoRaWAN.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include "keys.h"
#include <PayloadBuilder.h>
#include <MicrochipLoRaModem.h>


#define debugSerial Serial
#define loraSerial Serial1
#define SERIAL_BAUD 57600
#define WATER_SENSOR 20
#define SEND_EVERY 60000 // Send data every 10 minutes



MicrochipLoRaModem modem(&loraSerial, &debugSerial);
ATTDevice device(&modem, &debugSerial, false, 7000);
Adafruit_BME280 tph;
PayloadBuilder payload(device);



float temperature;
float humidity;
float pressure;
float hoursWithoutRain = 0.00;
float hoursWithRain = 0.00;
int16_t water;

unsigned long lastSendTime = 0;
unsigned long rainStartTime = 0;
unsigned long noRainStartTime = 0;
unsigned long rainDuration = 0;
unsigned long noRainDuration = 0;



void setup() {
    debugSerial.begin(SERIAL_BAUD);
    debugSerial.begin(SERIAL_BAUD);
    while ((!debugSerial) && (millis() < 10000)) {} // Wait until the serial bus is available

    loraSerial.begin(modem.getDefaultBaudRate()); // Set baud rate of the serial connection to match the modem
    while ((!loraSerial) && (millis() < 10000)) {} // Wait until the serial bus is available

    while(!device.initABP(DEV_ADDR, APPSKEY, NWKSKEY))
    debugSerial.println("Ready to send data");
    debugSerial.println();
    debugSerial.println("-- Apple --");

    pinMode(WATER_SENSOR, INPUT);

    if (!tph.begin()) {
        debugSerial.println("Could not find a valid BME280 sensor, check wiring!");
    }
    
    delay(500);
}


void loop() {
    unsigned long currentTime = millis();

    // Check if it's time to send data
    if (currentTime - lastSendTime >= SEND_EVERY) {
        lastSendTime = currentTime; // Update last send time

        updateRainDuration();
        calculateRainHours();
        readSensors();
        displaySensorValues();
        //sendSensorValues();
        debugSerial.println(".....................");
        debugSerial.print("Odgođeno za: ");
        debugSerial.println(SEND_EVERY);
        debugSerial.println();

         lastSendTime = currentTime; // Update last send time
    }else{
      updateRainDuration();
       
    }
}
void updateRainDuration() {
    static bool wasRaining = false;
    unsigned long currentMillis = millis();
    bool isRaining = (digitalRead(WATER_SENSOR)== LOW);

    if (isRaining && !wasRaining) {
        rainStartTime = currentMillis;
        noRainStartTime = 0;
    } else if (!isRaining && wasRaining) {
        noRainStartTime = currentMillis;
        rainStartTime = 0;
    }

    if (isRaining) {
        rainDuration = currentMillis - rainStartTime;
        noRainDuration = 0;
       
    } else {
        noRainDuration = currentMillis - noRainStartTime;
        rainDuration = 0;
       
    }

    wasRaining = isRaining;
}

void calculateRainHours() {
    unsigned long currentMillis = millis();
    if (rainStartTime != 0) {
        hoursWithRain =rainDuration / 3600000.0; // Pretvori milisekunde u sate
    }
    if (noRainStartTime != 0) {
        hoursWithoutRain =noRainDuration/ 3600000.0; // Pretvori milisekunde u sate
    }
}


void readSensors() {
    water = digitalRead(WATER_SENSOR);
    temperature = tph.readTemperature();
    humidity = tph.readHumidity();
    pressure = tph.readPressure() / 100.0;
}

void process()
{
  while(device.processQueue() > 0)
  {
    debugSerial.print("QueueCount: ");
    debugSerial.println(device.queueCount());
    delay(10000);
  }
}

void sendSensorValues() {
 
  
  payload.reset();
  payload.addNumber(hoursWithoutRain);
  payload.addNumber(hoursWithRain);
  payload.addNumber(temperature);
  payload.addNumber(humidity);
  payload.addNumber(pressure);
  payload.addInteger(water);
  payload.addToQueue(false);
  process();
  
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

  if (water == 0) {
        
        debugSerial.print("Kiša počela da pada u: ");
        debugSerial.println(rainStartTime);
        debugSerial.print("Trajanje kiše: ");
        debugSerial.print(hoursWithRain, 2); // Ispis trajanja u satima sa dvije decimale
        debugSerial.println(" sati");
    } else {
        debugSerial.println("Nema kiše");
       
            debugSerial.print("Sušni period počeo u: ");
            debugSerial.println(noRainStartTime);
            debugSerial.print("Trajanje sušnog perioda: ");
            debugSerial.print(hoursWithoutRain, 2); // Ispis trajanja u satima sa dvije decimale
            debugSerial.println(" sati");
        
    }

  
  delay(200);                                           // A necessary delay so the serial output isn't cut off while outputting
}
