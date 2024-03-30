// Display
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Websocket libs
#include <ArduinoWebsockets.h>
#include <WiFi.h>
#include <Wire.h>
#include "Adafruit_TCS34725.h"

//libs
#include "pins.h"
#include "functions.h"

#include <WiFi.h>
#include <esp_now.h>


//I2C
#define I2C_SDA 21 // SDA Connected to GPIO 14
#define I2C_SCL 22 // SCL Connected to GPIO 15

 // Counter for picture number
unsigned int pictureCount = 0;
const char* ssid = "mispot"; //Enter SSID
const char* password = "heslo123"; //Enter Password
const char* websockets_server_host;
const char* ip_prefix = "192.168.4."; //IP address of server

const uint16_t websockets_server_port = 8765; // Enter server port

// setup IP
const int longPressTime = 2000; // long press time in milliseconds
const int acceptTime = 10000; // time to accept input in milliseconds
unsigned long lastChangeTime = 0;
int currentDigit = 0;
int digits[3] = {0, 0, 0};

// Structure to represent a peer device
typedef struct Device {
  uint8_t mac[6]; // MAC address of the device
  unsigned long lastSeen; // Last seen timestamp
} Device;

Device knownDevices[20]; // Adjust size based on expected number of devices
int knownDevicesCount = 0;

int iter = 1;
using namespace websockets;
WebsocketsClient client;
TwoWire I2CSensors = TwoWire(0);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2CSensors, OLED_RESET);
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_300MS, TCS34725_GAIN_1X);


void connect_server(const char* websockets_server_host, const uint16_t websockets_server_port){
    Serial.println("Connecting to Websocket server.");
    char message[50];
    sprintf(message, "Connecting to IP address: %s", websockets_server_host);
    Serial.println(message);
    // try to connect to Websockets server
    int i = 0;
    
    display.clearDisplay();
    bool connected = false;
    while (!connected) {
                        display.setTextSize(1);      // Normal 1:1 pixel scale
                        display.setTextColor(SSD1306_WHITE); // Draw white text
                        display.setCursor(0, 0);     // Start at top-left corner
                        display.cp437(true);         // Use full 256 char 'Code Page 437' font
                        display.println(F("Connecting ot Server"));
                        char url[64];
                        sprintf(url, "%s:%d", websockets_server_host, websockets_server_port);
                        display.println(F(url));
                        display.display();

              connected = client.connect(websockets_server_host, websockets_server_port, "/");
                                     
                      display.drawLine(0, 20, i, display.height()-1, SSD1306_WHITE);
                      display.display(); // Update screen with each newly-drawn line
                      delay(1);

                      display.drawLine(0, 20, display.width()-1, i + 20, SSD1306_WHITE);
                      display.display();
                      delay(1);
                i+=4;
                
                
        if(connected) {
            Serial.println("Connected!");

                display.clearDisplay();
              
                display.setTextSize(2); // Draw 2X-scale text
                display.setTextColor(SSD1306_WHITE);
                display.setCursor(10, 0);
                display.println(F("Connected"));
                display.display();      // Show initial text

                delay(500);
            
                              
        } else {
            Serial.println("Not Connected!");
            delay(100); // Wait for a second before trying to connect again
        }
    }
}

         //--------------WEBSOCKET EVENTS----------------
                       
              void onEventsCallback(WebsocketsEvent event, String data) {
                  if(event == WebsocketsEvent::ConnectionOpened) {
                      Serial.println("Connnection Opened");
                  } else if(event == WebsocketsEvent::ConnectionClosed) {
                      Serial.println("Connnection Closed");
                  } else if(event == WebsocketsEvent::GotPing) {
                      Serial.println("Got a Ping!");
                  } else if(event == WebsocketsEvent::GotPong) {
                      Serial.println("Got a Pong!");
                  }
              }


