/******************************************************
 * FileName:      LobotServoController.cpp
 ** Company:       乐幻索尔
 * Date:           2016/07/02  16:53
 * Description:    Lobot舵机控制板二次开发库，本文件包含
                 了此库的具体实现
 *****************************************************/

#include "LobotServoController.h"

#define GET_LOW_BYTE(A) (uint8_t)((A))
//宏函数 获得A的低八位
#define GET_HIGH_BYTE(A) (uint8_t)((A) >> 8)
//宏函数 获得A的高八位
#define BYTE_TO_HW(A, B) ((((uint16_t)(A)) << 8) | (uint8_t)(B))
//宏函数 以A为高八位 B为低八位 合并为16位整形

LobotServoController::LobotServoController()
{
	//初始化运行中动作组号为0xFF，运行次数为0，运行中标识为假，电池电压为0
	numOfActinGroupRunning = 0xFF;
	actionGroupRunTimes = 0;
	isRunning = false;
	batteryVoltage = 0;
#if defined(__AVR_ATmega32U4__)  //for Arduino Leonardo，Micro....
	SerialX = &Serial1;
#else
	SerialX = &Serial;
#endif
}

LobotServoController::LobotServoController(HardwareSerial &A)
{
	LobotServoController();
	SerialX = &A;
}
LobotServoController::~LobotServoController()
{
}

/*********************************************************************************
 * Function:  moveServo
 * Description： 控制单个舵机转动
 * Parameters:   sevoID:舵机ID，Position:目标位置,Time:转动时间
                    舵机ID取值:0<=舵机ID<=31,Time取值: Time > 0
 * Return:       无返回
 * Others:
 **********************************************************************************/
void LobotServoController::moveServo(uint8_t servoID, uint16_t Position, uint16_t Time)
{
	uint8_t buf[11];
	if (servoID > 31 || !(Time > 0)) { //舵机ID不能打于31,可根据对应控制板修改
		return;
	}
	buf[0] = FRAME_HEADER;                   //填充帧头
	buf[1] = FRAME_HEADER;
	buf[2] = 8;                              //数据长度=要控制舵机数*3+5，此处=1*3+5
	buf[3] = CMD_SERVO_MOVE;                 //填充舵机移动指令
	buf[4] = 1;                              //要控制的舵机个数
	buf[5] = GET_LOW_BYTE(Time);             //填充时间的低八位
	buf[6] = GET_HIGH_BYTE(Time);            //填充时间的高八位
	buf[7] = servoID;                        //舵机ID
	buf[8] = GET_LOW_BYTE(Position);         //填充目标位置的低八位
	buf[9] = GET_HIGH_BYTE(Position);        //填充目标位置的高八位

	SerialX->write(buf, 10);
}

/*********************************************************************************
 * Function:  moveServos
 * Description： 控制多个舵机转动
 * Parameters:   servos[]:舵机结体数组，Num:舵机个数,Time:转动时间
                    0 < Num <= 32,Time > 0
 * Return:       无返回
 * Others:
 **********************************************************************************/
void LobotServoController::moveServos(LobotServo servos[], uint8_t Num, uint16_t Time)
{
	uint8_t buf[103];    //建立缓存
	if (Num < 1 || Num > 32 || !(Time > 0)) {
		return; //舵机数不能为零和大与32，时间不能为零
	}
	buf[0] = FRAME_HEADER;    //填充帧头
	buf[1] = FRAME_HEADER;
	buf[2] = Num * 3 + 5;     //数据长度 = 要控制舵机数*3+5
	buf[3] = CMD_SERVO_MOVE;  //填充舵机移动指令
	buf[4] = Num;             //要控制的舵机个数
	buf[5] = GET_LOW_BYTE(Time); //取得时间的低八位
	buf[6] = GET_HIGH_BYTE(Time); //取得时间的高八位
	uint8_t index = 7;
	for (uint8_t i = 0; i < Num; i++) { //循环填充舵机ID和对应目标位置
		buf[index++] = servos[i].ID; //填充舵机ID
		buf[index++] = GET_LOW_BYTE(servos[i].Position); //填充目标位置低八位
		buf[index++] = GET_HIGH_BYTE(servos[i].Position);//填充目标位置高八位
	}
	SerialX->write(buf, buf[2] + 2); //发送帧，长度为数据长度+两个字节的帧头
}

