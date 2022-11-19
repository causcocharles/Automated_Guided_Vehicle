#include "ESP32Car.h"
#include <math.h>
#include "PS2X_lib.h"
#include "pins_ESP32S3YD.h"
#include <Ticker.h>
#include "OOPConfig.h"

/******************全局初始化***************************/
#define BAUD 115200
void E_Revenger_Init(void);

/******************电机控制***************************/
void Chassis_Init(void);
int motospeed[4] = {0,0,0,0},
    xspeed = 0,
    yspeed = 0,
    angle_speed = 0;
void RotateSpeedToLinearSpeed(int xspeed, int yspeed, int w,int *wheel_linear_speed);
/************************* PID ****************************/
#define TARGET_PULSES long(COUNTS_PER_10MS/2)

//创建4个编码器实例
SunEncoder ENC[WHEELS_NUM] = {
    SunEncoder(M1ENA, M1ENB), SunEncoder(M2ENA, M2ENB),
    SunEncoder(M3ENA, M3ENB), SunEncoder(M4ENA, M4ENB)};


long targetPulses[WHEELS_NUM] = {TARGET_PULSES, TARGET_PULSES, TARGET_PULSES, TARGET_PULSES};  //四个车轮的目标计数
long feedbackPulses[WHEELS_NUM] = {0, 0, 0, 0};  //四个车轮的10ms定时中断编码器四倍频计数
double outPWM[WHEELS_NUM] = {0, 0, 0, 0};//输出PWM值

//创建4个速度PID实例
/*PID(float min_val, float max_val, float kp, float ki, float kd)
 * float min_val = min output PID value
 * float max_val = max output PID value
 * float kp = PID - P constant PID控制的比例、积分、微分系数
 * float ki = PID - I constant
 * float di = PID - D constant
 * Input	(double)输入参数feedbackPulses，待控制的量
 * Output	(double)输出参数outPWM，指经过PID控制系统的输出量
 * Setpoint	(double)目标值targetPulses，希望达到的数值
 */
#ifdef USE_12V366RPM13PPR30_MOTOR
float Kp = 10, Ki = 0.1, Kd = 0;
#endif

#ifdef USE_12V333RPM11PPR30_MOTOR
float Kp = 10, Ki = 0.1, Kd = 0;
#endif

#ifdef USE_12V366RPM500PPR30_MOTOR
float Kp = 0.5, Ki = 0.01, Kd = 0;
#endif
PID VeloPID[WHEELS_NUM] = {
    PID(PWM_MIN, PWM_MAX, Kp, Ki, Kd), PID(PWM_MIN, PWM_MAX, Kp, Ki, Kd),
    PID(PWM_MIN, PWM_MAX, Kp, Ki, Kd), PID(PWM_MIN, PWM_MAX, Kp, Ki, Kd)};

//*****************创建1个4路电机对象***************************//
BDCMotor motors;

int print_Count = 0;

/******************编码器***************************/
#define FIXENCODER1 1
#define FIXENCODER2 0
#define FIXENCODER3 1
#define FIXENCODER4 0
#define WHEELS_NUM 4
// #define COUNTS_PER_REV 1560  // wheel encoder's num of ticks per rev 390*4=1560

float Velocity[WHEELS_NUM];  //电机速度
int SpeedPWM[WHEELS_NUM];
bool fixEncoder[WHEELS_NUM];  //编码器计数方向修正，0不需要修正,1需要修正


void Encoder_Init(void);

/******************时钟中断***************************/

#define MYIRTIME 10  // 自定义中断时间，单位ms，只能是10ms的整数倍

Ticker timer1;                          // 中断函数
const unsigned int interruptTime = 10;  // 中断时间
unsigned int printCount = 0;            // 中断时间
int timer_flag = 0;                     //定时器标志；

unsigned long lastTime = 0;
int millisecond = 0, mytimer_flag = 0;  // millisecond 范围0~9999

//定时器中断处理函数,得到编码器采样周期内的计数并清零
void timerIsr(void);
void CheckTimInterupt(int tim);
void PeriodElapsedCallback(int tim);

/******************遥控器***************************/
// pin 15 HIGH使用遥控器，LOW不使用
// pin 16 有无动静
// pin 18 前进后退 LOW 前进， HIGH 后退
// pin 17 左右 LOW 左， HIGH 右
#define WHETHER_USE_REMOTE_CONTROL 1  // 1 开启遥控, 0 不开启遥控
#define PS2_DAT 13                    //MISO
#define PS2_CMD 11                    //MOSI
#define PS2_SEL 10                    //SS
#define PS2_CLK 12                    //SLK

#define RC_ON HIGH
#define RC_OFF LOW



void PS2_Init(void);
void RemoteControlRead(void);
void RemoteControlDelay(int tim);

//#define pressures   true
#define pressures false
//#define rumble      true
#define rumble false

PS2X ps2x;  // create PS2 Controller Class
int error = 0;
byte type = 0;
byte vibrate = 0;
int rc_flag = 0;

// main
void setup(void) {
  E_Revenger_Init();
}
void loop(void) {
}

//函数定义
/******************全局***************************/
void E_Revenger_Init(void) {
  IR_Init();
  Serial.begin(BAUD);
  Encoder_Init();
  Chassis_Init();
  // PS2_Init();
  Init_timerIsr();
}
/******************底盘***************************/
void Chassis_Init(void) {
  pinMode(M1PWM1, OUTPUT);
  pinMode(M1PWM2, OUTPUT);
  pinMode(M2PWM1, OUTPUT);
  pinMode(M2PWM2, OUTPUT);
  pinMode(M3PWM1, OUTPUT);
  pinMode(M3PWM2, OUTPUT);
  pinMode(M4PWM1, OUTPUT);
  pinMode(M4PWM2, OUTPUT);
}

