#include <LobotServoController.h>

LobotServoController myse;

void setup() {
  pinMode(13,OUTPUT);
  Serial.begin(9600);
  while(!Serial);
  digitalWrite(13,HIGH);

  myse.runActionGroup(100,0);  //不断运行100号动作组
  delay(5000);
  myse.stopActionGroup(); //停止动作组运行
  delay(2000);
  myse.setActionGroupSpeed(100,200); //设置100号动作组运行速度为200%
  delay(2000);
  myse.runActionGroup(100,5);  //运行100号动作组 5次
  delay(5000);
  myse.stopActionGroup(); //停止动作组运行
  delay(2000);
  myse.moveServo(0,1500,1000); //0号舵机1000ms移动至1500位置
  delay(2000);
  myse.moveServo(2,800,1000); //2号舵机1000ms移动至800位置
  delay(2000);
  myse.moveServos(5,1000,0,1300,2,700,4,600,6,900,8,790);  //
  //控制5个舵机，移动时间1000ms，0号舵机至1300位置，2号舵机至700位置，4号舵机至600位置，
  //6号舵机至900位置，8号舵机至790位置
  delay(2000);

  LobotServo servos[2];   //舵机ID位置结构数组
  servos[0].ID = 2;       //2号舵机
  servos[0].Position = 1400;  //1400位置
  servos[1].ID = 4;       //4号舵机
  servos[1].Position = 700;  //700位置
  myse.moveServos(servos,2,1000);  //控制两个舵机，移动时间1000ms,ID和位置有servos指定
}

void loop() {
}