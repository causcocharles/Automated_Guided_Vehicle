  #include <Arduino.h> 
  #include <AccelStepper.h>  //本示例程序使用AccelStepper库

/*----------------------------------------------*
 *
 *  车身位置及传感器\电机接线:
 *  小车的方位及电电机安装位置，会直接影响麦轮的控制。
 *  Y轴
 *  |           ——> 前
 *  |  左 A - - - - - - - X              左
 *  |  ^  |               |  |     Head
 *  |  |  |               |  V     Head  
 *  |     Z - - - - - - - Y  右          右
 *  |           <—— 后
 *  0-- -- -- -- -- -- -- -- X轴
 *
 * 前后左右行进的麦轮控制：
 *     1-前——>x轴正向: X+ Y+ Z+ A+
 *     2-后——>x轴负向: X- Y- Z- A-
 *     3-左——>y轴正向: X- Y+ Z- A+
 *     4-右——>y轴负向: X+ Y- Z+ A-
 *     5-左偏调整——>【顺时针】: X+ Y- Z- A+ 
 *     6-右偏调整——>【逆时针】: X- Y+ Z+ A-
 * 
 ************************************************/
 
//void moveTo(double dxy,double dw,int dir);    // 主要关注 这个函数就可

//void stopHalSec();
//void intiMotors();
//inline  void  runToSpeed();

/*----------------------------------------------*
 *            步进电机端口宏定义（2*4杜邦线）
 *----------------------------------------------*/
// 奇数为 STEP  偶数为 Dir
  #define En_Pin  40  // 步进电机使能端
  #define X_Step  32  // X 电机步进引脚
  #define X_Dir   33  // X 电机步方向进引脚
  #define Y_Step  34 
  #define Y_Dir   35  
  #define Z_Step  36  
  #define Z_Dir   37  
  #define A_Step  38  
  #define A_Dir   39  

/*--------------------------------------------*
 *            步进电机控制对象定义
 *--------------------------------------------*/
  AccelStepper motora(1,A_Step,A_Dir);    //定义步进电机控制对象a
  AccelStepper motorx(1,X_Step,X_Dir); 
  AccelStepper motory(1,Y_Step,Y_Dir);
  AccelStepper motorz(1,Z_Step,Z_Dir);

/*------------------------------------------*
 *        步进电机参数定义(这个得对应修改)
 *------------------------------------------*/
//  全步进- 1.8° | 半步进- 0.9° | 1/4步进- 0.45° | 1/8步进- 0.225°  —— 配置跳线帽
//  一圈：  200步 | 400步 | 800步 | 1600步 

#define D_Wheel        0.0762        //车轮的直径  单位:m  (三英寸麦轮=7.62cm)

#define AllStep         200       //步进电机全步进一圈步数为200  
//#define MicroStep       4       //步进电机细分数（短接帽设置）
//#define TotalStep       800      //4细分下每圈步数:1600步
#define MicroStep       8         //步进电机细分数（短接帽设置）
#define TotalStep       1600      //8细分下每圈步数:1600步

#define ClockWise       HIGH    //顺时针转动
#define AntiClockWise   LOW     //逆时针转动
#define NotSendPluse    false   //不发送脉冲
#define SendPluse       true    //发送脉冲
#define StartUse        true    //开始启用
#define StopUse         false   //停止启用步进电机

/*---------------------------------------------*
 *              比率变量的设定
 *---------------------------------------------*/
const double C_Wheel=M_PI*D_Wheel;              //车轮的周长 单位：m (M_PI--3.1415...)
const double StepOfMeter=TotalStep/C_Wheel;     // 8细分时 步数/米 

const double CarMaxSpeed=6000;                 // 最大步进速度
// const double Acceleration=50;                  // 设置加速度  --- 没用
const double KMaxFre = 6000;                   // 每秒最大步数   单位：步/秒

// 下边这个不用管，后面与 angle_L 相关的也不用管
// ps: 步进电机的基本程序有一部分是参考别人的，其中这里便是。
//     问过写代码的那位，但是他也忘记这个是啥意思了,不过这并不影响后续的使用。

// 暂未明白该变量  设取 v=wr ，取angle_l = r = 410 mm = 0.410 m   90?
const double angle_L=0.410/90;                 // MAYBE?
// const double angle_L=0.323/90;                 // 将设置的小车线速度转化为整体角速度


/*--------------------------------------------*
 *           步进电机移动方向宏定义
 *--------------------------------------------*/
