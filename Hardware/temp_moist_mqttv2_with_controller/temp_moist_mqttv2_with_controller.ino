#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define wifi_ssid "Touch-fire"
#define wifi_password "C3bui5th3b35t1!"

#define mqtt_server "128.199.158.11"
#define mqtt_user "sensor-service"
#define mqtt_password "12345"

#define humidity_topic "sensor/humidity"
#define temperature_topic "sensor/temperature"
#define moisture_topic "sensor/moisture"

const int soilMoistPin = A0;
const int solenoidPin = 3;
const int fanPin = 4;
const int wateringTime = 120000; //Set the watering time (2 min for a start)
const float wateringThreshold = 15; //Value below which the garden gets watered
const float tempThreshold = 32;
bool watering = false;
bool wateredToday = false;

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

float soilMoistureRaw = 0;
float soilMoisture = 0;
float humidity = 0;
float airTemp = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server,8883);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
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

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) &&
         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

long lastMsg = 0;
float temp = 0.0;
float hum = 0.0;
float moisture = 0.0;
float diff = 0.1;



void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
  delay(2000); 
  //Collect sensor data
  soilMoistureRaw = analogRead(soilMoistPin)*(3.3/1024);
  delay(20);
  
  if (soilMoistureRaw < 1.1)
   {
   Serial.println(soilMoistureRaw);
   soilMoisture = (10 * soilMoistureRaw) - 1;
   Serial.println(soilMoisture);
   Serial.println("Cup of water");
   Serial.println("---------------------");
  }
  else if (soilMoistureRaw < 1.3)
   {
    Serial.println(soilMoistureRaw);
    soilMoisture = (25 * soilMoistureRaw) - 17.5;
    Serial.println(soilMoisture);
    Serial.println("Well watered");
    Serial.println("---------------------");
  }  
  else if (soilMoistureRaw < 1.82)
   {
    Serial.println(soilMoistureRaw);
    soilMoisture = (48.08 * soilMoistureRaw) - 47.5;
    Serial.println(soilMoisture);
    Serial.println("Probably as low as you'd want");
    Serial.println("---------------------");
  } 
  else if (soilMoistureRaw < 2.2)
   {
    Serial.println(soilMoistureRaw);
    soilMoisture = (26.32 * soilMoistureRaw) - 7.89;
    Serial.println(soilMoisture);
    Serial.println("Really dry soil");
    Serial.println("---------------------");
  }
  else
   {
    Serial.println(soilMoistureRaw);
    soilMoisture = (62.5 * soilMoistureRaw) - 87.5;
    Serial.println(soilMoisture);
    Serial.println("Air");
    Serial.println("---------------------");
  }

  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float newHumidity = dht.readHumidity();
  // Read temperature as Celsius
  float newTemp = dht.readTemperature();

  if (isnan(newHumidity) || isnan(newTemp)) {
    Serial.println("Failed to read from DHT sensor!");
    Serial.println("---------------------");
    return;
  }

  Serial.print("Humidity: "); 
  Serial.print(newHumidity);
  Serial.print(" %\t");
  Serial.print("Temperature: "); 
  Serial.print(newTemp);
  Serial.println(" *C ");
  Serial.println("---------------------");

  if (checkBound(newTemp, temp, diff)) {
      temp = newTemp;
      Serial.print("New temperature:");
      Serial.println(String(temp).c_str());
      client.publish(temperature_topic, String(temp).c_str(), true);
    }
    
  if (checkBound(newHumidity, hum, diff)) {
      hum = newHumidity;
      Serial.print("New humidity:");
      Serial.println(String(hum).c_str());
      client.publish(humidity_topic, String(hum).c_str(), true);
    }

  if (checkBound(soilMoisture, moisture, diff)) {
      moisture = soilMoisture;
      Serial.print("New soil moisture:");
      Serial.println(String(moisture).c_str());
      client.publish(moisture_topic, String(moisture).c_str(), true);
    }
    
  }

  if (temp > tempThreshold) {
    //water the garden
    digitalWrite(fanPin, HIGH);
  }
  digitalWrite(fanPin, LOW);
  
   if ((soilMoisture < wateringThreshold) && (wateredToday = false)) {
    //water the garden
    digitalWrite(solenoidPin, HIGH);
    delay(wateringTime);
    digitalWrite(solenoidPin, LOW);

    //record that we're watering
    #if ECHO_TO_SERIAL
        Serial.print("TRUE");
    #endif
      
        wateredToday = true;
      }
      else {
        
    #if ECHO_TO_SERIAL
        Serial.print("FALSE");
    #endif
      }
      
    #if ECHO_TO_SERIAL
      Serial.println();
    #endif
      delay(50);
}
