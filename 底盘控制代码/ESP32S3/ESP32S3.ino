#include <Ticker.h>     //定时中断
#include "OOPConfig.h"  //包含配置库
#include <U8g2lib.h>    //包含OLED库
#include "ESP32Car.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps_V6_12.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#include <math.h>
#endif

// #include "SunGrayscale_b.h"

#ifndef DEBUG_INFO
#define DEBUG_INFO
#endif
#ifndef BAUDRATE
#define BAUDRATE 115200
#endif

#define TARPULSE 30
#define ADJUSTPLSE 20
#define XP 1
#define XI 0.01
#define XD 0.1
#define YP 1.5
#define YI 0.01
#define YD 0.01
#define AP 1
#define AI 0.01
#define AD 0.01
#define KX 7

int stage = 0, waitcount = 0, waittar = 0, waitopen = 0;
int linepatrol_on = 0, linecount = 0, linetar = 0, microtimer = 0;
int turn_on = 0;
int adjust_on = 0;
void MapRunning(void);

MPU6050 mpu;
// MPU control/status vars
bool dmpReady = false;   // set true if DMP init was successful如果dmp设置为真则设置为true
uint8_t mpuIntStatus;    // holds actual interrupt status byte from MPU
uint8_t devStatus;       // return status after each device operation (0 = success,
                         // !0 = error)
uint16_t packetSize;     // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;      // count of all bytes currently in FIFO
uint8_t fifoBuffer[64];  // FIFO storage buffer FIFO存储缓存区

// orientation/motion vars
Quaternion q;    // [w, x, y, z]         quaternion container
VectorInt16 aa;  // [x, y, z]            accel sensor measurements
VectorInt16 gy;  // [x, y, z]            gyro sensor measurements
VectorInt16
  aaReal;  // [x, y, z]            gravity-free accel sensor measurements
VectorInt16
  aaWorld;            // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;  // [x, y, z]            gravity vector
float
  ypr[3];  // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

void MPU6050_Init(void);
float raw_yaw = 0, tar_yaw = 0;
int raw_yaw_flag = 0;

// OLED配置esp32引脚映射
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, SCL, SDA);  // ESP32 Thing, HW I2C with pin remapping
void OLED_Init(void);
void OLED_Print(String x);

float xspeed = 0, yspeed = 0, angular_velocity = 0;
float X_SUM = 0, Y_SUM = 0, A_SUM = 0;
float x_offset = 0, y_offset = 0, a_offset = 0;

//灰度传感器函数
Grayscale GraySensors;
Grayscale_b GraySensors_b;
// uart输出量的结构体
Grayscale::strOutput GrayUartOutputIO;
Grayscale_b::strOutput_b GrayUartOutputIO_b;
// 中断函数
Ticker timer1;
bool timer_flag = 0;

// 创建4个编码器实例
SunEncoder ENC[WHEELS_NUM] = {
  SunEncoder(M1ENA, M1ENB), SunEncoder(M2ENA, M2ENB),
  SunEncoder(M3ENA, M3ENB), SunEncoder(M4ENA, M4ENB)
};

long targetpulse[WHEELS_NUM] = { 0, 0, 0, 0 };     //四个车轮的目标计数
long feedbackPulses[WHEELS_NUM] = { 0, 0, 0, 0 };  //四个车轮的10ms定时中断编码器四倍频计数
double outPWM[WHEELS_NUM] = { 0, 0, 0, 0 };        //输出PWM值

// PID参数设置
float Kp = 10, Ki = 0.1, Kd = 0;
float P, I, D;
PID VeloPID[WHEELS_NUM] = {
  PID(PWM_MIN, PWM_MAX, Kp, Ki, Kd), PID(PWM_MIN, PWM_MAX, Kp, Ki, Kd),
  PID(PWM_MIN, PWM_MAX, Kp, Ki, Kd), PID(PWM_MIN, PWM_MAX, Kp, Ki, Kd)
};

// 创建1个4路电机对象
BDCMotor motors;

