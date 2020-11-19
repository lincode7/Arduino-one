#include <Adafruit_NeoPixel.h>

#define PIN 0
#define MAX_LED 24

Adafruit_NeoPixel strip = Adafruit_NeoPixel( MAX_LED, PIN, NEO_RGB + NEO_KHZ800 );

#define Sensor 2  //Input Pin

int pre = 0, cur = 0;
bool people_statue = false;  //人是否在检测区

void setup() {

  Serial.begin(9600);
  // initialize the LCD
  pinMode (Sensor, INPUT);  //Define Pin as input
  pinMode(13, OUTPUT);
  strip.begin();           //初始化Adafruit_NeoPixel
  Serial.println("Waiting for motion");
}

void loop() {
  if (cur = digitalRead(2))
  {
    if (pre == 0 && cur == 1) {  //人区域内状态出现变化
      people_statue = !people_statue;
      if (people_statue) {
        Serial.println("People are approaching.");
        LED_Blink();
      }
      else {
        Serial.println("People are leaving.");
        LED_Blowout();
      }
    }
    digitalWrite(13, LOW);
    pre = cur;
  }
  else
  {
    digitalWrite(13, HIGH);
    Serial.println("No montion");
    pre = cur;
    delay(1000);
  }
}

void LED_Blink() {
  strip.clear();
  strip.setBrightness(255);
  strip.fill(strip.Color(166,255,50));
  strip.show();
}
void LED_Blowout() {
  strip.clear();
  strip.setBrightness(0);
  strip.show();
}
void LED_breath(int wait) {
  int brightness;
  for (brightness = 0; brightness <= 255; brightness += 5) {
    strip.clear();
    for (int i = 0; i < MAX_LED; i++) {
      strip.setBrightness(brightness);
      strip.setPixelColor(i, strip.Color(166, 255, 50));
    }
    strip.show();
    delay(wait);
  }
  for (brightness -= 5; brightness >= 0; brightness -= 5) {
    strip.clear();
    for (int i = 0; i < MAX_LED; i++) {
      strip.setBrightness(brightness);
      strip.setPixelColor(i, strip.Color(166, 255, 50));
    }
    strip.show();
    delay(wait);
  }
}