#define MFord     1         //移动方向 :前、后、左、右
#define MBack     2
#define MLeft     3
#define MRight    4
#define RLeft     5         // 用于寻迹时修正方向
#define RRight    6

#define MZX     7           // 可参考ppt - 对角线方向走   // 右上
#define MAY     8           // 右下
#define MXZ     9           // 左下
#define MYA     10          // 左上


/*-------------------------------------------*
 *           步进电机的初始设定方向-次
 *-------------------------------------------*/

 void defaultDir(){     //  正反转 、 步进与否 、 是否启用  （maybe）
    // 初始化正转(顺时针)、不发生脉冲步进、启用
    motorx.setPinsInverted(ClockWise,NotSendPluse,StartUse);//步进电机初始正转向为顺时针，是否发送步进脉冲步进，是否启用步进电机
    motory.setPinsInverted(ClockWise,NotSendPluse,StartUse);
    motorz.setPinsInverted(ClockWise,NotSendPluse,StartUse);
    motora.setPinsInverted(ClockWise,NotSendPluse,StartUse);
 }


/*-------------------------------------------*
 *           步进电机的最大速度-次
 *-------------------------------------------*/
void defaultMaxSpeed(){
  motorx.setMaxSpeed(CarMaxSpeed);
  motory.setMaxSpeed(CarMaxSpeed);
  motorz.setMaxSpeed(CarMaxSpeed);
  motora.setMaxSpeed(CarMaxSpeed);
}

/*-------------------------------------------*
 *            步进电机的初始化
 *-------------------------------------------*/
 void intiMotors(){                  //四个电机共用一个EN，任取其一控制即可
  motorx.setEnablePin(En_Pin);       //设置步进电机使能端口
  defaultDir();                      //设置步进电机初始方向
  defaultMaxSpeed();                 //设置步进电机最大速度
  motorx.enableOutputs();            //步进端口使能
 }


/*-----------------------------------------*
 *           步进电机运动到设定的速度
 *-----------------------------------------*/
inline  void  runToSpeed(){     // .setSpeed(Val) 之后调用。

    motorx.runSpeed();         // 匀速运动 -- 开始运动
    motory.runSpeed();
    motorz.runSpeed();
    motora.runSpeed();

  }



/*------------------------------------------*
 *      步进电机速度设置(运动方向)
 *------------------------------------------*/
//  根据小车按照情况，需对应修改方向
//    dir-运动方向（前后左右）  vxy-xy方向行进速度（直行）    vw-斜着走
inline void xySetVal(double vxy,double vw,int dir){
  // 步进电机以“步”驱动。   
  double v0=vxy*StepOfMeter;    //直行速度转化为步数的速率 —— 【m/s * 步/m = 步/s】

  double v1=vw*StepOfMeter;     //转动的速率转化为步数的速率  ——  斜向行进所需，用于修正偏移

  switch(dir){       // 设定行进方向集步进数
    case MFord:      // 1 前——>x轴正向: X+ Y+ Z+ A+
              motorx.setSpeed(v0);
              motory.setSpeed(v0);
              motorz.setSpeed(v0);
              motora.setSpeed(v0);
              break;
    case MBack:     // 2  后——>x轴负向: X- Y- Z- A-
              motorx.setSpeed(-v0);
              motory.setSpeed(-v0);
              motorz.setSpeed(-v0);
              motora.setSpeed(-v0);
              break;
    case MLeft:    //  3  左——>y轴正向: X- Y+ Z- A+
              motorx.setSpeed(-v0);
              motory.setSpeed(v0);
              motorz.setSpeed(-v0);
              motora.setSpeed(v0);
              break;
    case MRight:   //  4  右——>y轴负向: X+ Y- Z+ A-
              motorx.setSpeed(v0);
              motory.setSpeed(-v0);
              motorz.setSpeed(v0);
              motora.setSpeed(-v0);
              break;
              
              //  用于寻迹时修正方向
    case RLeft:      // 5 左偏调整——>顺时针: X+ Y- Z- A+ 
              motorx.setSpeed(v1);
              motory.setSpeed(-v1);
              motorz.setSpeed(-v1);
              motora.setSpeed(v1);
              break;
    case RRight:    // 6  右偏调整——>逆时针: X- Y+ Z+ A-
              motorx.setSpeed(-v1);
              motory.setSpeed(v1);
              motorz.setSpeed(v1);
              motora.setSpeed(-v1);
              break;

    case MZX:    // 7  Z-->X调整——>右上↗: X0 Y+ Z0 A+
              motorx.setSpeed(0);
              motory.setSpeed(v1);
              motorz.setSpeed(0);
              motora.setSpeed(v1);
              break;
              
     case MAY:    // 8  A-->Y调整——>右下↘: X+ Y0 Z+ A0
              motorx.setSpeed(v1);
              motory.setSpeed(0);
              motorz.setSpeed(v1);
              motora.setSpeed(0);
              break;
              
     case MXZ:    // 9  X-->Z调整——>左下↙: X0 Y- Z0 A-
              motorx.setSpeed(0);
              motory.setSpeed(-v1);
              motorz.setSpeed(0);
              motora.setSpeed(-v1);
              break;
              
     case MYA:    // 10  Y-->A调整——>左上↖: X- Y0 Z- A0
              motorx.setSpeed(-v1);
              motory.setSpeed(0);
              motorz.setSpeed(-v1);
              motora.setSpeed(0);
              break;
       
    default:break;
              }   
 }


