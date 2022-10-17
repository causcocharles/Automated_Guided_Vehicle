
// 1-YLS  2-YLX 3-JGQS 4-JCQX  5-BCS 6-BCX
/********************************
 *  2021/1/30     —— by Poao
 *  共计 9 个步骤:stepOne --> stepNine
 *  主调用函数：Move_xy， moveTo(微调距离) 
 *  行走路线： 出发+返回区—>moveTo || 中间路程 —> Move_xy(巡线，直行+直角) 
 *********************************/
// 正常巡线只能使小车在十字网格移动，下面的参数用于调整车到达指定的搬运点
float step_v = 0.16;    // 较低速进行，以便于控制车身不倾斜
float yawed = 0.8;   // 到达抓取物料地点时的角度矫正精度

float qr_y = 0.05;  // 物料顺序，机械臂，距离原料区距离。 
// 为便于机械臂抓取，设置的微调距离 【两次抓取是否需要单独对待？  原料区与半成品区】
float yl_x  =  0.1;    // 使机械臂处于中心位置(原料区)         //0.1 上 - 0.1  下-0.072
float yl_y  = 0.245;    // 调整机械臂距离原料区的距离     ！！  // 上-0.25   下- 0.125
float yl_x2 = 0;    // 调整机械臂距离原料区的距离     ！！2

float yl_y2 = 0.108;    // 调整机械臂距离原料区的距离     ！！  // 0.125  0.05仅调试

float cjg_x = 0.152;   // 调整机械臂距离粗加工区的距离   ！！
float cjg_Xx = 0.15;   // 调整机械臂距离粗加工区的距离   ！！
float cjg_y = 0;   // 使机械臂处于中心位置(粗加工)  【暂未使用】

float bcp_x = 0.00;   // 使机械臂处于中心位置(半成品区)  
float bcp_y = 0.000;   // 调整机械臂距离半成品区的距离   ！！
float bcp_y2 = 0.000;   // 调整机械臂距离半成品区的距离   ！！// 0.1


// 主要关注函数内容，一些当时写的注释并未对应修改（修改意义不大）
// 很多内容还是需要结合系列文章阅读的，不然看半天不知所以然。

/*--------------------------------------*
    步骤一：离开出发区,移动至（0，1）后
  --------------------------------------*/
void stepOne() {
  QR_shibie();
  delay(100);
  float keep = linear_v;
  linear_v = step_v;    // 低速行驶
  moveTo(0.3, 0, 7);  // 右上行进
  Adj_Init(1);  // X+ 向寻迹前，调整车身位置
  linear_v = keep;
}


/*--------------------------------------*
        步骤二：前去识别二维码
    （0，1)-->（1，1）+ X+(0.15m)
  --------------------------------------*/
void stepTwo() {

    Move_xy(0,1,1,1);    // （1，1)-->（1，1） 
    float keep = linear_v;
    linear_v = step_v;
    moveTo(0.15, 0, 1);  // X+ 使小车居于二维码板对面
    linear_v = keep;
    
     recv_OK(); // 完成动作后的反馈   第一个动作的1
    //  Openmv 进行二维码识别
    Get_message('Q');    // 获取任务码
    delay(50); 
    UART_update(1);
    Shibie_Wuliao();
    delay(50);
    
    // 使小车回到寻迹位置 （3,1）
    linear_v = step_v;
    moveTo(0.145, 0, 1);  
    Adj_Init(3);  // Y+ 向寻迹前，调整车身位置 
    Adj_Init(1);  // X+ 向寻迹前，调整车身位置 
    linear_v = keep;

}

/*--------------------------------------*
     步骤三：前往原料区（暂采取 直角弯|—|）
    （2，1） ---> (6, 2) + Y-(0.25--待调整) 
    坐标更新为 （6，1）
     暂为： 同一位置抓取3个物料
  --------------------------------------*/
