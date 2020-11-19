#include <Wire.h>
#include <BH1750.h>

 /* BH1750,光感传感器
  *  VCC-3.3V
  *  GND
  *  SCL→GPIO5
  *  SDA→GPIO4
 */
BH1750 bh1750; //default initialize the address is 0x23
 
uint32_t table_light = 500;  //The favourable light intensity on table
float p_lux = 0;       //上一个光强数值
float n_lux = 0;       //现在读取光强数值

void setup() {
  Serial.begin(9600);
  //BH1750,light sensor
  Wire.begin();
  if(bh1750.begin())
     Serial.println("BH1750 Test begin");
  else Serial.println("BH1750 Connet Failed");
}
void loop() {
  auto_adjust();
  delay(50);
}
void auto_adjust(){//根据环境自动调整感应模式
  p_lux = n_lux;
  n_lux = bh1750.readLightLevel();
  Serial.print("Light: ");
  Serial.print(n_lux);
  Serial.println(" lx");
  if (n_lux < 0) {
    Serial.println(F("Error condition detected"));
  }
  else {
    if (n_lux > 40000.0) {
      // reduce measurement time - needed in direct sun light
      if (bh1750.setMTreg(32)) {
        Serial.println(F("Setting MTReg to low value for high light environment"));
      }
      else {
        Serial.println(F("Error setting MTReg to low value for high light environment"));
      }
    }
    else {
        if (n_lux > 10.0) {
          // typical light environment
          if (bh1750.setMTreg(69)) {
            Serial.println(F("Setting MTReg to default value for normal light environment"));
          }
          else {
            Serial.println(F("Error setting MTReg to default value for normal light environment"));
          }
        }
        else {
          if (n_lux <= 10.0) {
            //very low light environment
            if (bh1750.setMTreg(138)) {
              Serial.println(F("Setting MTReg to high value for low light environment"));
            }
            else {
              Serial.println(F("Error setting MTReg to high value for low light environment"));
            }
          }
       }
    }

  }
  Serial.println(F("--------------------------------------"));
  delay(500);
}