void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA); // Set device to STA mode
    Serial.println("\nESP-NOW Brain Device Setup");

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(onDataRecv);

    I2CSensors.begin(I2C_SDA, I2C_SCL, 100000);
     pinMode(button, INPUT);
    
    // Disable brownout detector
   WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
     

     
      
    //display initalisation
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }else{
    Serial.println("Display OK!");
    display.clearDisplay();
    display.drawBitmap(0, 0, logo_bmp, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    display.display();
    delay(2000);
 }
    
 // int
  setup_led();
  
      // Initialize the camera
  Serial.print("Initializing the camera module...");
  configESPCamera();
  Serial.println("Camera OK!");

  // Initialize the RGB sensor
    if (tcs.begin(0x29, &I2CSensors)) {
    Serial.println("Color sensor OK!");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }

  
    // -------------------WEBSOCKET CALLBACK------------------------
    // This block of code is setting up a callback function that will be executed whenever a message is received from the WebSocket server.
      client.onMessage([&](WebsocketsMessage message){
          Serial.print("Got Message: ");
          Serial.println(message.data());
          String message_type, data;
          int separatorIndex = message.data().indexOf(':');
          if (separatorIndex != -1) {
              message_type = message.data().substring(0, separatorIndex);
              data = message.data().substring(separatorIndex + 1);
          }
  
      // If control_value recieved, setup the LED output
          if (message_type == "control_value") {
            display.clearDisplay();
               display.setTextSize(2);
              display.setCursor(5, 5);
             display.setTextColor(WHITE);
              display.println(F("Gathering"));
              display.setCursor(55, 30);
              display.println(iter);
             display.display();
            
              // Handle control value here
              float control_value = data.toFloat();
                std::pair<int, int> brightnesses = led_controller(control_value);
                int warmBrightness = brightnesses.first;
                int coldBrightness = brightnesses.second;
                ledcWrite(LEDC_CHANNEL_1, warmBrightness);
                 ledcWrite(LEDC_CHANNEL_2, coldBrightness);
                  Serial.print("Warm White level: ");
                  Serial.println(warmBrightness);
                  Serial.print("Cold White level: ");
                  Serial.println(coldBrightness);
                  
                   Serial.println("DIMMING THE LEDs!");
                   delay(300);
                      
                        // Take a picture
                        camera_fb_t * fb = esp_camera_fb_get();
                        if(!fb) {
                            Serial.println("Camera capture failed");
                            return;
                        } else {
                         Serial.println("Image taken.");
                        }
            
                          // Convert the image data to a base64 string
                          String base64Image = base64::encode((const uint8_t*)fb->buf, fb->len);
                          
                                                 
                        // Get RGBW values and send them over the WebSocket connection
                        uint16_t r, g, b, c, colorTemp, lux;
                        tcs.getRawData(&r, &g, &b, &c);
                        Serial.println("Color data read.");
                          colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);
                          lux = tcs.calculateLux(r, g, b);
                        Serial.println("Light props calculated.");
                            
                          //  print_colors(colorTemp, lux, r, g, b, c); // output of the sensor

                        if(client.available()) {
                             client.send("image:" + base64Image);
                              Serial.println("Image sent to the server.");
                              
                            String rgbw_values = String(r) + "," + String(g) + "," + String(b) + "," + String(c);
                            client.send("rgbw_values:" + rgbw_values);
                            String light_specs = String(colorTemp) + "," + String(lux);
                            client.send("light_specs:" + light_specs);
                            Serial.println("color data sent to the server.");
                            iter += 1;
                   }
                    esp_camera_fb_return(fb); 
                  
                  Serial.println("----------END--OF--THE--CALL----------");       
                  Serial.println(""); 
                    }
                });
          client.onEvent(onEventsCallback);
          create_ap(true, ssid, password, display);  // setup/connect to AP 
          websockets_server_host = ip_setup(display, button, longPressTime, acceptTime, ip_prefix); //setup server IP
    }


void loop() {
    // let the websockets client check for incoming messages
    if(client.available()) {
     
        client.poll();
       delay(100);
    } else {     
      connect_server(websockets_server_host, websockets_server_port);
      delay(400);
             display.clearDisplay();
              display.setTextSize(2);   
            display.setCursor(10, 0);
        display.setTextColor(BLACK, WHITE);
         display.println(F("Connected"));

               display.setTextSize(2);
              display.setCursor(55, 35);
             display.setTextColor(WHITE);
            display.println(F("Touch!"));
            display.display();
     
        display.startscrollleft(0x00, 0x0F);        
        delay(100); 
     
              while (digitalRead(button) == false) {
                }
         
        display.stopscroll();
        display.clearDisplay();
     
     }
}
