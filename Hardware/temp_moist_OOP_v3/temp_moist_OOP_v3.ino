#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define DHTPIN 6     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define SOILMOISTPIN 2
#define TIP120PIN 16
#define PLANT_DROWNING 1.1
#define WELL_WATERED 1.3
#define LOWEST_ALLOWABLE 1.82
#define REALLY_DRY 2.2
#define SENSOR_TEMPERATURE 1
#define SENSOR_HUMIDITY 2
#define HIGH_RANGE 25.0
#define MOISTURE_HI_RANGE 1.1
#define MOISTURE_LO_RANGE 1.82
#define POWER_SUPPLY 3.3
#define MOISTURE_RANGE 1024
#define WIFI_SSID "Touch-fire"
#define WIFI_PASSWORD "C3bui5th3b35t1!"
#define MQTT_SERVER "128.199.158.11"
#define MQTT_USER "sensor-service"
#define MQTT_PASSWORD "12345"
#define HUMIDITY_TOPIC "sensor/humidity"
#define TEMPERATURE_TOPIC "sensor/temperature"
#define MOISTURE_TOPIC "sensor/moisture"

class Sensor {
 public:
  Sensor();
  ~Sensor();
  float sensorValue; 
 private:
  float sensorRange;
  int sensorState;
 public:
  virtual float getSensorValues() const = 0;
  int getSensorState();
};

class Regulator {
  public:
    Regulator();
//    ~Regulator();
  private:
    float regulationValue;
    int deviceState;
    int regulateCounter;
  public:
    boolean isValueRegulated();
    int getDeviceState();
    void setRegulateCounter(int counter);  
};

class MQTTAdapter{
  public:
    float subscribeTopic(String topic);
    void publishTopic(String topic, float data);
}; 

class ASG {
 private: 
  Sensor *pTemperature;
  Sensor *pHumidity;
  Sensor *pMoisture;
//  Regulator *pMoistureRegulator;
  Regulator *pam2302Regulator;
  MQTTAdapter *mqttAdapter;
  void setup_wifi();
  void reconnect();
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


//
//class MoistureRegulator : public Regulator {
//  private:
//    int sprinkleDuration;
//  public:
//    boolean isValueRegulated();
//    void setSprinklerDuration(int duration);
//};

class am2302Regulator : public Regulator {
  public:
    boolean isValueRegulated();
    am2302Regulator(int nType);
  private:
  int sensorType;
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
  Serial.begin(9600);
  delay(10);
  pinMode(TIP120PIN, OUTPUT);
  dht.begin();
}

void loop() {
  asg.monitorSystem();
}

Sensor::Sensor()
{
}

Sensor::~Sensor()
{
}

ASG::ASG()
{
  
  pTemperature = new am2302Sensor(SENSOR_TEMPERATURE);
  pHumidity = new am2302Sensor(SENSOR_HUMIDITY);
  pMoisture = new MoistureSensor();
//  pMoistureRegulator = new MoistureRegulator();
  pam2302Regulator = new am2302Regulator(SENSOR_TEMPERATURE);
//  pinMode(TIP120PIN, OUTPUT);
  setup_wifi();
  client.setServer(MQTT_SERVER,8883);
}


ASG::~ASG()
{
  delete pTemperature;
  delete pHumidity;
  delete pMoisture;
//  delete pMoistureRegulator;
  delete pam2302Regulator;
}

void ASG::setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void ASG::reconnect()
{
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void ASG::monitorSystem()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  float data;
  
  data = pTemperature->getSensorValues();
  if(data!=0)
    {
        
        digitalWrite(TIP120PIN, HIGH);
        delay(10000);
//      mqttAdapter->publishTopic(TEMPERATURE_TOPIC, data);
//        pMoistureRegulator->isValueRegulated();
    }
//    
//  data = pHumidity->getSensorValues();
//  if(data!=0)
//  {
//    mqttAdapter->publishTopic(HUMIDITY_TOPIC, data);
//  }
//  data = pMoisture->getSensorValues();
//  if(data!=0)
//  {
//    mqttAdapter->publishTopic(MOISTURE_TOPIC, data);
//  }
}

am2302Sensor::am2302Sensor(int nType): sensorType(nType)
{
}

float MoistureSensor::getSensorValues() const
{
 float newSensorValue;
 newSensorValue = analogRead(SOILMOISTPIN)*(POWER_SUPPLY/MOISTURE_RANGE);
 delay(1000);

 if(newSensorValue > MOISTURE_HI_RANGE || newSensorValue < MOISTURE_LO_RANGE)
  return newSensorValue;
 else
  return newSensorValue = 0;

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
 else
  return newSensorValue = 0;
}

am2302Regulator::am2302Regulator(int nType): sensorType(nType)
{
}

Regulator::Regulator()
{
 
}

//Regulator::~Regulator()
//{
// 
//}
boolean am2302Regulator::isValueRegulated()
{
  analogWrite(TIP120PIN, 255);
  delay(10000);  
  return true;
}

//void MQTTAdapter::publishTopic(String topic, float data)
//{
//  client.publish(topic, String(data).c_str());
//}


