/******************************************************
* FileName:      LobotServoController.h
* Company:       乐幻索尔
* Date:          2016/07/02  16:53
* Description:   Lobot舵机控制板二次开发库的宏定义、类等
*****************************************************/

#ifndef LOBOTSERVOCONTROLLER_H
#define LOBOTSERVOCONTROLLER_H

#include <Arduino.h>

//发送部分的指令
#define FRAME_HEADER            0x55   //帧头
#define CMD_SERVO_MOVE          0x03   //舵机移动指令
#define CMD_ACTION_GROUP_RUN    0x06   //运行动作组指令
#define CMD_ACTION_GROUP_STOP   0x07   //停止动作组运行指令
#define CMD_ACTION_GROUP_SPEED  0x0B   //设置动作组运行速度指令
#define CMD_GET_BATTERY_VOLTAGE 0x0F   //获得电池电压指令

//接收部分的指令
#define BATTERY_VOLTAGE       0x0F  //电池电压
#define ACTION_GROUP_RUNNING  0x06  //动作组被运行
#define ACTION_GROUP_STOPPED  0x07  //动作组被停止
#define ACTION_GROUP_COMPLETE 0x08  //动作组完成

struct LobotServo {  //舵机ID和位置结构体
  uint8_t  ID;       //舵机ID
  uint16_t Position; //舵机数据
};

class LobotServoController {
  public:
    LobotServoController();
    LobotServoController(HardwareSerial &A);
    ~LobotServoController();

    void moveServo(uint8_t servoID, uint16_t Position, uint16_t Time);
    void moveServos(LobotServo servos[], uint8_t Num, uint16_t Time);
    void moveServos(uint8_t Num,uint16_t Time, ...);
    void runActionGroup(uint8_t NumOfAction, uint16_t Times);
    void stopActionGroup(void);
    void setActionGroupSpeed(uint8_t NumOfAction, uint16_t Speed);
    void setAllActionGroupSpeed(uint16_t Speed);

    void getBatteryVoltage(void);
    void receiveHandle(void);

  public:
    uint8_t  numOfActinGroupRunning; //正在运行的动作组序号
    uint16_t actionGroupRunTimes; //正在运行的动作组运行次数
    bool isRunning; //有动作组运行？
    uint16_t batteryVoltage; //控制板电池电压

    HardwareSerial *SerialX;
};
#endif
