#include <Adafruit_NeoPixel.h>
#include <BH1750.h>
#include <Wire.h>
/* WS2812,可编程led
 * 目前led是grb格式
 * VCC-5V
 * GND
 * PIN_IN→GPIO2
 * PIN_OUT→NEXT WS2812, blank is aviliable
*/
#define LED_PIN 14 // D5
#define NUM_LED 16  // 灯珠数目
Adafruit_NeoPixel Lights =
    Adafruit_NeoPixel(NUM_LED, LED_PIN, NEO_RGB + NEO_KHZ800);
/* BH1750,光感传感器
 *  VCC-3.3V
 *  GND
 *  SCL→GPIO5
 *  SDA→GPIO4
*/
BH1750 bh1750; // default initialize the address is 0x23
/* MPU6050,空间运动传感器芯片,可以获取器件当前的三个加速度分量和三个旋转角速度
 *  VCC-5V
 *  GND
 *  SCL→GPIO5
 *  SDA→GPIO4
*/
#define Sensor 2  //Input Pin
int flg = 0;
//数据处理所需变量
uint32_t table_light = 500; // The favourable light intensity on table
int environment = 0;        // 0代表无需补光，1代表需要补光
float p_lux = 0;            //上一个光强数值
float n_lux = 0;            //现在读取光强数值

void setup() {
    Serial.begin(9600);
    Serial.println("Initializing WS2812 devices...");
    pinMode(Sensor, INPUT);
    // WS2812,RGB_led，
    Lights.begin();
    Lights.show();
    Lights.setBrightness(150);
    // BH1750,light sensor
    Wire.begin();
    bh1750.begin();
}
void loop() {
  Get_Light_Intensity();
  delay(50);
  int val = digitalRead(Sensor); //Read Pin as input
  if((val > 0) && (flg==0))  {
    Serial.println("Motion Detected");
    _blink();
    flg = 1;
  }
  if(val == 0) {
    Serial.println("NO Motion");
    Lights.setBrightness(0);
    Lights.clear();
    Lights.show();
    flg = 0;
  }
  delay(100);
}

int Get_Light_Intensity() { //根据环境自动调整感应模式
    p_lux = n_lux;
    n_lux = bh1750.readLightLevel();
    Serial.print("Light: ");
    Serial.print(n_lux);
    Serial.println(" lx");
    if (n_lux < 0) {
        Serial.println(F("检测到的错误条件"));
    } else {
        if (n_lux > 40000.0) {
            // reduce measurement time - needed in direct sun light
            if (bh1750.setMTreg(32)) {
                Serial.println(F("在高光环境下，将MTReg设置为低数值。"));
            } else {
                Serial.println(
                    F("在高光环境下，将MTReg设置为低值时出现错误。"));
            }
        } else {
            if (n_lux > 10.0) {
                if (n_lux < 50) {
                    environment = 1;
                }
                // typical light environment
                if (bh1750.setMTreg(69)) {
                    Serial.println(F("将MTReg设置为正常光照环境下的默认值。"));
                } else {
                    Serial.println(
                        F("将MTReg设置为正常光线环境下的默认值时出现错误。"));
                }
            } else {
                if (n_lux <= 10.0) {
                    environment = 1;
                    // very low light environment
                    if (bh1750.setMTreg(138)) {
                        Serial.println(F("在低光环境下将MTReg设置为高值"));
                    } else {
                        Serial.println(
                            F("在低光环境下，MTReg设置为高值时出现错误。"));
                    }
                }
            }
        }
    }
    Serial.println(F("--------------------------------------"));
    delay(500);
}
void _blink() {
  Lights.setBrightness(69);
  Lights.fill(Lights.Color(166,255,50));
  Lights.show();
}
