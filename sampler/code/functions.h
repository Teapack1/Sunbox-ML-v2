#include <utility>


std::pair<int, int> led_controller(float controlValue){
      int warmBrightness, coldBrightness;
       
      if (controlValue <= 0.5) {
      warmBrightness = 255;
      coldBrightness = int((controlValue * 2) * 255); // Map control value for Cold LED (0 to 0.5 -> 0 to 255)
    } else {
      warmBrightness = int((1 - (controlValue - 0.5) * 2) * 255); // Map control value for Warm LED (0.5 to 1 -> 255 to 0)
      coldBrightness = 255;
    }
    return std::make_pair(warmBrightness, coldBrightness);
}



void setup_led(){
    // Setup channel 0
    ledcSetup(LEDC_CHANNEL_1, LEDC_BASE_FREQ, LEDC_TIMER_8_BIT);
    ledcAttachPin(warmPin, LEDC_CHANNEL_1);

    // Setup channel 1
    ledcSetup(LEDC_CHANNEL_2, LEDC_BASE_FREQ, LEDC_TIMER_8_BIT);
    ledcAttachPin(coldPin, LEDC_CHANNEL_2);
}

void create_ap(bool AP, const char* ssid, const char* pass, Adafruit_SSD1306& display){

  if (AP == true){


    Serial.println("Setting up AP...");
    WiFi.softAP(ssid, pass);
    
    IPAddress IP = WiFi.softAPIP();
    
    Serial.println("AP setup succesfully!");
    Serial.println("");
    Serial.print("AP IP address: ");
    Serial.println(IP);

        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println("AP setup successfully!");
        display.println("");
        display.print("AP IP address: ");
        display.println(IP);
        display.display();
    delay(500);
    
} else if (AP == false){
        display.clearDisplay();
        display.setTextSize(1);   
      display.setCursor(0, 0);
        display.setTextColor(BLACK, WHITE);
     display.println(F("connecting to AP..."));
    display.display();
     
    WiFi.begin(ssid, pass);
    uint8_t idx = 0;
    while (WiFi.status() != WL_CONNECTED){
            if (idx<max(display.width(),display.height())/2){
        idx+=2;
      display.drawCircle(display.width()/2, display.height()/2, idx, SSD1306_WHITE);
      display.display();
      delay(1);
      
      } else if (idx>max(display.width(),display.height())/2){
        idx-=2;
      display.drawCircle(display.width()/2, display.height()/2, idx, SSD1306_WHITE);
      display.display();
      delay(1);
      }
      
      delay(1000);     
      Serial.print(".");
       
   }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("Local IP address: ");
    Serial.println(WiFi.localIP());

      for(uint8_t i=idx; i<max(display.width(),display.height())/2; i+=2) {
        display.drawCircle(display.width()/2, display.height()/2, i, SSD1306_WHITE);
        display.display();
        delay(1);
        idx = i;
      }

      for(uint8_t i=idx; i>0; i-=2) {
      display.drawCircle(display.width()/2, display.height()/2, i, SSD1306_BLACK);
      display.display();
      delay(1);
    }
    
        display.clearDisplay();
        display.setTextSize(1);   
      display.setCursor(0, 0);
        display.setTextColor(WHITE);
     display.println(F("WiFi Connected!"));
     display.println(WiFi.localIP());
    display.display();
    delay(2000);   
 }
}

void print_colors(int ct, int l, int r, int g, int b, int c){
    Serial.print("Color Temp: "); Serial.print(ct, DEC); Serial.print(" K - ");
    Serial.print("Lux: "); Serial.print(l, DEC); Serial.print(" - ");
    Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
    Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
    Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
    Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
    Serial.println(" ");
}

char* ip_setup(Adafruit_SSD1306& display, const int touchPin, const int longPressTime, const int acceptTime, const char* ip_prefix) {

unsigned long lastBlinkTime = 0;
unsigned long lastChangeTime = 0;
int currentDigit = 0;
bool blinkState = false;
int digits[4] = {0, -1, -1, -1}; 

static char websockets_server_host[16];

int lastTouchState = LOW;
  while (true) {
    int touchState = digitalRead(touchPin);
    if (touchState == HIGH && lastTouchState == LOW) {
      lastChangeTime = millis();
      digits[currentDigit]++;
      if (digits[currentDigit] > 9) {
        digits[currentDigit] = -1;
      }
    }
    else if (touchState == HIGH && (millis() - lastChangeTime >= longPressTime)) {
      lastChangeTime = millis();
      currentDigit++;
      if (currentDigit > 3) {
        currentDigit = 0;
      }
    }
    else if (millis() - lastChangeTime >= acceptTime) {
      if (digits[1] == -1) {
        sprintf(websockets_server_host, "%s%d", ip_prefix, digits[0]);
      } else if (digits[2] == -1) {
        sprintf(websockets_server_host, "%s%d%d", ip_prefix, digits[0], digits[1]);
      } else {
        sprintf(websockets_server_host, "%s%d%d%d", ip_prefix, digits[0], digits[1], digits[2]);
      }
      // websockets_server_host now contains the IP address
      break;
    }

    // Display the current IP address on the screen
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Server IP:");
    display.print(ip_prefix);
    for (int i = 0; i < 3; i++) {
      if (i == currentDigit && blinkState) {
        display.print(" ");
      } else if (digits[i] == -1) {
        display.print(" ");
      } else {
        display.print(digits[i]);
      }
    }
    display.display();

    // Toggle blinkState every 500ms
    if (millis() - lastBlinkTime >= 500) {
      blinkState = !blinkState;
      lastBlinkTime = millis();
    }

    lastTouchState = touchState;
  }
  return websockets_server_host;
}