void RotateSpeedToLinearSpeed(int xspeed, int yspeed, int w,int *wheel_linear_speed){
  // 转动模式
	// wheel_linear_speed[0] = -xspeed + yspeed + w;
	// wheel_linear_speed[1] =  xspeed + yspeed + w;
	// wheel_linear_speed[2] = -xspeed - yspeed + w;
	// wheel_linear_speed[3] =  xspeed - yspeed + w;
  // 平动模式
	wheel_linear_speed[0] = -xspeed + yspeed + w;
	wheel_linear_speed[1] =  xspeed + yspeed + w;
	wheel_linear_speed[2] = -xspeed + yspeed + w;
	wheel_linear_speed[3] =  xspeed + yspeed + w;
}
/******************编码器***************************/
void Encoder_Init(void) {
  // IO引脚初始化
  pinMode(M1ENA, INPUT_PULLUP);
  pinMode(M1ENB, INPUT_PULLUP);
  pinMode(M2ENA, INPUT_PULLUP);
  pinMode(M2ENB, INPUT_PULLUP);
  pinMode(M3ENA, INPUT_PULLUP);
  pinMode(M3ENB, INPUT_PULLUP);
  pinMode(M4ENA, INPUT_PULLUP);
  pinMode(M4ENB, INPUT_PULLUP);

  attachInterrupt(M1ENA, readM1EncoderA,
                  CHANGE);  //中断处理函数，电平变化触发
  attachInterrupt(M1ENB, readM1EncoderB,
                  CHANGE);  //中断处理函数，电平变化触发
  attachInterrupt(M2ENA, readM2EncoderA,
                  CHANGE);  //中断处理函数，电平变化触发
  attachInterrupt(M2ENB, readM2EncoderB,
                  CHANGE);  //中断处理函数，电平变化触发
  attachInterrupt(M3ENA, readM3EncoderA,
                  CHANGE);  //中断处理函数，电平变化触发
  attachInterrupt(M3ENB, readM3EncoderB,
                  CHANGE);  //中断处理函数，电平变化触发
  attachInterrupt(M4ENA, readM4EncoderA,
                  CHANGE);  //中断处理函数，电平变化触发
  attachInterrupt(M4ENB, readM4EncoderB,
                  CHANGE);  //中断处理函数，电平变化触发

  lastTime = millis();
}

void ReadEncoders(void) {
  int i;
  for (i = 0; i < WHEELS_NUM; i++) {
    if (fixEncoder[i] == 0) {
      Velocity[i] = EncoderCount[i];
    } else {
      Velocity[i] = -EncoderCount[i];
    }
    EncoderCount[i] = 0;
  }
}


/******************时钟***************************/
void Init_timerIsr(void) {
  // 定时中断 
  timer1.attach_ms(interruptTime, timerIsr);  // 打开定时器中断
  interrupts();                               //打开外部中断

/************************* PID *************************/
  //获取电机脉冲数（速度）
  timer_flag = 1;  //定时时间达到标志
  print_Count++;
  for (int i = 0; i < WHEELS_NUM; i++) {
    feedbackPulses[i] = ENC[i].read();

    ENC[i].write(0);  //复位
    outPWM[i] = VeloPID[i].Compute(targetPulses[i], feedbackPulses[i]);
  }
  // motors.setSpeeds(outPWM[0], outPWM[1], outPWM[2], outPWM[3]);
  for(int i = 0; i < WHEELS_NUM; i++){
    Serial.print(outPWM[i]);
    Serial.print(' ');
  }
  Serial.println(' ');
}

//定时器中断处理函数,得到编码器采样周期内的计数并清零
void timerIsr(void) {
  timer_flag = 1;  //定时时间达到标志
  ReadEncoders();
  // RemoteControlRead();
  CheckTimInterupt(MYIRTIME);
}
// 自定义中断
void CheckTimInterupt(int tim) {
  if (millisecond >= tim) {
    millisecond = 0;
    PeriodElapsedCallback(tim);
  } else {
    millisecond++;
  }
}

void PeriodElapsedCallback(int tim) {
  if (millisecond < tim) {
    mytimer_flag += 10;
  } else {
    mytimer_flag = 1;
  }
  judgemode();
}

/******************和Arduino_UNO通信***************************/
void IR_Init(void){
  pinMode(9,INPUT);
  pinMode(46,INPUT);
  pinMode(8,INPUT);
  pinMode(3,INPUT);
  Init_timerIsr();
}

void judgemode(void){
    int tmp;
    xspeed = yspeed = 0;
    if(digitalRead(3)==HIGH){
        if(digitalRead(46)==LOW){
          yspeed = 120;
          Serial.println("LEFT");
        }
        else{
          yspeed = -120;
          Serial.println("RIGHT");
        }
    }
    if(digitalRead(8)==HIGH){
        if(digitalRead(9)==LOW){
          xspeed = 120;
          Serial.println("FORWARD");
        }
        else{
          xspeed = -120;
          Serial.println("BACKWARD");
        }
    }
    RotateSpeedToLinearSpeed(xspeed, yspeed, angle_speed, motospeed);
    setSpeeds(motospeed[0], motospeed[1], motospeed[2], motospeed[3]);
}