/*---------------------------------------------*
 *               暂停(速度为零)
 *---------------------------------------------*/
void stopHalSec(){
  xySetVal(0, 0, 0);
  runToSpeed();
}


/*-----------------------------------------*
 *      步进电机移动固定距离-主
 *----------------------------------------*/
void moveTo(double dxy,double dw,int dir){

  if((dxy == 0) && (dw == 0)) return; 
  double t1 = dxy / linear_v;             // m/(m/s) = s  — 计算需要持续运行的时间
  
  dw=angle_L*dw;                          // 将角度运算转化为长度运行 ?
  double t2 = dw/linear_v;                // m/(m/s) = s  — 计算需要持续运行的时间
  double t_last =  max(t1,t2);            // dw=0时，t_last = t1.

  if(t_last==0)return;                    // 防止发生不可预料的错误

  xySetVal(linear_v,linear_v,dir);        // 运动速度一般固定，只是调整移动距离

  // 步进电机持续转动时间【t_last*1000*1.03】
  unsigned long delta_t = millis() + t_last*1000*1.03;      //  millis()-ms-1000   1.03-修正系数
  while(millis()<delta_t){
    runToSpeed();                 // 步进电机开始转动。（前边设定步进及速度）
  }
  stopHalSec();                   // 速度为零

 }

void Steeper_test()    // 字面意思
{
  // 电机接线说明，安装规矩来的。   电机红线接驱动板最上端
  moveTo(0.24,0,1);  // 向前走  ++++
  delay(2000);
  moveTo(0.24,0,2);  // 向后走  ----
  delay(2000);   //
  moveTo(0.24,0,3);  // 向左走
  delay(2000);
  moveTo(0.24,0,4);  // 向右走  
  delay(2000);
  moveTo(0.24,0,5);  // 顺时针  
  delay(2000);
  moveTo(0.24,0,6);  // 逆时针  
  delay(2000);
  
  moveTo(0.24,0,7);  // 右上  
  delay(2000);
  moveTo(0.24,0,8);  // 右下  
  delay(2000);
  moveTo(0.24,0,9);  // 左下  
  delay(2000);
  moveTo(0.24,0,10);  // 左上  
  delay(2000);
  
  
  }
 

// 2021/2/8   by poao 
/*---------------------------------------------*
 *   设定并调整姿态角（执行一次函数仅微调一次，非一步到位）
 *              控制误差 abs(△yaw)<0.8°
 *   加入pd控制 | 其后加入巡线环节（倒是是否加入其他调整方式？---不局限于顺时针、逆时针）
 *---------------------------------------------*/
 float Kp_Yaw = 0.040;   //  // +方向正确  0.036  --大幅度低频// 0.03 小震荡、0.04震荡加剧  0.05 反反复复
 float Kd_Yaw = -0.034;  //    1--【0.036&-0.030】     2--【0.040&-0.034】  ||  2 better
// kp ki *0.6 得到理论值   0.0216  0.006
  float Ki_Yaw = 0.0; 
// 此处仅使用 Kp Kd。 Kp--响应误差值的快慢，Kd--响应误差的变化（遏制超调震荡）
// 先调Kp，调至低频震荡；加入Kd，调至震荡消失。（对于此处的角度，Kd为-，当偏差较小时，其为主导，削弱Kp的作用，以遏制超调的发生）

