#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Wi-Fi Credentials
// const char* SSID = "Chaokuaychakunglao";
// const char* PASSWORD = "horwangwifi";
const char* SSID = "a";
const char* PASSWORD = "forminecraft";

// MQTT Broker Details
const char* MQTT_SERVER = "broker.netpie.io";
const int MQTT_PORT = 1883;
const char* MQTT_CLIENT = "6ac7ef6c-fb48-402e-a8ac-56b6360fbf41";
const char* MQTT_USERNAME = "Hu7aTTD367RT94tUfr8tzyG6HqGfzbT7";
const char* MQTT_PASSWORD = "p8AzGk5RAVUjJQGw88YzaM8kATx59h3x";

// MQTT Topics222---
const char* TEMP_TOPIC = "@msg/temp";
const char* LDR_TOPIC = "@msg/ldr";
const char* SOIL_TOPIC = "@msg/soil";

// Pin Definitions
#define DHTPIN 13
#define LDRPIN 34
#define SOILPIN 35

// Sensor Configurations
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

// Soil Moisture Conversion
int soilMoistureToPercentage(int soilValue) {
  int percentValue = soilValue*100.0/4095.0;
  return percentValue;
}

void connectWiFi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT broker...");
    if (client.connect(MQTT_CLIENT, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("MQTT Connected!");
      client.subscribe(TEMP_TOPIC);
      client.subscribe(LDR_TOPIC);
      client.subscribe(SOIL_TOPIC);
    } else {
      Serial.print("Failed. State: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void publishData(const char* topic, const String& payload) {
  if (client.publish(topic, payload.c_str())) {
    Serial.println("Published to " + String(topic) + ": " + payload);
  } else {
    Serial.println("Publish to " + String(topic) + " failed!");
  }
}

void setup() {
  Serial.begin(115200);
  connectWiFi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  dht.begin();
  delay(2000);
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  // Temperature & Humidity
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  Serial.print(temperature);
  Serial.print(humidity);

  if (!isnan(temperature) && !isnan(humidity)) {
    String tempPayload = "{\"temperature\": " + String(temperature) + 
                         ", \"humidity\": " + String(humidity) + "}";
    publishData(TEMP_TOPIC, tempPayload);
  }

  // LDR
  int ldrValue = analogRead(LDRPIN);
  String ldrPayload = "{\"ldr_value\": " + String(ldrValue) + "}";
  publishData(LDR_TOPIC, ldrPayload);

  // Soil Moisture
  int soilValue = analogRead(SOILPIN);
  int soilPercentage = soilMoistureToPercentage(soilValue);
  String soilPayload = "{\"soil_moisture\": " + String(soilPercentage) + 
                       ", \"raw_value\": " + String(soilValue) + "}";
  publishData(SOIL_TOPIC, soilPayload);

  delay(10000);  // 10-second interval
}