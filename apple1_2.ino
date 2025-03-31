#include <ATT_LoRaWAN.h>
#include <MicrochipLoRaModem.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include "keys.h"
#include <PayloadBuilder.h>

#define debugSerial Serial
#define loraSerial Serial1
#define SERIAL_BAUD 57600
#define WATER_SENSOR 4        // water sensor connected to pin D4/D5
#define SEND_EVERY 600000    // Send data every 10 minutes



MicrochipLoRaModem modem(&loraSerial, &debugSerial);
ATTDevice device(&modem, &debugSerial, false, 7000);    // minimum time between 2 messages set at 7000 milliseconds
Adafruit_BME280 tph;
PayloadBuilder payload(device);

float temperature, humidity, totalTemperature;
float sumAverage = 0;
float dailyAverage = 0;
float totalRainHours;
float totalNoRainHours;
float previousRainHours = 0;
float rainStartTime = 0;
float rainStopTime = 0;
float totalRainDuration = 0;
float totalNoRainDuration = 0;
int16_t water;
int16_t venturia = 0;
int16_t readingCount = 0;
unsigned long lastSendTime = 0;


void setup() {

  debugSerial.begin(SERIAL_BAUD);
  while ((!debugSerial) && (millis() < 10000)) {} // Wait until the serial bus is available

  loraSerial.begin(modem.getDefaultBaudRate()); // Set baud rate of the serial connection to match the modem
  while ((!loraSerial) && (millis() < 10000)) {} // Wait until the serial bus is available

  while (!device.initABP(DEV_ADDR, APPSKEY, NWKSKEY))
    debugSerial.println("Ready to send data");

  debugSerial.println();
  debugSerial.println("-- Apple --");
  debugSerial.println();

  pinMode(WATER_SENSOR, INPUT);



  if (!tph.begin()) {
    debugSerial.println("Could not find a valid BME280 sensor, check wiring!");
  }

  delay(500);
}


void process()
{
  while (device.processQueue() > 0)
  {
    debugSerial.print("QueueCount: ");
    debugSerial.println(device.queueCount());
    delay(10000);
  }
}



void loop() {


  unsigned long currentTime = millis();

  // Send sensor data every 10 minutes
  if (currentTime - lastSendTime >= SEND_EVERY) {
    lastSendTime = currentTime;

    readSensors();
    calculateDailyTemp ();
    calculateRainPeriod ();
    if ( totalNoRainHours >= 6) {
      venturia_case();
    }
    displaySensorValues();
    sendSensorValues();
  }


}

