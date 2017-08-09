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

class temperature : public am2302 {
 public:
  float tempValue;
  float tempMaxValue = 29.0;
  float getSensorValues(bool);
};

class humid : public am2302 {
 public:
   float humidValue;
  float getSensorValues(int);
};

class soilMoist : public Sensor {
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
am2302 dhtSensor;
soilMoist moist;
humid humi;
temperature temper;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  dht.begin();
}

void loop() {
  moist.readSensorValues();
 delay(1000);
}

void ASG::readSensorValues()
{
 float mst;
 float tmp;
 float hmd;
 moist.moistureValue = moist.getSensorValues(SOILMOISTPIN);
 mst = moist.moistureValue;
 monitorStage(mst, 1);
    
 temper.tempValue = temper.getSensorValues(true);
 tmp = temper.tempValue;
 monitorStage(tmp, 2);
    
 humi.humidValue = humi.getSensorValues(1);
 hmd=humi.humidValue;
 monitorStage(hmd, 3);
 dhtSensor.checkDHTStatus(hmd, tmp); 
}

float soilMoist::getSensorValues(int smPin)
{
 float soilMoistureRaw;
 soilMoistureRaw = analogRead(smPin)*(3.3/1024);
 delay(1000);
 
 return soilMoistureRaw;
}

float temperature::getSensorValues(bool temp)
{
 float newTemp = dht.readTemperature();
 delay(1000);
 
  return newTemp;
}

float humid::getSensorValues(int humid)
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

          if(sensorRaw > temper.tempMaxValue)
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

