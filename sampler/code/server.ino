#include <WiFi.h>
#include <esp_now.h>

// Structure to represent a peer device
typedef struct Device {
  uint8_t mac[6]; // MAC address of the device
  unsigned long lastSeen; // Last seen timestamp
} Device;

Device knownDevices[20]; // Adjust size based on expected number of devices
int knownDevicesCount = 0;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // Set device to STA mode
  Serial.println("\nESP-NOW Brain Device Setup");

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  static unsigned long lastSent = 0;
  if (millis() - lastSent > 500) {
    lastSent = millis();
    sendSensorData();
    printConnectedDevices();
  }
}

void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  // Check if device is already known
  bool known = false;
  for (int i = 0; i < knownDevicesCount; i++) {
    if (memcmp(knownDevices[i].mac, mac, 6) == 0) {
      knownDevices[i].lastSeen = millis(); // Update last seen
      known = true;
      break;
    }
  }
  
  if (!known && knownDevicesCount < 20) { // Adjust size based on your array size
    // Add new device
    memcpy(knownDevices[knownDevicesCount].mac, mac, 6);
    knownDevices[knownDevicesCount].lastSeen = millis();
    knownDevicesCount++;
  }
}

void sendSensorData() {
  int sensorValue = analogRead(34); // Read from an analog sensor on GPIO 34
  // Convert sensor value to a string to send
  char buffer[50];
  snprintf(buffer, sizeof(buffer), "Sensor Value: %d", sensorValue);
  esp_now_send(NULL, (uint8_t *)buffer, sizeof(buffer)); // Broadcast to all peers
}

void printConnectedDevices() {
  Serial.printf("Connected Devices: %d\n", knownDevicesCount);
  for (int i = 0; i < knownDevicesCount; i++) {
    Serial.printf("Device %d: ", i+1);
    for (int j = 0; j < 6; j++) {
      Serial.printf("%02X", knownDevices[i].mac[j]);
      if (j < 5) Serial.print(":");
    }
    Serial.println();
  }
}
