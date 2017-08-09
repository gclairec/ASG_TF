#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define SOILMOISTPIN 2

class ASG {
 public: 
  void readSensorValues();
  float monitorStage(float, int);
};

class Sensor : public ASG {
 public:
  float getSensorValues();
};

class am2302 : public Sensor {
 public:
  int checkDHTStatus(float, float);
};

class Temperature : public am2302 {
 public:
  float tempValue;
  float tempMaxValue = 29.0;
  float getSensorValues(bool);
};

class Humidity : public am2302 {
 public:
  float humidValue;
  float getSensorValues(int);
};

class Moisture : public Sensor {
 public:
  float moistureValue;
  float getSensorValues(int);
};

enum sensorChoice
{
  MOISTURE = 1,
  TEMPERATURE = 2,
  HUMIDITY = 3
};

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
ASG asg;
am2302 dhtSensor;
Moisture moisture;
Humidity humidity;
Temperature temperature;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  dht.begin();
}

void loop() {
 asg.readSensorValues();
 delay(1000);
}


void ASG::readSensorValues()
{
 float temp_moisture;
 float temp_temperature;
 float temp_humidity;
 
 moisture.moistureValue = moisture.getSensorValues(SOILMOISTPIN);
 temp_moisture = moisture.moistureValue;
 monitorStage(temp_moisture, 1);
    
 temperature.tempValue = temperature.getSensorValues(true);
 temp_temperature = temperature.tempValue;
 monitorStage(temp_temperature, 2);
    
 humidity.humidValue = humidity.getSensorValues(1);
 temp_humidity = humidity.humidValue;
 monitorStage(temp_humidity, 3);
 dhtSensor.checkDHTStatus(temp_humidity, temp_temperature); 
}

float Moisture::getSensorValues(int smPin)
{
 float soilMoistureRaw;
 soilMoistureRaw = analogRead(smPin)*(3.3/1024);
 delay(1000);
 
 return soilMoistureRaw;
}

float Temperature::getSensorValues(bool temp)
{
 float newTemp = dht.readTemperature();
 delay(1000);
 
  return newTemp;
}

float Humidity::getSensorValues(int humid)
{
 float newHumidity = dht.readHumidity();
 delay(1000);
 
  return newHumidity;
}

int am2302::checkDHTStatus(float humidVal, float tempVal)
{
 if (isnan(humidVal) || isnan(tempVal)) 
  {
  Serial.println("Failed to read from DHT sensor!");
  Serial.println("---------------------");
  return 0;
 }else
  return 1;
}


float ASG::monitorStage(float sensorRaw, int sensorChoice)
{
 float sensorVal;
 float soilMoisture;
 float soilMoistureRaw;
 int alertFlag = 0;
 
 switch(sensorChoice)
 {
  case MOISTURE: if (sensorRaw < 1.1)
            {
            Serial.println(sensorRaw);
            soilMoisture = (10 * sensorRaw) - 1;
            alertFlag = 11;
            Serial.println("Alert: Plant drowning");
            Serial.println("---------------------");
           }else if (sensorRaw < 1.3)
           {
            soilMoisture = (25 * sensorRaw) - 17.5;
            Serial.println("Well watered");
            Serial.println("---------------------");
           }else if (sensorRaw < 1.82)
           {
            soilMoisture = (48.08 * sensorRaw) - 47.5;
            Serial.println("Warning: Probably as low as you'd want");
            Serial.println("---------------------");
           } 
            else if (sensorRaw < 2.2)
            {
              soilMoisture = (26.32 * sensorRaw) - 7.89;
              alertFlag = 11;
              Serial.println("Alert: Really dry soil");
              Serial.println("---------------------");
            }else
            {
             soilMoisture = (62.5 * sensorRaw) - 87.5;
             alertFlag = 11;
             Serial.println("Alert: No Moisture detected");
             Serial.println("---------------------");
            }

            break;

  case TEMPERATURE: Serial.print("Temperature: "); 
          Serial.print(sensorRaw);
          Serial.println(" *C ");

          if(sensorRaw > temperature.tempMaxValue)
           {
            alertFlag = 21;
           }
           break;
  case HUMIDITY: Serial.print("Humidity: "); 
          Serial.print(sensorRaw);
          Serial.print("\n");
          break;
  default : break;
 }

  return sensorVal;
}

