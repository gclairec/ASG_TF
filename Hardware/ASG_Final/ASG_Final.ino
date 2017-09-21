#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define DHTPIN D2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define SOILMOISTPIN A0
#define SOLENOIDPIN D1
#define FANPIN D4
#define TEMP_HIGH 25.0
#define TEMP_LOW 24.0
#define MOISTURE_HI_RANGE 1.3
#define MOISTURE_LO_RANGE 1.82
#define PLANT_DROWNING 1.1
#define WELL_WATERED 1.3
#define LOWEST_ALLOWABLE 1.82
#define REALLY_DRY 2.2
#define WATERINGTIME 10000 //10s delay watering time 
#define SENSOR_TEMPERATURE 1
#define SENSOR_HUMIDITY 2
#define POWER_SUPPLY 3.3
#define MOISTURE_RANGE 1024
#define UPLOAD_SPEED 115200
#define 10MS 10
#define 1S 1000


enum sensorChoice
{
  MOISTURE = 1,
  TEMPERATURE = 2,
  HUMIDITY = 3
};

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(UPLOAD_SPEED);
  delay(10MS);
  dht.begin();
  pinMode(FANPIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  monitorSystem();
}

float getSensorValues(int deviceType)
{
  float newSensorValue = 0;
  
  if(deviceType == TEMPERATURE)
  {
    float newSensorValue = dht.readTemperature();
    Serial.println("Temperature ");
    Serial.println(newSensorValue);
    delay(1S);
    if (isnan(newSensorValue)) 
    {
      Serial.println("Failed to read from DHT sensor!");
    }
    return newSensorValue;
  }else if(deviceType == MOISTURE)
  {
    float newSensorValue = 0;  
    newSensorValue = analogRead(SOILMOISTPIN)*(POWER_SUPPLY/MOISTURE_RANGE);
    Serial.println("Moisture ");
    Serial.println(newSensorValue);
    delay(1S);
    if (isnan(newSensorValue)) 
    {
      Serial.println("Failed to read from MOISTURE sensor!");
    }
    return newSensorValue;
  }else if(deviceType == HUMIDITY)
  {
    Serial.println("Humidity ");
    float newSensorValue = dht.readHumidity();
    Serial.println(newSensorValue);
    delay(1S);
    if (isnan(newSensorValue)) 
    {
      Serial.println("Failed to read from DHT sensor!");
    }
    return newSensorValue;
  }
   
}

void regulateSystem(int deviceType)
{
  if(deviceType == TEMPERATURE)
  {
    Serial.print("Fan ON");
    digitalWrite(FANPIN, HIGH);
  }else if(deviceType == MOISTURE)
  {
    Serial.print("Sprinkler ON");
    digitalWrite(SOLENOIDPIN, HIGH);
    delay(WATERINGTIME);
    digitalWrite(SOLENOIDPIN, LOW);
  } 
  
}

void monitorSystem()
{
  float tempSensorVal = 0;
  float humidSensorVal = 0;
  float moistSensorVal = 0;
  tempSensorVal = getSensorValues(TEMPERATURE);
  humidSensorVal = getSensorValues(HUMIDITY);
  moistSensorVal = getSensorValues(MOISTURE);

  if(!isValueRegulated(TEMPERATURE, tempSensorVal))
  {
    regulateSystem(TEMPERATURE);
  }else 
  {
    digitalWrite(FANPIN, LOW);
  }
  if(!isValueRegulated(MOISTURE, moistSensorVal))
  {
    regulateSystem(MOISTURE);
  }
  
}

boolean isValueRegulated(int deviceType, float sensorValue)
{
  if(MOISTURE)
  {
    if((sensorValue > MOISTURE_LO_RANGE) && (sensorValue > MOISTURE_HI_RANGE)) 
    {
      return false;
    }else
    {
      return true;
    }
  }else if(TEMPERATURE)
  {
    if((sensorValue > TEMP_HIGH) && (sensorValue > TEMP_LOW)) 
    {
      return false;
    }else
    {
      return true;
    }
  }
}




