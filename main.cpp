#include <Arduino.h>
#include <ESP32Servo.h>
#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_wdt.h"

// --- Wi-Fi Credentials ---
const char* ssid = "Everything is";
const char* password = "coNNected";

// --- HiveMQ Public Broker Settings ---
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883; // Standard TCP port for Arduino clients

WiFiClient espClient;
PubSubClient client(espClient);

// --- Servo & LDR Configuration ---
Servo solarServo;  
const int servoPin = 18;  
const int ldrLeftPin = 34;   // Left LDR
const int ldrRightPin = 35;  // Right LDR
float currentAngle = 90.0;
unsigned long lastServoUpdate = 0;
const int servoInterval = 30; 

// --- DHT11 & Fan Configuration ---
#define DHTPIN 4        
#define DHTTYPE DHT11   
#define EN_PIN 27       // L298N ENA
#define IN1_PIN 25      // L298N IN1
#define IN2_PIN 26      // L298N IN2

DHT dht(DHTPIN, DHTTYPE);

// --- Potentiometer & Dual LED Controls ---
const int potPin = 32;          
const int greenLedPin = 16;     // Green LED (Fan ON)
const int redLedPin = 15;       // Red LED (Fan OFF)
const int onboardLedPin = 2;    

// Timers
unsigned long previousMillis = 0;
const long dhtInterval = 1000; 

float lastKnownTemp = 25.0;
float dynamicThreshold = 30.0;
bool fanShouldBeOn = false;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-M Mansif-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected to HiveMQ!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  rtc_wdt_protect_off();
  rtc_wdt_disable(); 

  Serial.begin(115200);
  
  dht.begin();
  solarServo.setPeriodHertz(50); 
  solarServo.attach(servoPin, 500, 2400); 
  solarServo.write((int)currentAngle);

  pinMode(EN_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(onboardLedPin, OUTPUT);

  digitalWrite(IN1_PIN, HIGH);
  digitalWrite(IN2_PIN, LOW);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();

 
  if (currentMillis - lastServoUpdate >= servoInterval) {
    lastServoUpdate = currentMillis;

    int leftValue = analogRead(ldrLeftPin);
    int rightValue = analogRead(ldrRightPin);
    int diff = rightValue - leftValue;
    
    if (abs(diff) > 45) {
      float targetAngle = currentAngle;
      if (diff > 0) {
        targetAngle = currentAngle + 2.0; 
      } else {
        targetAngle = currentAngle - 2.0; 
      }
      targetAngle = constrain(targetAngle, 50, 170);
      currentAngle = currentAngle + (targetAngle - currentAngle) * 0.2;
      solarServo.write((int)currentAngle);
    }
  }

  // 2. --- POTENTIOMETER & FAN/LED CHECK ---
  int potValue = analogRead(potPin); 
  dynamicThreshold = map(potValue, 0, 4095, 50, 20); 

  fanShouldBeOn = (lastKnownTemp > dynamicThreshold);

  if (fanShouldBeOn) {
    digitalWrite(EN_PIN, HIGH);       
    digitalWrite(greenLedPin, HIGH);  
    digitalWrite(redLedPin,     LOW);     
    digitalWrite(onboardLedPin, HIGH);
  } else {
    digitalWrite(EN_PIN, LOW);        
    digitalWrite(greenLedPin, LOW);   
    digitalWrite(redLedPin, HIGH);    
    digitalWrite(onboardLedPin, LOW);
  }

  // 3. --- NON-BLOCKING DHT11 & MQTT PUBLISH CHECK ---
  if (currentMillis - previousMillis >= dhtInterval) {
    previousMillis = currentMillis;

    float temp = dht.readTemperature(); 
    float hum = dht.readHumidity();     

    if (!isnan(temp) && !isnan(hum)) {
      lastKnownTemp = temp; 
      
      // Publish live telemetry data to HiveMQ broker topics
      client.publish("esp32/solar_tracker/temperature", String(temp).c_str());
      client.publish("esp32/solar_tracker/threshold", String(dynamicThreshold).c_str());
      client.publish("esp32/solar_tracker/fan_status", fanShouldBeOn ? "ON" : "OFF");

      Serial.print("Temp Published: ");
      Serial.print(temp);
      Serial.print("°C | Threshold: ");
      Serial.print(dynamicThreshold);
      Serial.print("°C | Fan: ");
      Serial.println(fanShouldBeOn ? "ON" : "OFF");
    } else {
      Serial.println("Failed to read from DHT sensor!");
    }
  }
}