void stepThree() {

    
//    Move_xy(3,1,6,2);  // （2，1） ---> (6, 2) 
     
    Move_zZ(1);  // 右上行进

    Move_xy(4,2,6,2);  // （4，2） ---> (6, 2)
    while(abs_yaw(yaw-0) > yawed)
       Set_yaw(yaw,0);    
    Adj_Init(1);  // X+ 向寻迹前，调整车身位置  
    Adj_Init(4);  // X+ 向寻迹前，调整车身位置    // 到点 【6，2】 ！！
    delay(50);
    
    float keep = linear_v;
    linear_v = step_v;    // 低速行进
    moveTo(yl_x, 0, 2);  // X- 使机械臂居于原料区中心  // 坐标更新为 （6，1）
    moveTo(qr_y, 0, 4);  // Y- 使小车靠近原料区   ---  物料识别
        
   Get_message('P');  // 获取物料位置
   delay(50);
   UART_update(2);
   Compute_xj();   // 计算并保存机械臂动作顺序
   delay(50);    // 停留等待2s

    moveTo(yl_y-qr_y, 0, 4);  // Y- 使小车靠近原料区  --- 准备抓取上层物料
    linear_v = keep;
    while(abs_yaw(yaw-0) > yawed)
       Set_yaw(yaw,0); 

    Shangceng_Yuanliaoqu_Zhua(); 
    delay(20);    // 停留等待2s

    // 使小车回到寻迹位置 （6,2）
    linear_v = step_v;
    moveTo(yl_x, 0, 1);  // X+ 使小车中心回到原料区中心 
    Adj_Init(3);  // Y+ 向寻迹前，调整车身位置 
    moveTo(yl_y, 0, 3);  // Y+ 使小车靠近寻迹位置
    Adj_Init(3);  // Y+ 向寻迹前，调整车身位置 
    linear_v = keep;
       
}


/*--------------------------------------*
     步骤四：前往粗加工区
    （6，2） ---> (6, 4) + X+(0.25--待调整) 
    坐标更新为 （7，4）
     暂为： 同一位置抓取、放置3个物料
  --------------------------------------*/
void stepFour() {

    Move_xy(6,2,6,4);  //（6，2） ---> (6, 4) 
    
    float keep = linear_v;
    linear_v = step_v;
    delay(10);
    Adj_Init(3);  // Y+ 向寻迹前，调整车身位置 
    Adj_Init(1);  // X+ 向寻迹前，调整车身位置 
    while(abs_yaw(yaw-0) > yawed)     //  【6，4】
       Set_yaw(yaw,0); 
    delay(50);
    
    moveTo(cjg_x, 0, 1);  // X+ 使小车靠近粗加工区 
    linear_v = keep;
    while(abs_yaw(yaw-0) > yawed)
       Set_yaw(yaw,0); 
    delay(50);
    Adj_Init(1);  // X+ 向寻迹前，调整车身位置 
    moveTo(0.001,0,3);   // 稍微往上一点
    delay(5);
    Adj_Init(1);  // X+ 向寻迹前，调整车身位置 
    
    // 机械臂进行放置、抓取
    Fangdao_Cujiagongqu(0);
    delay(20);    // 停留等待2s
    Cujiagongqu_Zhuaqu(0);

     // 使小车回到寻迹位置 （6，4） --  GAI
    linear_v = step_v;
    Adj_Init(4);  // Y- 向寻迹前，调整车身位置 
    moveTo(cjg_Xx, 0, 2);  // X- 使小车回到寻迹区域
    Adj_Init(3);  // Y+ 向寻迹前，调整车身位置      【6，4】
    Adj_Init(2);  // X- 向寻迹前，调整车身位置 
    linear_v = keep;
    delay(50);
    
}


/*--------------------------------------*
     步骤五：前往半成品区
    （7，4） ---> (4, 6) + Y+(0.1--待调整) 
    坐标更新为 （4，7）
     暂为： 同一位置放置3个物料
  --------------------------------------*/
void stepFive() {
    float keep = linear_v;
     
    Move_zZ(4);  // 左上行进 
    Move_zZ(4);  // 左上行进 
    
    linear_v = step_v;
//    Adj_Init(1);  // X+ 向寻迹前，调整车身位置   
        
    while(abs_yaw(yaw-90)>0.8)     // 车身旋转90°，以便于进行物料的放置动作
           Set_yaw(yaw,90); 
    delay(20);
    Adj_Init(1);  // X+ 向寻迹前，调整车身位置   
    delay(20);
    Adj_Init(4);  // X+ 向寻迹前，调整车身位置 
    delay(20);
    Adj_Init(1);  // X+ 向寻迹前，调整车身位置   
    delay(20);

    while(digitalRead(A1) == 1)   // 专门针对特殊情况
     {
      moveTo(0.001,0,3);
      }
    while(digitalRead(A3) == 1)   // 专门针对特殊情况
     {
      moveTo(0.001,0,4);
      }
      Adj_Init(1);  // X+ 向寻迹前，调整车身位置   
      
    linear_v = keep;
    
     

      
    // 机械臂进行放置
    Fangdao_Banchengpinqu_Shangceng();
    delay(20);    // 停留等待2s
    
    while(abs_yaw(yaw-0) > yawed)
       Set_yaw(yaw,0); 
    
    // 使小车回到寻迹位置 （4,6）
    Adj_Init(1);  // X+ 向寻迹前，调整车身位置     【【4，6】】
    Adj_Init(4);  // X+ 向寻迹前，调整车身位置 
    
}


