#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

const char* mqtt_server = "11bd520f2b114b35919f020c3c751466.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_username = "hivemq.webclient.1750282472107";
const char* mqtt_password = "fi<B:F;3,G0jehkH12AE";

WiFiClientSecure secureClient;
PubSubClient client(secureClient);
WiFiManager wifiManager;

const int buttonPin = D5;

bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void setup_wifi() {
    wifiManager.setConfigPortalTimeout(180);
    if (!wifiManager.autoConnect("ESP_Doorbell_AP")) {
        Serial.println("Failed to connect and hit timeout");
        ESP.reset();
        delay(1000);
    }

    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
    while (!client.connected()) {
        Serial.println("Attempting MQTT connection...");

        if (client.connect("ESP_Doorbell_Button", mqtt_username, mqtt_password)) {
            Serial.println("Connected to MQTT broker");
        } else {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(buttonPin, INPUT_PULLUP);

    Serial.println("Starting setup");

    setup_wifi();
    secureClient.setInsecure();

    client.setServer(mqtt_server, mqtt_port);

    Serial.println("Setup complete");
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    int reading = digitalRead(buttonPin);
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        Serial.println(lastButtonState);
        if (reading == LOW) {
            Serial.println("Button pressed, publishing MQTT message");
            client.publish("doorbell/status", "pressed");
        }
    }
    lastButtonState = reading;
}