typedef struct grayscaledata {
  uint8_t lf, lm, lb, rf, rm, rb;
  int8_t translation_error = 0,  // 旋转偏差: 逆时针偏角为正，顺时针偏角为负，取值
    rotation_error = 0;          // 平动(左右)偏差: 正为偏前，负为偏后，取值-2~2
  uint8_t unknowncase;
} GrayscaleDataType;

GrayscaleDataType GrayscaleData;

int n = 0, flag = 0;
int8_t begin, end;
uint8_t direction = 0;  // 运功方向0:不动,1:x轴,2:y轴

// 电机函数
void MotorSpeeds(int m1, int m2, int m3, int m4);
void Forward_Move(int targetpulse);
void Backward_Move(int targetpulse);
void Left_Move(int targetpulse);
void Right_Move(int targetpulse);
// void Left_Turn(int angle);
void Right_Turn(int angle);
void Turn_Callback(void);
void Line_Patrol(void);  // 巡线
void Line_Patrol_y(void);
void PID_Static_Adjust(void);
void X_Adjust(void);
void Y_Adjust(void);
void Z_Adjust(void);
void U8G2_Print(void);
void WaitCallback(void);
void Wait(int time);


void setup() {
  OLED_Init();
  Serial.begin(BAUDRATE);
  Chassis_Init();
  MPU6050_Init();
  delay(2000);
  // Right_Turn(90);
}
uint8_t flag_MPU = 0;
void loop() {
  MPU6050_Read();
  U8G2_Print();
  MapRunning();
}

void MapRunning() {

  switch (stage) {
    case 0: Wait(500); break;
    case 1: Line_Patrol_on(3); break;
    case 2: Wait(500); break;
    case 3: Static_Adjust_on(); break;
    case 4: Wait(500); break;
    case 5: Right_Turn(90); stage++; break;
    case 6: Wait(2000); break;
    case 7: Static_Adjust_on(); break;
    case 8: Wait(500); break;
    case 9: Line_Patrol_on(2); break;
    case 10: Wait(500); break;
    case 11: Static_Adjust_on(); break;
    case 12: Wait(500); break;
    case 13: Right_Turn(90); stage++; break;
    case 14: Wait(2000); break;
    case 15: Static_Adjust_on(); break;
    case 16: Wait(500); break;
    case 17: Line_Patrol_on(3); break;
    case 18: Wait(500); break;
    case 19: Static_Adjust_on(); break;
    case 20: Wait(500); break;
    case 21: Right_Turn(90); stage++; break;
    case 22: Wait(2000); break;
    case 23: Static_Adjust_on(); break;
    case 24: Wait(500); break;
    case 25: Line_Patrol_on(2); break;
    case 26: Wait(500); break;
    case 27: Static_Adjust_on(); break;
    case 28: Wait(500); break;
    case 29: Right_Turn(90); stage++; break;
    case 30: Wait(2000); break;
    case 31: Static_Adjust_on(); break;
    case 32: Wait(500); break;
      // case 3: Right_Turn(-90);break;
      // case 4: Wait(2000);

    default: break;
  }
}

void Wait(int time) {
  waitopen = 1;
  waittar = time / 10;
  if (waitcount > waittar) {
    stage++;
    waitopen = 0;
  }
}

void WaitCallback(void) {
  if (waitopen) waitcount++;
  else waitcount = 0;
}

void U8G2_Print(void) {
  u8g2.clearBuffer();                  // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB12_tf);  // choose a suitable font
  u8g2.setCursor(0, 16);

  u8g2.print("x_offset:");
  u8g2.print(x_offset);
  // u8g2.print("  rf");
  // u8g2.print(GrayscaleData.rf);
  u8g2.setCursor(0, 32);
  u8g2.print("y_offset:");
  u8g2.print(y_offset);
  // u8g2.print("  rm");
  // u8g2.print(GrayscaleData.rm);
  u8g2.setCursor(0, 48);
  u8g2.print("a_offset:");
  u8g2.print(a_offset);
  // u8g2.print("  rb");
  // u8g2.print(GrayscaleData.rb);
  u8g2.setCursor(0, 64);
  // u8g2.print("D:");
  // u8g2.print(D);
  u8g2.print("stage:");
  u8g2.print(stage);
  // u8g2.print("  re");
  // u8g2.print(GrayscaleData.rotation_error);

  u8g2.sendBuffer();
}

