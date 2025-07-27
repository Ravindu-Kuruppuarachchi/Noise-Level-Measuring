#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// Wi-Fi Settings
const char* ssid = "Ravindu's Galaxy A12 ";
const char* password = "smwd3183";

// Raspberry Pi IP and UDP Port
const char* pi_ip = "192.168.57.191"; 
const unsigned int udp_port = 12345;

WiFiUDP udp;

// Audio Settings
const int sample_rate = 4000; // Hz
const int samples_per_packet = 128;

void setup() {
  Serial.begin(115200);
  delay(10);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // FIX: Properly print IP address
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
}

void loop() {
  static int16_t samples[samples_per_packet];
  
  // Collect samples
  for (int i = 0; i < samples_per_packet; i++) {
    samples[i] = analogRead(A0); // Read from microphone
    delayMicroseconds(1000000 / sample_rate); // Control sample rate
  }

  // Send via UDP
  udp.beginPacket(pi_ip, udp_port);
  udp.write((uint8_t*)samples, sizeof(samples));
  udp.endPacket();

  Serial.printf("Sent %d samples to %s:%d\n", samples_per_packet, pi_ip, udp_port);
}