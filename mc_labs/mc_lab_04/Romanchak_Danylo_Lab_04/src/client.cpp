#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

const char* mqtt_server = "11bd520f2b114b35919f020c3c751466.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "hivemq.webclient.1750282472107";
const char* mqtt_pass = "fi<B:F;3,G0jehkH12AE";

const int ledPin = D3;
const int buzzerPin = D8;

WiFiClientSecure espClient;
PubSubClient client(espClient);
WiFiManager wifiManager;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void initWiFi() {
  wifiManager.setConfigPortalTimeout(180);
  if(!wifiManager.autoConnect("ESP_Doorbell_Listener_AP")) {
    Serial.println("Failed to connect and hit timeout");
    ESP.reset();
    delay(1000);
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void initDisplay() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while(true);
  }
  display.clearDisplay();
  display.display();
}

void triggerDoorbellActions() {
  digitalWrite(ledPin, HIGH);
  tone(buzzerPin, 1000);

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println("RING!");
  display.display();
  delay(100);
  noTone(buzzerPin);
  digitalWrite(ledPin, LOW);

  display.clearDisplay();
  display.display();
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(message);

  if (String(topic) == "doorbell/status" && message == "pressed") {
    triggerDoorbellActions();
  }
}

void reconnectMqtt() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP_Doorbell_Listener", mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      client.subscribe("doorbell/status");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  noTone(buzzerPin);

  initDisplay();
  initWiFi();

  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(onMqttMessage);
}

void loop() {
  if (!client.connected()) {
    reconnectMqtt();
  }
  client.loop();
}