void Chassis_Init(void) {
  motors.init();
  motors.map2model(1, 2, 3, 4);                                                   //映射实际接线电机编号到理论建模编号，默认顺序1，2，3，4
  motors.flipMotors(FLIP_MOTOR[0], FLIP_MOTOR[1], FLIP_MOTOR[2], FLIP_MOTOR[3]);  // 需修改 OOPConfig.h 设置为绿色尾部电机参数
  motors.setSpeeds(0, 0, 0, 0);
  for (int i = 0; i < WHEELS_NUM; i++) {
    ENC[i].init();
    ENC[i].flipEncoder(FLIP_ENCODER[i]);
  }
  GrayscaleData.rotation_error = GrayscaleData.translation_error = 0;
  timer1.attach_ms(TIMER_PERIOD, timerISR);  // 打开定时器中断

  // attachInterrupt(44,UART_Receive,CHANGE);
  interrupts();  //打开外部中断
}

//定时器中断处理函数,其功能主要为了输出编码器得到的数据
void timerISR() {
  // raw_yaw_flag = 2;
  UART_Receive();
  GrayUartOutputIO = GraySensors.readUart();  //读取串口数字量数据
  GrayUartOutputIO_b = GraySensors_b.readUart_b();
  WaitCallback();
  PID_Static_Adjust();
  Line_Patrol();
  Turn_Callback();
  Velocity_Transform();

  //获取电机脉冲数（速度）
  timer_flag = 1;  //定时时间达到标志
  for (int i = 0; i < WHEELS_NUM; i++) {
    feedbackPulses[i] = ENC[i].read();

    ENC[i].write(0);  //复位
    outPWM[i] = VeloPID[i].Compute(targetpulse[i], feedbackPulses[i]);
  }
  motors.setSpeeds(outPWM[0], outPWM[1], outPWM[2], outPWM[3]);
}


void UART_Receive(void) {
  if ((flag = Serial.available()) >= 11) {
    begin = Serial.read();
    GrayscaleData.lf = Serial.read();
    GrayscaleData.lm = Serial.read();
    GrayscaleData.lb = Serial.read();
    GrayscaleData.rf = Serial.read();
    GrayscaleData.rm = Serial.read();
    GrayscaleData.rb = Serial.read();

    GrayscaleData.translation_error = Serial.read();
    GrayscaleData.rotation_error = Serial.read();
    GrayscaleData.translation_error = -GrayscaleData.lf + GrayscaleData.lb - GrayscaleData.rf + GrayscaleData.rb;
    GrayscaleData.rotation_error = GrayscaleData.lf - GrayscaleData.lb - GrayscaleData.rf + GrayscaleData.rb;
    GrayscaleData.unknowncase = Serial.read();

    end = Serial.read();
    // flag = 1;
    while (Serial.read() >= 0) {}
  }
}

// OLED打印函数
void OLED_Init(void) {
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.clearBuffer();               // clear the internal memory
  u8g2.setFont(u8g2_font_7x14_tf);  // choose a suitable font
  u8g2.setCursor(0, 16);
  u8g2.println("E-Revenger");
  u8g2.setCursor(0, 16);
  u8g2.sendBuffer();
}

// void OLED_Print(String x) {
//   u8g2.println(x);
//   u8g2.setCursor(0, 16);
//   u8g2.sendBuffer();
// }

void Velocity_Transform(void) {
  targetpulse[0] = int(xspeed - yspeed - angular_velocity);
  targetpulse[1] = int(xspeed + yspeed + angular_velocity);
  targetpulse[2] = int(xspeed + yspeed - angular_velocity);
  targetpulse[3] = int(xspeed - yspeed + angular_velocity);
}

