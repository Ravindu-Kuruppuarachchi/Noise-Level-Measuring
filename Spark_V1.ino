#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <math.h>

// Wi-Fi Settings
const char* ssid = "Ravindu's Galaxy A12 ";
const char* password = "smwd3183";

// Raspberry Pi IP and UDP Port
const char* pi_ip = "192.168.57.191";  // Replace with your Pi's IP
const unsigned int udp_port = 12345;

// Device Identification (UNIQUE PER ESP8266)
const char* device_id = "esp-noise-01"; // Change this for each device!
const int device_id_length = 12;        // Fixed length for Pi-side parsing

WiFiUDP udp;

// Audio Settings
const int sample_rate = 8000;          // Hz
const int samples_per_packet = 512;    // Samples per UDP packet
const int mic_analog_pin = A0;         // MAX4466 connected to A0

// MAX4466 specific settings
const float VCC = 3.3;                 // ESP8266 operating voltage
const float ADC_REF = VCC;             // ADC reference voltage
const int ADC_RESOLUTION = 1023;       // 10-bit ADC
const float QUIET_AMPLITUDE = 0.0002;  // Environment noise floor


void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("Device ID: %s\n", device_id);
  
  analogReference(EXTERNAL); // Use VCC as reference (3.3V)
}

float calculateDB(int16_t* samples, int count) {
  // Calculate RMS value
  long sum = 0;
  for (int i = 0; i < count; i++) {
    sum += (long)samples[i];
  }

  float average = sum /count;
  float voltage = (average / ADC_RESOLUTION) * ADC_REF;
  
  // Calculate dB SPL
  float voltageDiff = abs(voltage);
  
  float dB = 20.0 * log10(voltageDiff /QUIET_AMPLITUDE);  // Basic dB conversion
  
  // Clamp dB to realistic values (0-120 dB)
  dB = constrain(dB, 0, 120);
  return dB;
}

void loop() {
  static int16_t samples[samples_per_packet];
  
  // Collect samples
  for (int i = 0; i < samples_per_packet; i++) {
    int raw = analogRead(mic_analog_pin);
    samples[i] = abs(raw-512);  // Center at 0 (MAX4466 outputs Vcc/2 centered signal)
    delayMicroseconds(1000000 / sample_rate);
  }

  // Calculate sound intensity in dB
  float sound_level_db = calculateDB(samples, samples_per_packet);

  // Send via UDP
  udp.beginPacket(pi_ip, udp_port);
  // 1. Send device ID (fixed length)
  char padded_id[device_id_length] = {0};
  strncpy(padded_id, device_id, device_id_length-1);
  udp.write(padded_id, device_id_length);
  
  // 2. Send audio samples
  udp.write((uint8_t*)samples, sizeof(samples));
  
  udp.endPacket();

  Serial.printf("Sent %d samples, Sound Level: %.1f dB SPL\n", samples_per_packet, sound_level_db);
}