/*--------------------------------------*
     步骤六：返回原料区 
    （4，7） ---> (6, 2) + Y-(0.25--待调整) 
    坐标更新为 （6，1）
     暂为： 同一位置抓取下层的3个物料
  --------------------------------------*/
void stepSix() {
  
   float keep = linear_v;

    Move_zZ(2);  // 右下行进 
    Move_zZ(2);  // 右下行进 

   linear_v = 0.28;  // 太快时，会急剧抖动
   Move_xy(6,4,6,2);  //（4，7） ---> (6, 2)    // 对角？？
   
    linear_v = step_v;    // 低速行进
    Adj_Init(1);  // X+ 向寻迹前，调整车身位置 
    Adj_Init(4);  // X+ 向寻迹前，调整车身位置 
    
    moveTo(yl_y2, 0, 4);  // Y- 使小车靠近原料区     
    moveTo(yl_x2, 0, 2);  // X- 使机械臂居于原料区中心  
    linear_v = keep;
    while(abs_yaw(yaw+90)>1.0)     // 车身旋转90°，以便于进行下层物料的抓取动作
       Set_yaw(yaw,-90); 
    
   //  进行机械臂的抓取
 Xiaceng_Yuanliaoqu_Zhua();
    delay(100);
    while(abs_yaw(yaw-0) > yawed)      // 车身恢复 0°，进行后续的行进
       Set_yaw(yaw,0); 
       
    // 使小车回到寻迹位置 （6，2）
    linear_v = step_v;
    moveTo(yl_x2, 0, 1);  // X+ 使小车中心回到原料区中心 
    Adj_Init(3);  // Y+ 向寻迹前，调整车身位置 
    moveTo(yl_y2, 0, 3);  // Y+ 使小车靠近寻迹位置
    Adj_Init(3);  // Y+ 向寻迹前，调整车身位置 
    linear_v = keep;
}

/*--------------------------------------*
     步骤七：前往粗加工区（第二趟）
    （6，2） ---> (6, 4) + X+(0.25--待调整) 
    坐标更新为 （7，4）
     暂为： 同一位置抓取、放置3个物料
  --------------------------------------*/
void stepSeven() {

    Move_xy(6,2,6,4);  //（6，2） ---> (6, 4) 
    
    float keep = linear_v;
    linear_v = step_v;
    moveTo(cjg_x, 0, 1);  // X+ 使小车靠近粗加工区 
    Adj_Init(1);  // Y+ 向寻迹前，调整车身位置 
    moveTo(0.001,0,3);   // 稍微往上一点
    delay(5);
    Adj_Init(1);  // X+ 向寻迹前，调整车身位置 
    // 这个使机械臂居于中心，这个待定是否需要调整  暂不考虑
    //moveTo(cjg_y, 0, 1);  // X+ 使小车靠近粗加工区 
    linear_v = keep;
    
    while(abs_yaw(yaw-0) > yawed)
       Set_yaw(yaw,0); 
       
    // 机械臂进行放置、抓取
    Fangdao_Cujiagongqu(1);
    delay(20);    // 停留等待2s
    Cujiagongqu_Zhuaqu(1);
    Adj_Init(1);  // X+ 向寻迹前，调整车身位置 
  
     // 使小车回到寻迹位置 （6，4）  --- GAI
    linear_v = step_v;
    Adj_Init(4);  // Y- 向寻迹前，调整车身位置 
    moveTo(cjg_Xx, 0, 2);  // X- 使小车回到寻迹区域
    linear_v = keep;
    delay(100);
}


/*--------------------------------------*
     步骤八：前往半成品区（第二趟）
    （6，4） ---> (4, 6) + Y+(0.1--待调整) 
    坐标更新为 （4，7）
     暂为： 同一位置放置3个物料
  --------------------------------------*/
