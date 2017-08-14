#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define SOILMOISTPIN 2
#define PLANT_DROWNING 1.1
#define WELL_WATERED 1.3
#define LOWEST_ALLOWABLE 1.82
#define REALLY_DRY 2.2
#define SENSOR_TEMPERATURE 1
#define SENSOR_HUMIDITY 2
#define HIGH_RANGE 29
#define POWER_SUPPLY 3.3
#define MOISTURE_RANGE 1024

class Sensor {
 public:
  Sensor();
  ~Sensor(); 
 private:
  float sensorRange;
  int sensorState;
  float sensorValue;
 public:
  virtual float getSensorValues() const = 0;
  int getSensorState();
};

class Regulator {
  protected:
    Regulator();
    ~Regulator();
  private:
    float regulationValue;
    int deviceState;
    int regulateCounter;
  public:
    boolean isValueRegulated(int deviceType);
    int getDeviceState();
    void setRegulateCounter(int counter);  
};

//class MQTTAdapter{
//  public:
//    float subscribeTopic(string topic);
//    int publishTopic(string topic, float data);
//}; 

class ASG {
 private: 
  Sensor *pTemperature;
  Sensor *pHumidity;
  Sensor *pMoisture;
  Regulator *pMoistureRegulator;
  Regulator *pTemperatureRegulator;
 public:
  ASG();
  ~ASG();
  void autoShutdownSystem();
  void monitorSystem();
  void adjustDevice(int deviceSelector);
};

class am2302Sensor : public Sensor {
 public:
  float getSensorValues() const;
  int checkDHTStatus(float sensorVal);
  am2302Sensor(int nType);
  ~am2302Sensor();
 private:
  int sensorType;
};

class MoistureSensor : public Sensor {
 public:
  float getSensorValues() const;
};



class MoistureRegulator : public Regulator {
  private:
    int sprinkleDuration;
  public:
    boolean isValueRegulated(int deviceType);
    void setSprinklerDuration(int duration);
};

class TemperatureRegulator : public Regulator {
  public:
    boolean isValueRegulated(int deviceType);
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  dht.begin();
}

void loop() {
  asg.monitorSystem();
}

ASG::ASG()
{
  
  pTemperature = new am2302Sensor(SENSOR_TEMPERATURE);
  pHumidity = new am2302Sensor(SENSOR_HUMIDITY);
  pMoisture = new MoistureSensor();
}


ASG::~ASG()
{
  delete pTemperature;
  delete pHumidity;
  delete pMoisture;
}

Sensor::Sensor()
{
}

Sensor::~Sensor()
{
}


void ASG::monitorSystem()
{
  pTemperature->getSensorValues();
  pHumidity->getSensorValues();
}

am2302Sensor::am2302Sensor(int nType): sensorType(nType)
{
}

float MoistureSensor::getSensorValues() const
{
 float soilMoistureRaw;
 soilMoistureRaw = analogRead(SOILMOISTPIN)*(POWER_SUPPLY/MOISTURE_RANGE);
 delay(1000);
 
 return soilMoistureRaw;
}

int am2302Sensor::checkDHTStatus(float sensorVal)
{
 if (isnan(sensorVal)) 
  {
  Serial.println("Failed to read from DHT sensor!");
  return 0;
 }else
  return 1;
}

float am2302Sensor::getSensorValues() const
{
 float newSensorValue;
  
 if(sensorType ==  SENSOR_TEMPERATURE)
 {
  float newSensorValue = dht.readTemperature();
  Serial.println("Temperature ");
  Serial.println(newSensorValue);
  delay(1000);
 }else if (sensorType ==  SENSOR_HUMIDITY)
 {
  Serial.println("Humidity ");
  float newSensorValue = dht.readHumidity();
  Serial.println(newSensorValue);
  delay(1000);
 }

 if(newSensorValue > HIGH_RANGE)
  return newSensorValue;
}