/*********************************************************************************
 * Function:  moveServos
 * Description： 控制多个舵机转动
 * Parameters:   Num:舵机个数,Time:转动时间,...:舵机ID,转动角，舵机ID,转动角度 如此类推
 * Return:       无返回
 * Others:
 **********************************************************************************/
void LobotServoController::moveServos(uint8_t Num, uint16_t Time, ...)
{
	uint8_t buf[128];
	va_list arg_ptr = NULL;
	va_start(arg_ptr, Time); //取得可变参数首地址
	if (Num < 1 || Num > 32 || (!(Time > 0)) || arg_ptr == NULL) {
		return; //舵机数不能为零和大与32，时间不能为零，可变参数不能为空
	}
	buf[0] = FRAME_HEADER;     //填充帧头
	buf[1] = FRAME_HEADER;
	buf[2] = Num * 3 + 5;      //数据长度 = 要控制舵机数 * 3 + 5
	buf[3] = CMD_SERVO_MOVE;   //舵机移动指令
	buf[4] = Num;              //要控制舵机数
	buf[5] = GET_LOW_BYTE(Time); //取得时间的低八位
	buf[6] = GET_HIGH_BYTE(Time); //取得时间的高八位
	uint8_t index = 7;
	for (uint8_t i = 0; i < Num; i++) { //从可变参数中取得并循环填充舵机ID和对应目标位置
		uint16_t tmp = va_arg(arg_ptr, uint16_t); //可参数中取得舵机ID
		buf[index++] = GET_LOW_BYTE(tmp); //貌似avrgcc中可变参数整形都是十六位
		                                  //再取其低八位
		uint16_t pos = va_arg(arg_ptr, uint16_t); //可变参数中取得对应目标位置
		buf[index++] = GET_LOW_BYTE(pos); //填充目标位置低八位
		buf[index++] = GET_HIGH_BYTE(pos); //填充目标位置高八位
	}
	va_end(arg_ptr);     //置空arg_ptr
	SerialX->write(buf, buf[2] + 2); //发送帧
}


/*********************************************************************************
 * Function:  runActionGroup
 * Description： 运行指定动作组
 * Parameters:   NumOfAction:动作组序号, Times:执行次数
 * Return:       无返回
 * Others:       Times = 0 时无限循环
 **********************************************************************************/
void LobotServoController::runActionGroup(uint8_t numOfAction, uint16_t Times)
{
	uint8_t buf[7];
	buf[0] = FRAME_HEADER;   //填充帧头
	buf[1] = FRAME_HEADER;
	buf[2] = 5;      //数据长度，数据帧除帧头部分数据字节数，此命令固定为5
	buf[3] = CMD_ACTION_GROUP_RUN; //填充运行动作组命令
	buf[4] = numOfAction;      //填充要运行的动作组号
	buf[5] = GET_LOW_BYTE(Times); //取得要运行次数的低八位
	buf[6] = GET_HIGH_BYTE(Times); //取得要运行次数的高八位
	SerialX->write(buf, 7);      //发送数据帧
}

/*********************************************************************************
 * Function:  stopActiongGroup
 * Description： 停止动作组运行
 * Parameters:   Speed: 目标速度
 * Return:       无返回
 * Others:
 **********************************************************************************/
void LobotServoController::stopActionGroup(void)
{
	uint8_t buf[4];
	buf[0] = FRAME_HEADER;     //填充帧头
	buf[1] = FRAME_HEADER;
	buf[2] = 2;                //数据长度，数据帧除帧头部分数据字节数，此命令固定为2
	buf[3] = CMD_ACTION_GROUP_STOP; //填充停止运行动作组命令

	SerialX->write(buf, 4);      //发送数据帧
}