void stepEight() {

    Move_zZ(4);  // 左上行进 
    Move_zZ(4);  // 左上行进   
    
    float keep = linear_v;
    linear_v = step_v;

    while(abs_yaw(yaw-90)>0.8)     // 车身旋转90°，以便于进行物料的放置动作
           Set_yaw(yaw,90); 
    delay(20);
    Adj_Init(1);  // X+ 向寻迹前，调整车身位置   
    delay(20);
    Adj_Init(4);  // X+ 向寻迹前，调整车身位置 
    delay(20);
    Adj_Init(1);  // X+ 向寻迹前，调整车身位置   
    delay(20);

     while(digitalRead(A1) == 1)   // 专门针对特殊情况
     {
      moveTo(0.001,0,3);
      }
      while(digitalRead(A3) == 1)   // 专门针对特殊情况
     {
      moveTo(0.001,0,4);
      }
      Adj_Init(1);  // X+ 向寻迹前，调整车身位置 
      
    linear_v = keep;
    
    // 机械臂进行放置
    Fangdao_Banchengpinqu_Xiaceng();
    delay(20);    // 停留等待2s
    while(abs_yaw(yaw-0) > yawed)
       Set_yaw(yaw,0); 

    Shibie_WuliaoHou();
    
     // 使小车回到寻迹位置 （4，6）
    linear_v = step_v;
    Adj_Init(2);  // X- 向寻迹前，调整车身位置  
    linear_v = keep;
}


/*--------------------------------------*
     步骤九：返回返回区（由半成品区）  *** 高速有问题
     (4, 6)  ---> (1, 7)
     X- 0.15 Y+ 0.15 (待调整为斜向走)
     暂为： 同一位置抓取下层的3个物料
  --------------------------------------*/
void stepNine() {
  
    Move_xy(4,6,3,6);  // (4, 6)  ---> (4, 7)
    Adj_Init(1);  // x+ 向寻迹前，调整车身位置
    Adj_Init(3);  // y+ 向寻迹前，调整车身位置
    
    Move_zZ(4);  // 左上行进 
    float keep = linear_v;
    linear_v = step_v;    // 降速行进
    linear_v = keep;
    
     moveTo(0.30, 0, 10);   // 左上
     moveTo(0.30, 0, 2);  // X- 往上走一格
     while(abs_yaw(yaw-0) > yawed)
       Set_yaw(yaw,0); 
    delay(2000);    // 停留等待2s
 
}






//  Other function
//  update_101()         // 更新hwt101的数据
//  UART_update(int i)   // 串口屏显示

/*============== 更新yaw ============*/ 
void update_101()
{
  while (Serial3.available()) 
  {
    JY901.CopeSerialData(Serial3.read()); //Call JY901 data cope function
   }

  Gyro_0 = Gyro;   // 记录之前的数据，用于pid整定
  yaw_0 = yaw;
  
  //print received data. Data was received in serialEvent;
  Gyro = (float)JY901.stcGyro.w[2]/32768*2000;
  yaw = (float)JY901.stcAngle.Angle[2]/32768*180;

  //Set_yaw(yaw,0);
  UART_update(0);  // 串口发送给串口屏显示
  //Serial.print("Gyro:");Serial.println(Gyro);
  //Serial.print("Angle:");Serial.println(yaw);
  
  }

/*============== 串口屏显示数据 ============*/ 
void UART_update(int i)
{
  switch(i)
  {
    case 1:   // 任务码
        UART.write("t1.txt=\"");     // 发送字符串命令
        UART.print(QR1_code);     // 发送字符串命令
        UART.print(QR2_code);     // 发送字符串命令
        UART.write("\"");     // 发送字符串命令
        UART.write(HexEnd,3);  //发送3个16进制ff结束符
    break;

    case 2:   // 物料放置顺序
        UART.write("t3.txt=\"");     // 发送字符串命令
        UART.print(WL1_pose);     // 发送字符串命令
        UART.print(WL2_pose);     // 发送字符串命令
        UART.write("\"");     // 发送字符串命令
        UART.write(HexEnd,3);                 //发送3个16进制ff结束符
    break;

//    case 3:    // 抓取顺序
//    
//    UART.print("t9.txt=\""+String(A)+"\"");     // 发送字符串命令
//    UART.write(HexEnd,3);     delay(1);         //发送3个16进制ff结束符
//    UART.print("t10.txt=\""+String(B)+"\"");     // 发送字符串命令
//    UART.write(HexEnd,3);     delay(1);   //发送3个16进制ff结束符
//    UART.print("t8.txt=\""+String(C)+"\"");     // 发送字符串命令
//    UART.write(HexEnd,3);     delay(1);   //发送3个16进制ff结束符
//    UART.print("t11.txt=\""+String(D)+"\"");     // 发送字符串命令
//    UART.write(HexEnd,3);     delay(1);   //发送3个16进制ff结束符
//    break;

    case 0:   // 发送yaw角。 
        UART.write("t13.txt=\"");     // 发送字符串命令
        UART.print(String(yaw));     // 发送字符串命令
        UART.write("\"");     // 发送字符串命令
        UART.write(HexEnd,3);                 //发送3个16进制ff结束符
//    UART.print("t13.txt=\""+String(yaw)+"\"");     // 发送字符串命令
    break;
    
    default:
    break;
    }
}




