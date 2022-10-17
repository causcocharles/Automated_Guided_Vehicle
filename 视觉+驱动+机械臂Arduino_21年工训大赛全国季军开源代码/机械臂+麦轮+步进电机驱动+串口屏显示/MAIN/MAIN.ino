/************ demo大师例子包 *************/
//  内容：2021-工训赛物流搬运-天津赛区
//  参考教程：https://zhuanlan.zhihu.com/p/366523960
//  知乎：Poao  
//  地址：https://www.zhihu.com/people/panda-13-16 
//       加油！💪?   --- 2021/10/5?? 

// 1、主要程序都配备诸多注释解释
// 2、部分注释内容是当时用于调试的代码段，可忽略掉

#include <Arduino.h>
#include <string.h>     // 计算需要
// Serial1 -- openmv   ||  Serial2  -- 机械臂  ||  Serial3  --  hwt101
// SoftwareSerial UART(45,47); // RX\TX  --- 串口屏


#include <LobotServoController.h>
LobotServoController myse(Serial2);
//#include <avr/pgmspace.h>

/*==============小车行驶的速度（主程序中定义的）============*/
float linear_v = 0.32; //速度控制     适合速度： 0.32
// 该参数值并非实际的速度，只是与之成正比。  
// 调试时，建议先修改数值为0.15及以下 —— 数值设置过高，驱动芯片发热严重。
// 使用a4988驱动时，速度在0.15比较合适，drv8825时，速度在0.32比较合适
// 当然，具体设置多少，请以自己的实际情况来定，毕竟硬件环境不一样。


/************ openmv-uart1 *************/
// 顺序的计算部分，不建议一上来就看。 —— 知乎上会更新讲解文章。
// 发送  CM+XX   XX = QR|扫描二维码 、 WL|获取物料放置位置
// 接收  QR_XXXXXX  WL_XXXXXX   (X = 1、2、3)
// 对应 QR|任务码  WL|物料位置  SX|机械臂抓取顺序（依据QR&WL计算）
String SeH_pose = "321";   // 色环顺序 （已知）
char* SeHuan = "321";     // 色环顺序
              // char* 仅为简化计算顺序 
String WL1_pose;
String WL2_pose;
String QR1_code;     //  上层物料，放置抓取至 粗加工、半成品区动作顺序
String QR2_code;     //  下层物料，放置抓取至 粗加工、半成品区动作顺序
//char* WL1;        // 上层物料顺序
//char* WL2;        // 下层物料顺序
//char* QR1;        // 任务码1顺序
//char* QR2;        // 任务码2顺序

//   openmv发送的数据  
//  String WL1_pose = "312";
//  String WL2_pose = "123";
//  String QR1_code = "123";
//  String QR2_code = "123";

////  下边是机械臂控制所需的四个顺序
//int aa,bb,cc,dd;   // 分别代表四个顺序  A[3]='\0'     //避免它计算错误。。
char A[4];   //  # 抓取上层物料      QR1 IN POS1
char B[4];   //  # 放置上层物料至色环       QR1 IN SH
char C[4];   //  # 抓取下层物料      QR2 IN POS2
char D[4];   //  # 放置下层物料至色环       QR2 IN SH

// 控制机械臂动作的顺序被拆分了（以前采取的方式）
//  YLS = "YLS_"+ A + "_"+ B;  aa bb   √
//  YLX = "YLX_"+ C + "_"+ D;  cc dd   √
//  JGS = "JGQ_"+ QR1_code;
//  JGX = "JGQ_"+ QR2_code;
//  BCS = "BCS_"+ QR1_code;
//  BCX = "BCX_"+ QR2_code;


char uart_recv[64];     // 串口缓存数据
void Uart_send_openmv(char CMD);
void Uart_recv_openmv();     // 串口接收数据


/************ 串口屏- uart(2,3) *************/
#include <SoftwareSerial.h>
SoftwareSerial UART(45, 47); // RX\TX  --- 串口屏
unsigned char HexEnd[3] = {0xff, 0xff, 0xff}; // 串口屏所需结束符


/************ HWT101- uart3 *************/
#include <Wire.h>
#include <JY901.h>

float yaw_0 = 0;   // 上一次角度数据
float Gyro_0 = 0;  // 上一次角加速度
float yaw;   // 当前角度数据
float Gyro;  // 当前角加速度
// int count=0;


/*=============== 定时器2 =================*/
//  用于获取 HWT101 数据，并显示
#include <FlexiTimer2.h>  // 定时器2  
// mega2560 使用 FlexiTimer2时， 9/10 引脚pwm无法使用


// 随着代码量的增加，后期发现abs函数有时候不能正常使用
float abs_yaw(float x)   // 使用默认的 abs 容易出错
{
  if (x < 0)
    x = -x;
  return x;
}


void setup()
{

  /*--------------硬件串口初始化----------------*/
  Serial.begin(115200);   // 调试用的串口

  //uart3 ---  to hwt101
  Serial3.begin(115200);

  //uart1 ---  to openmv4 [TX-P4, RX-P5]
  Serial1.begin(9600);

  //uart2 ---  to arduino uno （机械臂）
  Serial2.begin(9600);  // 115200

  //uart ---  to 串口屏 【软串口，纯发数据】
  UART.begin(115200);

  // UART_update(1);

  /*-------------- 开定时器2的中断 ----------------*/
  // 200ms5Hz -- 单纯用于更新 HWT101数据，可行。  附加TM1637显示，则出现 咔咔咔（不明显）
  // 纯粹为了更新数据，可加快尝试 100ms  / 150ms
  FlexiTimer2::set(200, 1.0 / 1000, update_101); // 50ms太卡。每xx*1ms执行一次Event
  FlexiTimer2::start();    // 开中断   50ms20Hz-太卡  200ms5Hz- 依旧有些咔咔咔

  /*------------步进电机初始化--------------*/
  intiMotors();
  delay(100);
 

  bool test_tag = 1;   // 测试使用的中间变量
  if (test_tag == 1) {   // 下面这些 stepxx 在“Step”中。
    // 步骤一：离开出发区  √
    stepOne();

    // 步骤二：前去识别二维码  √
    stepTwo();

    // 步骤三：前往原料区 √
    stepThree();

    // 步骤四：前往粗加工区  √
    stepFour();

    // 步骤五：前往半成品区    √  
    stepFive();

    // 步骤六：返回原料区,【抓取第二层物料】√
    stepSix();

    // 步骤七：前往粗加工区 √
    stepSeven();

    // 步骤八：前往半成品区   √
    stepEight();

    // 步骤九：返回返回区 √
    stepNine();

  }   // test_tag


}


void loop()   // 用不上
{

// 将setup 中的 test_tag=0 ，就可进行一系列的调试
//  XJ_test();     // 调试循迹专用函数
//  delay(500);

}
