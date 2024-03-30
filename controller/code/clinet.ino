#include <WiFi.h>
#include <esp_now.h>

// Brain device's MAC address
// Replace it with the actual MAC address of your brain device
uint8_t brainMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Use broadcast address for simplicity

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // Set device to STA mode
  Serial.println("ESP-NOW Device Setup");

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, brainMacAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // Register for incoming messages
  esp_now_register_recv_cb(onDataRecv);

  // Send a check-in message to the brain device
  const char* message = "Hello, Brain! I'm here.";
  esp_now_send(brainMacAddress, (uint8_t *)message, strlen(message));
}

void loop() {
  // No need to do anything here
}

// Callback function for receiving data
void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  Serial.print("Received data: ");
  for (int i = 0; i < len; i++) {
    Serial.print((char)incomingData[i]);
  }
  Serial.println();
}
