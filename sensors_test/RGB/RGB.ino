#include <Adafruit_NeoPixel.h>

#define PIN 0
#define MAX_LED 24

Adafruit_NeoPixel strip = Adafruit_NeoPixel( MAX_LED, PIN, NEO_RGB + NEO_KHZ800 );

void setup()
{
  Serial.begin(9600);
  strip.begin();           //初始化Adafruit_NeoPixel；
  strip.show();           //显示所有LED为关状态;
}

void loop()
{
  Serial.println("Send: a:blink; b:blowout");
  if (Serial.available()) {
    char ch = Serial.read();
    if (ch == 'a') {
      _blink();
    }
    if (ch == 'b') {
      _blowout();
    }
  }
  //LED_breath(50);
  //rainbow(50);
}
void _blink() {
  strip.clear();
  strip.setBrightness(69);
  strip.fill(strip.Color(166, 255, 50));
  strip.show();
}
void _blowout() {
  strip.clear();
  strip.setBrightness(0);
  strip.show();
}
void LED_breath(int wait) {
  int brightness;
  for (brightness = 0; brightness <= 255; brightness += 5) {
    strip.clear();
    strip.setBrightness(brightness);
    strip.fill(strip.Color(166, 255, 50));
    strip.show();
    delay(wait);
  }
  for (brightness -= 5; brightness >= 0; brightness -= 5) {
    strip.clear();
    strip.setBrightness(brightness);
    strip.fill(strip.Color(166, 255, 50));
    strip.show();
    delay(wait);
  }
}
void rainbow(int wait) {
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
    for (int i = 0; i < MAX_LED; i++) {
      int pixelHue = firstPixelHue + 65536L;
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}