void MotorSpeeds(int m1, int m2, int m3, int m4) {
  targetpulse[0] = m1;
  targetpulse[1] = m2;
  targetpulse[2] = m3;
  targetpulse[3] = m4;
}

void Forward_Move(int targetpulse) {
  MotorSpeeds(targetpulse, targetpulse, targetpulse, targetpulse);
}

void Backward_Move(int targetpulse) {
  MotorSpeeds(-targetpulse, -targetpulse, -targetpulse, -targetpulse);
}

void Left_Move(int targetpulse) {
  MotorSpeeds(-targetpulse, targetpulse, targetpulse, -targetpulse);
}

void Right_Move(int targetpulse) {
  MotorSpeeds(targetpulse, -targetpulse, -targetpulse, targetpulse);
}

void Right_Turn(int angle) {
  raw_yaw_flag = 1;
  tar_yaw = ypr[0] * 180 / M_PI + angle < 180 ? ypr[0] * 180 / M_PI + angle : ypr[0] * 180 / M_PI + angle - 360;
}

void Turn_Callback(void) {
  if (raw_yaw_flag) {
    // raw_yaw_flag ++;
    a_offset = -(tar_yaw - ypr[0] * 180 / M_PI);
    if (a_offset > 180) {
      a_offset -= 360;
    } else if (a_offset < -180) {
      a_offset += 360;
    }
    A_Adjust();
    if (abs(a_offset) < 5) {
      raw_yaw_flag = 0;
      a_offset = 0;
      stage++;
    }
  }
}

void Normal_Stop() {
  MotorSpeeds(0, 0, 0, 0);
}

void Brake_Stop() {
  brake();
}

void CalculateOffset(void) {
  x_offset = (KX * GrayscaleData.translation_error);
  y_offset = (GrayUartOutputIO.offset - GrayUartOutputIO_b.offset_b) < 100 ? GrayUartOutputIO.offset - GrayUartOutputIO_b.offset_b : 0;
  if (direction == 0) a_offset = 0.6 * (-(GrayUartOutputIO.offset + GrayUartOutputIO_b.offset_b) < 100 ? -GrayUartOutputIO.offset - GrayUartOutputIO_b.offset_b : 0) + 0.4 * KX * GrayscaleData.rotation_error;
  else if (direction == 1) a_offset = -(GrayUartOutputIO.offset + GrayUartOutputIO_b.offset_b) < 100 ? -GrayUartOutputIO.offset - GrayUartOutputIO_b.offset_b : 0;
  else a_offset = KX * GrayscaleData.rotation_error;
}

void Line_Patrol_on(int n) {
  linepatrol_on = 1;
  linetar = n;
  if (linecount >= linetar) {
    // Brake_Stop();
    stage++;
    linepatrol_on = 0;
    xspeed = yspeed = angular_velocity = 0;
  }
}

void Line_Patrol(void) {
  if (linepatrol_on) {
    direction = 1;
    xspeed = 20;
    CalculateOffset();
    Y_Adjust();
    A_Adjust();
    microtimer++;
    if (microtimer >= 30) {
      if ((!GrayscaleData.lf || !GrayscaleData.lm || !GrayscaleData.lb) && (!GrayscaleData.rf || !GrayscaleData.rm || !GrayscaleData.rb)) {
        linecount++;
        microtimer = 0;
      }
     
    }
  } else {
    linecount = 0;
    microtimer = 0;
  }
}

void Line_Patrol_y(void) {
  direction = 2;
  yspeed = 40;
  CalculateOffset();
  X_Adjust();
  A_Adjust();
}

void Static_Adjust_on(void){
  adjust_on = 1;
}