/*********************************************************************************
 * Function:  setActionGroupSpeed
 * Description： 设定指定动作组的运行速度
 * Parameters:   NumOfAction: 动作组序号 , Speed:目标速度
 * Return:       无返回
 * Others:
 **********************************************************************************/
void LobotServoController::setActionGroupSpeed(uint8_t numOfAction, uint16_t Speed)
{
	uint8_t buf[7];
	buf[0] = FRAME_HEADER;     //填充帧头
	buf[1] = FRAME_HEADER;
	buf[2] = 5;                //数据长度，数据帧除帧头部分数据字节数，此命令固定为5
	buf[3] = CMD_ACTION_GROUP_SPEED; //填充设置动作组速度命令
	buf[4] = numOfAction;      //填充要设置的动作组号
	buf[5] = GET_LOW_BYTE(Speed); //获得目标速度的低八位
	buf[6] = GET_HIGH_BYTE(Speed); //获得目标熟读的高八位

	SerialX->write(buf, 7);      //发送数据帧
}


/*********************************************************************************
 * Function:  setAllActionGroupSpeed
 * Description： 设置所有动作组的运行速度
 * Parameters:   Speed: 目标速度
 * Return:       无返回
 * Others:
 **********************************************************************************/
void LobotServoController::setAllActionGroupSpeed(uint16_t Speed)
{
	setActionGroupSpeed(0xFF, Speed); //调用动作组速度设定，组号为0xFF时设置所有组的速度
}

/*********************************************************************************
 * Function:  getBatteryVoltage
 * Description： 发送获取电池电压命令
 * Parameters:   无输入参数
 * Return:       无返回
 * Others:
 **********************************************************************************/
void LobotServoController::getBatteryVoltage()
{
	uint8_t buf[4];
	buf[0] = FRAME_HEADER;         //填充帧头
	buf[1] = FRAME_HEADER;
	buf[2] = 2;                   //数据长度，数据帧除帧头部分数据字节数，此命令固定为2
	buf[3] = CMD_GET_BATTERY_VOLTAGE; //填充后的电池电压命令

	SerialX->write(buf, 4);        //发送数据帧
}


/*********************************************************************************
 * Function:  receiveHandle
 * Description： 处理串口接收数据
 * Parameters:   无输入参数
 * Return:       无返回
 * Others:
 **********************************************************************************/
void LobotServoController::receiveHandle()
{
	uint8_t buf[16];
	static uint8_t len = 0;
	static uint8_t getHeader = 0;
	if (!SerialX->available())
		return;
	//如果没有数据则返回
	do {
		switch (getHeader) {
		case 0:
			if (SerialX->read() == FRAME_HEADER)
				getHeader = 1;
			break;
		case 1:
			if (SerialX->read() == FRAME_HEADER)
				getHeader = 2;
			else
				getHeader = 0;
			break;
		case 2:
			len = SerialX->read();
			getHeader = 3;
			break;
		case 3:
			if (SerialX->readBytes(buf, len - 1) > 0)
				getHeader = 4;
			else{
				len = 0;
				getHeader = 0;
				break;
			}
		case 4:
			switch (buf[0]) {
			case BATTERY_VOLTAGE: //电池电压指令
				batteryVoltage = BYTE_TO_HW(buf[2], buf[1]); //高低八位组合成电池电压
				break;
			case ACTION_GROUP_RUNNING:         //有动作组被运行
				numOfActinGroupRunning = buf[1]; //获得运行中动作组号
				actionGroupRunTimes = BYTE_TO_HW(buf[3], buf[2]); //高低八位组合成运行次数
				isRunning = true; //设置运行中标识为真
				break;
			case ACTION_GROUP_STOPPED: //动作组停止
			case  ACTION_GROUP_COMPLETE://动作组运行完成
				isRunning = false; //设置运行中标识为假
				numOfActinGroupRunning = 0xFF; //设置运行中动作组号为0xFF
				actionGroupRunTimes = 0; //设置运行次数为0
				break;
			default:
				break;
			}
		default:
			len = 0;
			getHeader = 0;
			break;
		}
	} while (SerialX->available());
}
