#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi Configuration
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";

// MQTT Configuration
const char* mqtt_server = "RASPBERRY_PI_IP";
const int mqtt_port = 1883;
const char* raw_topic = "noise/raw/";
const char* mode_topic = "noise/mode/";  // For receiving mode commands

// Device Configuration
const char* device_id = "esp-noise-01";
const int sample_rate = 4000;
const int samples_per_packet = 128;

// Operation mode
String current_mode = "general";  // Default mode

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  String payloadStr;
  
  for (int i = 0; i < length; i++) {
    payloadStr += (char)payload[i];
  }

  // Check if this is a mode command for our device
  if (topicStr.equals(String(mode_topic) + device_id)) {
    if (payloadStr.equals("industrial") || payloadStr.equals("general")) {
      current_mode = payloadStr;
      Serial.println("Mode changed to: " + current_mode);
    }
  }
}

void setup_wifi() {
  // ... (same as previous WiFi setup code)
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(device_id)) {
      Serial.println("MQTT connected");
      // Subscribe to mode topic
      client.subscribe((String(mode_topic) + device_id).c_str());
    } else {
      delay(5000);
    }
  }
}

void publish_samples(int16_t* samples) {
  StaticJsonDocument<512> doc;
  doc["device_id"] = device_id;
  doc["sample_rate"] = sample_rate;
  doc["timestamp"] = millis();
  doc["mode"] = current_mode;  // Include current mode in payload
  
  JsonArray data = doc.createNestedArray("samples");
  for (int i = 0; i < samples_per_packet; i++) {
    data.add(samples[i]);
  }

  char payload[1024];
  size_t n = serializeJson(doc, payload);
  
  char topic[50];
  snprintf(topic, sizeof(topic), "%s%s", raw_topic, device_id);
  
  client.publish(topic, (uint8_t*)payload, n, true);
}

// ... (rest of the ESP code remains the same)