void PID_Static_Adjust(void) {
  if(adjust_on){
    direction = 0;
    CalculateOffset();
    X_Adjust();
    Y_Adjust();
    A_Adjust();
    if(!x_offset&&!y_offset&&!a_offset){
      adjust_on = 0;
      stage++;
      xspeed = yspeed = angular_velocity = 0;
    }
  }  
}

void X_Adjust(void) {
  float error = x_offset;
  float P, I, D;
  if (X_SUM < 20 / XI) X_SUM += error;
  P = XP * error;
  I = XI * X_SUM > 10 ? XI * X_SUM : 0;
  D = XD * (error - x_offset);
  x_offset = error;
  xspeed = P + I + D < 30 ? P + I + D : 30;
}

void Y_Adjust(void) {
  float error = y_offset;
  static int flag = 0;
  flag++;
  float P, I, D;
  if (Y_SUM < 20 / YI) Y_SUM += error;
  P = YP * error;
  I = YI * Y_SUM > 10 ? YI * Y_SUM : 0;
  D = YD * (error - y_offset);
  y_offset = error;
  yspeed = P + I + D < 30 ? P + I + D : 30;
}

void A_Adjust(void) {
  float error = a_offset;
  // float P, I, D;
  if (A_SUM < 20 / AI) A_SUM += error;
  P = AP * error;
  I = AI * A_SUM > 7 ? AI * A_SUM : 0;
  D = AD * (error - a_offset);
  a_offset = error;
  angular_velocity = P + I + D < 30 ? P + I + D : 30;
}

void MPU6050_Init(void) {
  // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
  Wire.setClock(400000);  // 400kHz I2C clock. Comment this line if having
                          // compilation difficulties
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif

  // initialize serial communication
  // (115200 chosen because it is required for Teapot Demo output, but it's
  // really up to you depending on your project)
  Serial.begin(115200);
  while (!Serial)
    ;  // wait for Leonardo enumeration, others continue immediately

  // NOTE: 8MHz or slower host processors, like the Teensy @ 3.3V or Arduino
  // Pro Mini running at 3.3V, cannot handle this baud rate reliably due to
  // the baud timing being too misaligned with processor ticks. You must use
  // 38400 or slower in these cases, or use some kind of external separate
  // crystal solution for the UART timer.

  // initialize device
  Serial.println(F("Initializing I2C devices..."));
  mpu.initialize();

  // verify connection
  Serial.println(F("Testing device connections..."));
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful")
                                      : F("MPU6050 connection failed"));

  // load and configure the DMP
  Serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();

  // supply your own gyro offsets here, scaled for min sensitivity
  //提供陀螺仪的偏移量，运行IMU_Zero获得后输入OOPConfg.h
  mpu.setXAccelOffset(XAccelOffset);
  mpu.setYAccelOffset(YAccelOffset);
  mpu.setZAccelOffset(ZAccelOffset);
  mpu.setXGyroOffset(XGyroOffset);
  mpu.setYGyroOffset(YGyroOffset);
  mpu.setZGyroOffset(ZGyroOffset);
  // make sure it worked (returns 0 if so)
  if (devStatus == 0) {
    // Calibration Time: generate offsets and calibrate our MPU6050
    mpu.CalibrateAccel(6);
    mpu.CalibrateGyro(6);
    Serial.println();
    mpu.PrintActiveOffsets();
    // turn on the DMP, now that it's ready
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

    dmpReady = true;

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }
}

void MPU6050_Read(void) {
  // if programming failed, don't try to do anything
  if (!dmpReady) {
    flag_MPU = 1;
    return;
  }
  // read a packet from FIFO
  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) {  // Get the Latest packet

    // display Euler angles in degrees
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    // Serial.print("yaw\t");
    // Serial.print(ypr[0] * 180 / M_PI);
    // Serial.println();
    mpu.dmpGetGyro(&gy, fifoBuffer);
    // Serial.print("Raw Gyro XYZ\t");
    // Serial.print(gy.x);
    // Serial.print("\t");
    // Serial.print(gy.y);
    // Serial.print("\t");
    // Serial.print(gy.z);
    // Serial.println();
  }
}