void calculateRainPeriod() {


  if (water == 0) { // Check if water is equal to 0 (indicating rain)
    if (rainStartTime == 0) {
      // Record the start time of rain and initialize variables
      rainStartTime = millis();
      rainStopTime = 0;
      totalNoRainDuration = 0;
      totalRainHours = 0;
    } else {
      // Calculate total rain duration and hours if rain has been ongoing
      if (totalNoRainHours >= 8) {
        totalRainDuration = millis() - rainStartTime;
        totalRainHours = totalRainDuration / 3600000.0;
        previousRainHours = 0; 
        
      } else {
        // Adjust total rain hours with previous rain hours if rain duration is less than 8 hours
        totalRainDuration =  millis() - rainStartTime;
        totalRainHours = (totalRainDuration / 3600000.0) + previousRainHours;
      }
    }
  } else {
    //(Not raining)
    if (rainStopTime == 0) {
      // Record the stop time of rain and update variables
      rainStopTime = millis();
      rainStartTime = 0;
      totalNoRainHours = 0;
      previousRainHours = totalRainHours;
    } else {
      // Calculate total non-rain duration and hours if rain has stopped
      totalNoRainDuration = millis() - rainStopTime;
      totalNoRainHours = totalNoRainDuration / 3600000.0;
    }
  }
}




  void calculateDailyTemp () {

    
    totalTemperature += temperature;

    // Check if 144 temperature readings have been collected
    if (readingCount == 144) {
      // Calculate the average temperature for the day
      dailyAverage = totalTemperature / readingCount;
      sumAverage += dailyAverage;
      readingCount = 0; 
      totalTemperature = 0;
    } else {
      
      dailyAverage = 0;
      readingCount++;
    }

  }  

    void readSensors() {
      water = digitalRead(WATER_SENSOR);
      temperature = tph.readTemperature();
      humidity = tph.readHumidity();
      //    pressure = tph.readPressure() / 100.0;

    }





    void sendSensorValues() {



      payload.reset();
      payload.addNumber(dailyAverage);
      payload.addNumber(totalRainHours);
      payload.addNumber(temperature);
      payload.addNumber(humidity);
      payload.addNumber(sumAverage);
      payload.addInteger(venturia);
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
      debugSerial.print("UPOZORENJE 1: ");
      debugSerial.print(venturia);
      debugSerial.println(" hPa");
      
      debugSerial.print("broj: ");
      debugSerial.println(readingCount);
      debugSerial.print("dailysum: ");
      debugSerial.println(dailyAverage);
      debugSerial.print("suma srednjih vrednosti: ");
      debugSerial.println(sumAverage);
          
      debugSerial.println("Water_level: ");
      if (water) {
        debugSerial.print("Kisa  ne pada vec: ");
        debugSerial.println(totalNoRainDuration );
        debugSerial.print(" vreme u satima: ");
        debugSerial.println(totalNoRainHours);
        debugSerial.print(" vreme prethodnog padanja kise: ");
        debugSerial.println(previousRainHours);
        
      } else {
        debugSerial.print("Kisa  pada.");
        debugSerial.println(totalRainDuration );
        debugSerial.print(" vreme u satima: ");
        debugSerial.println(totalRainHours);
        debugSerial.print(" vreme prethodnog padanja kise: ");
        debugSerial.println(previousRainHours);
       
      }

      delay(200);                                           // A necessary delay so the serial output isn't cut off while outputting
    }





int16_t venturia_case() {

      if (totalRainHours >= 40.5 && totalRainHours <= 500) {
        if (temperature <= 1) {
          venturia = 1;
        }
      }
      else if (totalRainHours >= 34.7 && totalRainHours <= 500) {
        if (temperature <= 2 && temperature > 1) {
          venturia = 1;
        }
      }
      else if (totalRainHours >= 29.6 && totalRainHours <= 500) {
        if (temperature <= 3 && temperature > 2) {
          venturia = 1;
        }
      }
      else if (totalRainHours >= 21.2 && totalRainHours <= 500) {
        if (temperature <= 4 && temperature > 3) {
          venturia = 1;
        }
      }
      else if (totalRainHours >= 18 && totalRainHours <= 500) {
        if (temperature <= 5 && temperature > 4) {
          venturia = 1;
        }
      }
      else if (totalRainHours >= 15.4 && totalRainHours <= 500) {
        if (temperature <= 6 && temperature > 5) {
          venturia = 1;
        }
      }
      else if (totalRainHours >= 13.4 && totalRainHours <= 500) {
        if (temperature <= 7 && temperature > 6) {
          venturia = 1;
        }
      }
      else if (totalRainHours >= 12.2 && totalRainHours <= 500) {
        if (temperature <= 8 && temperature > 7) {
          venturia = 1;
        }
      }
      else if (totalRainHours >= 11 && totalRainHours <= 500) {
        if ((temperature <= 9 && temperature > 8) || (temperature <= 26 && temperature > 25)) {
          venturia = 1;
        }
      }
      else if (totalRainHours >= 9 && totalRainHours <= 500) {
        if (temperature <= 10 && temperature > 9) {
          venturia = 1;
        }
      }
      else if (totalRainHours >= 8 && totalRainHours <= 500) {
        if ((temperature <= 13 && temperature > 10) || (temperature <= 25 && temperature > 24)) {
          venturia = 1;
        }
      }
      else if (totalRainHours >= 7 && totalRainHours <= 500) {
        if (temperature <= 15 && temperature > 13) {
          venturia = 1;
        }
      }
      else if (totalRainHours >= 6 && totalRainHours <= 500) {
        if (temperature <= 24 && temperature > 15) {
          venturia = 1;
        }
      }
      else {
        venturia = 0;
      }

      return venturia;
    }
