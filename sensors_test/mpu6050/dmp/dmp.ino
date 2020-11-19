#include <MPU6050_9Axis_MotionApps41.h>
#include <Wire.h>

MPU6050 mpu;

#define OUTPUT_READABLE_EULER
#define INTERRUPT_PIN 2 // 外部中断引脚
#define LED_PIN 13 // 中断提示，板载led
bool blinkState = false;

// MPU控制/状态变量
bool dmpReady = false; // 如果DMP初始化成功则设置为true
uint8_t mpuIntStatus;  // 保存来自MPU的实际中断状态字节。
uint8_t devStatus; // 在每次设备操作后返回状态（0=成功，！0=错误）。
uint16_t packetSize;    // 预期的DMP数据包大小（默认为42字节）。
uint16_t fifoCount;     // 当前在FIFO中的所有字节数。
uint8_t fifoBuffer[64]; // FIFO存储缓冲区

// 方向/运动变量
Quaternion q;        // [w, x, y, z]         四元组
VectorInt16 aa;      // [x, y, z]            加速度传感器测量
VectorInt16 aaReal;  // [x, y, z]            无重力加速度传感器测量
VectorInt16 aaWorld; // [x, y, z]            世界帧加速度传感器测量
VectorFloat gravity; // [x, y, z]            重力矢量
float euler[3];      // [psi, theta, phi]    欧拉角容器
float ypr[3]; // [yaw, pitch, roll]   偏航/俯仰/滚动 容器和重力矢量。

// ================================================================
// ===                       拦截检测程序。                        ===
// ================================================================

volatile bool mpuInterrupt = false; //  表示MPU的中断引脚是否为高电平
void ICACHE_RAM_ATTR dmpDataReady();
void dmpDataReady() {
  mpuInterrupt = true;
}

// ================================================================
// ===                      初始设置                       ===
// ================================================================
void mpu_init() {
  mpu.initialize();
  pinMode(INTERRUPT_PIN, INPUT);

  // 待命读取数据
  Serial.println(F("\n发送任意字符，开始DMP编程和演示: "));

  // 加载并配置DMP
  Serial.println(F("初始化DMP..."));
  devStatus = mpu.dmpInitialize();

  // 在这里提供你自己的陀螺仪偏移量，根据最小灵敏度进行调整。
  mpu.setXGyroOffset(220);
  mpu.setYGyroOffset(76);
  mpu.setZGyroOffset(-85);
  mpu.setZAccelOffset(1788); // 我的测试芯片的出厂默认值是1688

  // 确保成功(如果成功则返回0)
  if (devStatus == 0) {
    // 校准时间：生成偏移量并校准我们的MPU6050。
    mpu.CalibrateAccel(6);
    mpu.CalibrateGyro(6);
    mpu.PrintActiveOffsets();
    // 开启DMP，现在它已经准备好了。
    Serial.println(F("启用DMP..."));
    mpu.setDMPEnabled(true);

    // 启用Arduino中断检测
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady,
                    RISING);
    mpuIntStatus = mpu.getIntStatus();
    // 设置我们的DMP Ready标志，这样main loop()函数就知道可以使用它了。
    Serial.println(F("DMP准备好了! 等待第一次中断..."));
    dmpReady = true;

    // 获取预期的DMP数据包大小，以便日后比较。
    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
    // ERROR!
    // 1 = 初始内存加载失败
    // 2 = DMP配置更新失败
    // (如果要中断，通常代码会是1)
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }
}
void setup() {
  Serial.begin(9600);
  Wire.begin();
  // 初始化设备
  Serial.println(F("初始化I2C设备..."));
  mpu_init();
  // configure LED for output
  pinMode(LED_PIN, OUTPUT);
}

// ================================================================
// ===                         主程序循环                         ===
// ================================================================

void loop() {
  mpu_dmp();
}

void mpu_dmp() {
  // 如果编程失败，就不要尝试做任何事情。
  if (!dmpReady)
    return;

  // 等待MPU中断或额外的数据包可用。
  while (!mpuInterrupt && fifoCount < packetSize) {
    if (mpuInterrupt && fifoCount < packetSize) {
      // 试着跳出死循环
      fifoCount = mpu.getFIFOCount();
    }
    // 其他程序行为的东西在这里
    // .
    // .
    // .
    // 如果你真的有妄想症，你可以经常在其他的中间测试
    // 查看mpuInterrupt是否为真，如果为真，则用 "break; "从
    // while()循环立即处理MPU数据
    // .
    // .
  }

  // 复位中断标志，并获得INT_STATUS字节。
  mpuInterrupt = false;
  mpuIntStatus = mpu.getIntStatus();

  // 获取当前FIFO数
  fifoCount = mpu.getFIFOCount();
  if (fifoCount < packetSize) {
    // 让我们回去，等待另一个中断。我们不应该在这里，我们从另一个事件中被打断了。
    // 这是阻塞，所以不要这么做 while (fifoCount < packetSize) fifoCount =
    // mpu.getFIFOCount();
  }
  // 溢出检查 (除非我们的代码效率太低，否则这种情况绝对不会发生)
  else if ((mpuIntStatus & (0x01 << MPU6050_INTERRUPT_FIFO_OFLOW_BIT)) ||
           fifoCount >= 1024) {
    // 重启 so we can continue cleanly
    mpu.resetFIFO();
    //  fifoCount = mpu.getFIFOCount(); // 复位后将为零，无需询问。
    Serial.println(F("FIFO overflow!"));
    // 否则，检查是否有DMP数据准备好的中断(应该经常发生)
  } else if (mpuIntStatus & (0x01 << MPU6050_INTERRUPT_DMP_INT_BIT)) {
    //  读包 from FIFO
    while (fifoCount >= packetSize) { // Lets catch up to NOW, someone is
      // using the dreaded delay()!
      mpu.getFIFOBytes(fifoBuffer, packetSize);
      // 在这里跟踪FIFO计数，以备有>1个数据包可用。
      // (这样可以让我们在不等待中断的情况下立即读取更多信息)
      fifoCount -= packetSize;
    }


#ifdef OUTPUT_READABLE_EULER
    // 以度为单位显示欧拉角
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetEuler(euler, &q);
    Serial.print("euler\t");
    Serial.print(euler[0] * 180 / M_PI);
    Serial.print("\t");
    Serial.print(euler[1] * 180 / M_PI);
    Serial.print("\t");
    Serial.println(euler[2] * 180 / M_PI);
#endif

    // 閃爍的LED指示活動
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);
  }
  mpu.resetFIFO();
}