void Set_yaw(float yaw, float target)
{   
  // 5 - 顺时针   6 - 逆时针
  // yaw + : 车身逆时针旋转   yaw - : 车身顺时针旋转
  // 0.00001 + 100ms + 0.04 +1.0  效果不错，用于修正小角度
  
 // 采取控制速度的方式，调整转角的速度快慢 linear_v = 0 -- 0.175 m/s
  float vmax = 0.3 ;       // 设定调整角度时的最大速度
  float yaw_v = linear_v;   // 保存原有速度设置
  float moves = 0.00001;    // 每次调整的微小距离
//  linear_v = Kp_Yaw*err + Ki_Yaw*err_integral + Kd_Yaw*err_err;  // 用于车身旋转的速度

 static float err, err_integral, Last_err, err_err;
  err = yaw - target;           // 偏差                    P
  err_err = err - Last_err;     // 偏差的微分--偏差的偏差   D
//  Last_err = err;
//  err_integral += err;          // 偏差的积分--偏差和      I
  linear_v = Kp_Yaw*err + Kd_Yaw*err_err;  // 用于车身旋转的速度

  if(linear_v >= 0 && abs_yaw(err) > 0.8)
  {
   if(linear_v > vmax)
        linear_v = vmax;
    
    moveTo(moves,0,5);   // 顺时针调整
    }
    else if(linear_v<0 && abs_yaw(err) > 0.8 )
    {   
      linear_v = -linear_v;
      if(linear_v > vmax)
          linear_v = vmax;
      
      moveTo(moves,0,6);   // 逆时针调整
      }
      
  linear_v = yaw_v;  // 恢复之前设定的速度

}


// 2021/2/9   by poao 
/*---------------------------------------------*
 *     设定并调整姿态角（一步调整到位）
 *         控制误差 abs(△yaw)<0.8°
 *   加入pd控制 | 加入巡线环节 Track_Fun，即每次执行完矫正一次角度（是否有必要，待调试）
 *---------------------------------------------*/
void adjust_yaw(float yaw, float target)
{
  while(abs_yaw(yaw-target) > 0.8)
  {
    Set_yaw(yaw,target);
    }
  
  }


/*---------------------------------------------*
 *     设定并调整姿态角 （太慢了，未引入pid）
 *  abs(△yaw)<1.0   暂且是慢慢调整，并非一次调整到位
 *    暂不使用，设定角度请使用 【Set_yaw(yaw,target)】
 *---------------------------------------------*/
/*
void Hold_yaw(float yaw,float target) 
{   
  // 5 - 顺时针   6 - 逆时针
  // yaw + : 车身逆时针旋转   yaw - : 车身顺时针旋转
  // 0.00001 + 100ms + 0.04 +1.0  效果不错，用于修正小角度
  float yaw_v = linear_v;   // 替换原有速度设置
  linear_v = 0.04;          // 用于车身旋转的速度
  float moves = 0.00001;  
  float sub_yaw = yaw - target;
 
    if(sub_yaw > 1.0)   // 车身逆时针偏转
  {
    moveTo(moves,0,5);   // 顺时针调整
    } 
   else if(sub_yaw < 0 && abs(sub_yaw) > 1.0)
    {
      moveTo(moves,0,6);   // 逆时针调整
      }
    
  linear_v = yaw_v;  // 恢复之前设定的速度
 
    //  【之前调试的内容】
    //  moveTo(0.0001,0,5);    旋转
    //  0.005  -- 1    0.0005 -- 0.2   
    //  0.0001  -- 0.03   速度设定： 0.01 - 0.1 效果雷同
    //  delay(1500);

    // 0.00001  + 100ms + 0.05 + 2.0 效果很好（±0.5）
    //   0.0001 也尚可
    //  0.00001 | 0.0001  + 0.15 (小车来回颠簸--速度太快)  【0.1时也可能颠簸】
    // 0.00001  + 100ms + 0.05 + 1.0   偶有震荡  无需pid ，降低速度即可？ 
    //  速度 微调距离 yaw采样频率
    // 0.000002  依旧有震荡 --- 速度对震荡影响大
    // 0.00001 + 0.03 有些慢，精准度还可
    // 暂取 0.00001 + 100ms + 0.04 +1.0  效果不错，用于修正小角度 
}

*/

 /*---------------------------------------------*
 *          步进电机使能开关
inline void xyStop(){xySetVal(0, 0, 0); runToSpeed(); motorx.disableOutputs();}//步进电机使能关闭
inline void xyStar(){motorx.enableOutputs();}//步进电机使能开启   
     
  这段程序应该是不需要的，直接调用 motorx.enableOutputs() 就可。
*---------------------------------------------*/