/******************************
 *      UART 相关函数
 * 
 *****************************/
// Uart_send_openmv(char CMD)    // 给openmv发送识别命令[不调用]
// Uart_recv_openmv()            // 接收openmv返回的数据[不调用]
// Get_message(char task)        // Q-二维码 P-物料位置 S-抓取顺序【调用】
// recv_OK()                     // 串口接收 机械臂完成动作的标志

void Uart_send_openmv(char CMD)
{
  //Serial1.listen();
  switch(CMD){
    case '1':
    Serial1.print("CM+QR");  // 识别二维码
    Serial.println("Send to openmv:CM+QR");
    break;

    case '2':
    Serial1.print("CM+WL");  // 识别物料上层位置
    Serial.println("Send to openmv:CM+WL");
    break;
    
    default :
    Serial.println("Wrong cmd to send.");
    break;
  }
  }



void Uart_recv_openmv()   // QR1/QR2   CT1/CT2
{
  if(Serial1.available()>0)
  {
   int i=0;
   while(Serial1.available()>0)
  {
    uart_recv[i]=Serial1.read();
    i++;
    delay(2);   // 延时3ms
   }
   
   if(uart_recv[0]=='Q'&&uart_recv[1]=='R')     // 获取二维码
   {
    QR1_code = String(uart_recv[3])+String(uart_recv[4])+String(uart_recv[5]);
    QR2_code = String(uart_recv[6])+String(uart_recv[7])+String(uart_recv[8]);
    
    Serial.println("Got QR_CODE:"+QR1_code+QR2_code);
      
    }


    if(uart_recv[0]=='W'&&uart_recv[1]=='L')     // 获取物料放置位置
    {
      WL1_pose = String(uart_recv[3])+String(uart_recv[4])+String(uart_recv[5]);
      WL2_pose = String(uart_recv[6])+String(uart_recv[7])+String(uart_recv[8]);
      
//    WL1[0] = uart_recv[3];    WL2[0] = uart_recv[6];
//    WL1[1] = uart_recv[4];    WL2[1] = uart_recv[7];
//    WL1[2] = uart_recv[5];    WL2[2] = uart_recv[8];
    
    Serial.println("Got WL:"+WL1_pose+WL2_pose);
      }

     // 输出接收到的数据
     Serial.println(uart_recv);
       } 
  }



void Get_message(char task)   // Q-二维码 P-物料位置 S-抓取顺序
{
  switch(task)
  {
    case 'Q':
    Uart_send_openmv('1');  // 发送扫描二维码的指令  √

     while(QR1_code=="" && QR2_code=="")
      {
        Uart_recv_openmv();

      }
      
//      UART_update(1);   // 串口屏显示任务码
      
//      Serial1.write("QRcode recved.");
      Serial.println("********** QRcode recved. ************");
//      Serial.println("QRcode:"+QR1_code+QR2_code);
    break;

    case 'P':
    Uart_send_openmv('2');  // 发送扫描上层物料的指令  √
    while(WL1_pose=="" || WL2_pose=="")
      Uart_recv_openmv();
//    UART_update(2);   // 串口屏显示物料顺序
//    Serial1.write("WLpose recved.");
    Serial.println("********** WLpose recved. ************");
//    Serial.println("WLpose:"+WL1_pose+WL2_pose);
    break;

    default : 
    break;
  }
  }

//  机械臂接收到执行命令后返回一次 UU , 执行完命令后再返回一次 UU .  需使用两个 recv_OK() !!
void recv_OK()            // 串口接收 机械臂完成动作的标志  
{
   char uart_recvs[6];
   bool tag=0;
   while(tag == 0)
   {
    if(Serial2.available()>0)
  {
   int iii=0;
   while(Serial2.available()>0)
  {
    uart_recvs[iii]=Serial2.read();
    iii++;
    delay(5);   // 延时3ms
   }
   //Serial.println(uart_recvs);
   if(uart_recvs[0] == 'U')
  {
    Serial.println(F("Task ok!"));
    tag = 1;
    }
  } 
  }

  
  }
