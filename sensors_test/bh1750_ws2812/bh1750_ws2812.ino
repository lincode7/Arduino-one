#include <Adafruit_NeoPixel.h>
#include <BH1750.h>
#include <Wire.h>
/* WS2812,可编程led
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
 
//数据处理所需变量
uint32_t table_light = 500; // The favourable light intensity on table
int environment = 0;        // 0代表无需补光，1代表需要补光
float p_lux = 0;            //上一个光强数值
float n_lux = 0;            //现在读取光强数值

void setup() {
    Serial.begin(9600);

    Serial.println("Initializing WS2812 devices...");
    // WS2812,RGB_led，
    Lights.begin();
    if (Lights.canShow())
        Serial.println("WS2812 Test begin");
    else
        Serial.println("WS2812 Connet Failed");
    Lights.setBrightness(0);
    // BH1750,light sensor
    Wire.begin();
    if (bh1750.begin())
        Serial.println("BH1750 Test begin");
    else
        Serial.println("BH1750 Connet Failed");
}
void loop() {
    /*
     * 跟据光强变化改变led亮度
     * 每10min进行一次检测，个人估计10min内环境需求光强变化不大
     */
    // mpu_data();
    // delay(50);
    Get_Light_Intensity();
    delay(50);
    Auto_Brightness(50); //根据2个光强制计算偏差比重（当前偏差/标准偏差）
    delay(50);
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
/**
 * 根据光强数值改变亮度
 * 取2个光强值，前后值间隔10min，组合成光强变化偏差值
 * 定时校准函数获取 最大暗-亮光强偏差值 （standard）映射到亮度255
 * 其余情况 暗-亮光强迫偏差值 （others）映射到255*(others/standard)
 **/
void Auto_Brightness(int wait) {
    if (environment == 1) {
        int last_brightness = Lights.getBrightness();
        int brightness = last_brightness * (1 + (p_lux - n_lux) / p_lux);
        Lights.clear();
        for (int j = 0; j < NUM_LED; j++) {
            Lights.setBrightness(brightness);
            Lights.setPixelColor(j, Lights.Color(127, 150, 60));
            delay(wait);
        }
        Lights.show();
    }
}
