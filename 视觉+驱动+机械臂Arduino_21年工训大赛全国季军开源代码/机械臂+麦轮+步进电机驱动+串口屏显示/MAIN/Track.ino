/*====================== 循迹 =================*/
// Move_xy(int x0, int y0, int x1, int y1)  //  x0,y0  --> x1,y1 二维坐标移动
// Track_Fun(int xy0, int xy1, char dir)    //  xy0 --> xy1     一维坐标移动
// trackingX()  tracking_X()  trackingY()  tracking_Y()  //  xy方向的循迹函数
// Move_zZ(int dir)     // 斜向行进

/*----------------------------------------------*
// 前、后、左、右 四个方向，左右两侧各一个
 *  Y轴
 *  |          LB    LF
 *  |      A - - - - - - - X            左
 *  |  BR^ |               |   FL    
 *  |  BL  |               |   FR      
 *  |      Z - - - - - - - Y            右
 *  |          RB    RF 
 *  0-- -- -- -- -- -- -- -- X轴
 *  
 ************************************************/

int S_FL = A0;         // 前方 
int S_FR = A2;
int S_RF = A4;
int S_RB = A6;
int S_BL = A8;
int S_BR = A10;
int S_LB = A12;
int S_LF = A14;

int _S_FL = A1;         // 这些是后来装模作样加的循迹。
int _S_FR = A3;
int _S_RF = A5;
int _S_RB = A7;
int _S_BL = A9;
int _S_BR = A11;
int _S_LB = A13;
int _S_LF = A15;

int count_Y = 0;  //Y向黑线计数
char count_X = 0;  //X向黑线计数

void XJ_test(){     // 用于测试循迹接线是否正确
  
//  Serial.println("S_FL:"+String(digitalRead(S_FL)));
//  Serial.println("S_FR:"+String(digitalRead(S_FR)));
//  //delay(500);
//
//  Serial.println("S_RF:"+String(digitalRead(S_RF)));
//  Serial.println("S_RB:"+String(digitalRead(S_RB)));
//
//  Serial.println("S_BL:"+String(digitalRead(S_BL)));
//  Serial.println("S_BR:"+String(digitalRead(S_BR)));
//
//  Serial.println("S_LB:"+String(digitalRead(S_LB)));
//  Serial.println("S_LF:"+String(digitalRead(S_LF)));

//   Serial.println("_S_FL:"+String(digitalRead(_S_FL)));
//  Serial.println("_S_FR:"+String(digitalRead(_S_FR)));
  //delay(500);
//
  Serial.println("_S_RF:"+String(digitalRead(_S_RF)));
  Serial.println("_S_RB:"+String(digitalRead(_S_RB)));
//
//  Serial.println("_S_BL:"+String(digitalRead(_S_BL)));
//  Serial.println("_S_BR:"+String(digitalRead(_S_BR)));

  Serial.println("_S_LB:"+String(digitalRead(_S_LB)));
  Serial.println("_S_LF:"+String(digitalRead(_S_LF)));
}


// 测试单走一格的脉冲距离  —— 先不用管这个是干嘛的
void test_moves()
{
Serial.println("begin: ");
int countss = 0;
float xxx= 0.021;
moveTo(xxx,0,2);
while((digitalRead(S_RB) && digitalRead(S_LB))==0)
  { 
    tracking_X();
    countss++;
    xxx = xxx + 0.001;
    }
  Serial.println("Count: "+String(countss));
  Serial.println("xxxxx: "+String(xxx));
    // +x: 301  286  287   298   0.32  0.29 
    // -x: 293 297  289   0.28  0.29  0.28 
    //     【平均 (290-300)*0.001 = 0.29-0.30    位移: 0.28-0.32】
}


/********************************
 *  2021/1/30     —— by Poao
 *  1） 基础循迹函数 trackingX()  tracking_X()  trackingY()  tracking_Y()
 *  2） 巡线函数  Move_xy(x0,y0,x1,y1)   | Track_Fun -- 循迹计数函数
 *  3） 采用定点巡线： 【以出发区为坐标原点建立 xoy 坐标系， 以十字交点为计数点。】
 *  tips ：基础循迹函数有待改进
 *  
 * 2CM   2.XX
 *  Y轴
 *  |          LB    LF
 *  |      A - - - - - - - X            左
 *  |  BR^ |               |   FL    
 *  |  BL  |               |   FR      
 *  |      Z - - - - - - - Y            右
 *  |          RB    RF 
 *  0-- -- -- -- -- -- -- -- X轴
 *    //Track_Fun(2,5,'Y')
 *
    #define MZX     7           //  对角线方向走
    #define MAY     8
    #define MXZ     9       
    #define MYA     10
 *********************************/

// Move_xy(int x0, int y0, int x1, int y1)  //  x0,y0  --> x1,y1 二维坐标移动
// Track_Fun(int xy0, int xy1, char dir)    //  xy0 --> xy1     一维坐标移动
// trackingX()  tracking_X()  trackingY()  tracking_Y()  //  xy方向的循迹函数
// Move_zZ(int dir)     // 斜向行进

/*---------------------------------------------*
 * 小车斜向行驶：右上 右下 左下 左上 || 1 2 3 4
 * 此为斜向行走一定距离后，先X调整，再Y调整，最后对中。
 *---------------------------------------------*/
float movezs = 0.46;   // 行进一段距离   
void Move_zZ(int dir)  // 先x再y
{
  float vv = linear_v;
  linear_v = 0.30;    // 斜向行进时的速度
  float mms = 0.005; 
  switch(dir){
    case 1:   // 右上
    moveTo(movezs, 0, 7);  // 右上行进   
    while(abs_yaw(yaw-0) > 1.5)       // 角度矫正先放低要求
       Set_yaw(yaw,0); 
    delay(20);
    
    linear_v = 0.15;   //  微调速度  不可太快，减小惯性。
    while(digitalRead(S_RF)!=1 && digitalRead(S_RF)!=1)  // 满足一个，退出while
    {
      moveTo(mms,0,1);   // x+向微调
      }
     moveTo(mms,0,1);   // x+向微调
    delay(20);    
    while(digitalRead(S_FL)!=1 && digitalRead(S_BL)!=1)  // 满足一个，退出while
    {
      moveTo(mms,0,3);   // Y+向微调
      }
     moveTo(mms,0,3);   // Y+向微调
     while(abs_yaw(yaw-0) > 1.0)       // 角度矫正先放低要求
       Set_yaw(yaw,0); 
//   最后进行对中操作
delay(30);  
     Adj_Init(1);  // X+ 向寻迹前，调整车身位置
     delay(30);  
     Adj_Init(3);  // Y+ 向寻迹前，调整车身位置
     
    break;
    case 2:   // 右下
    moveTo(movezs, 0, 8);  // 右下行进   
    while(abs_yaw(yaw-0) > 1.5)       // 角度矫正先放低要求
       Set_yaw(yaw,0); 
    delay(20);
    
    linear_v = 0.15;   //  微调速度  不可太快，减小惯性。
    while(digitalRead(S_RF)!=1 && digitalRead(S_RF)!=1)  // 满足一个，退出while
    {
      moveTo(mms,0,1);   // x+向微调
      }
    moveTo(mms,0,1);   // x+向微调
    delay(20);    
    while(digitalRead(S_FR)!=1 && digitalRead(S_BR)!=1)  // 满足一个，退出while
    {
      moveTo(mms,0,4);   // Y-向微调
      }
    moveTo(mms,0,4);   // Y-向微调
    while(abs_yaw(yaw-0) > 1.0)       // 角度矫正先放低要求
       Set_yaw(yaw,0); 
       delay(30);  
    Adj_Init(1);  // X+ 向寻迹前，调整车身位置
    delay(30);       
    Adj_Init(4);  // Y- 向寻迹前，调整车身位置 
    break;
    case 3:   // 左下   -- 用不上
    moveTo(movezs, 0, 9);  // 左下行进   
    while(abs_yaw(yaw-0) > 1.0)      // 车身恢复 0°，进行后续的行进
       Set_yaw(yaw,0);
    Adj_Init(1);  // X+ 向寻迹前，调整车身位置    
    Adj_Init(4);  // X+ 向寻迹前，调整车身位置 
    break;
    case 4:   // 左上
    moveTo(movezs, 0, 10);  // 左上行进    
    while(abs_yaw(yaw-0) > 1.5)       // 角度矫正先放低要求
       Set_yaw(yaw,0); 
    delay(20);
    
    linear_v = 0.15;   //  微调速度  不可太快，减小惯性。
    while(digitalRead(S_RB)!=1 && digitalRead(S_RB)!=1)  // 满足一个，退出while
    {
      moveTo(mms,0,2);   // x-向微调
      }
     moveTo(mms,0,2);   // x-向微调
    delay(20);   
    while(digitalRead(S_FL)!=1 && digitalRead(S_BL)!=1)  // 满足一个，退出while
    {
      moveTo(mms,0,3);   // Y+向微调
      }
     moveTo(mms,0,3);   // Y+向微调
    while(abs_yaw(yaw-0) > 1.0)       // 角度矫正先放低要求
       Set_yaw(yaw,0); 
    delay(30);  
    Adj_Init(1);  // X+ 向寻迹前，调整车身位置 
    delay(30);   
    Adj_Init(3);  // Y+ 向寻迹前，调整车身位置 
    break;
    
    default : 
    break;
      }
  linear_v = vv;
  }
  



void Move_step(double dxy,double dw,int dir, float mv)
{
  float keep = linear_v;
  linear_v = mv;
  moveTo(dxy,dw,dir);
  linear_v = keep;
  }


  
float abs_int(int x)   // 使用默认的 abs 容易出错
{
  if(x <0)
    x = -x;
  return x;
  }


/*--------------------------------------*
  点对点移动 ：（x0，y0） ——> （x1，y1）  （走直角） 
  (details: 路程长的一侧，先走)
  【先走一步再走个∟，再走剩下的，这个绕路方式仅针对第一次去原料区】
   绕路这个，这样的都会 (0,1) ——> (4,2)  【X+正向，且x走的距离大于y】
  --------------------------------------*/
void Move_xy(int x0, int y0, int x1, int y1)
{  // Move_xy(1,1,2,5)
  //  初始位置上电后，初始化当前角度为 0° 
  float err_yaw = 1.1;   // 即将转直角时，或移动结束后，矫正一下角度 【角度误差】
  
  int x = abs_int(x0-x1);
  int y = abs_int(y0-y1);
  if (x >= y)
  {   // 先走个“∟”，再走剩下的
    if (x0 < x1){   // X+ 方向  (0,1) ——> (4,2)
    Track_Fun(x0, x0+1, 'X');  // X 先走一步
    // 1) 矫正角度
    while(abs_yaw(yaw-0) > err_yaw)
       Set_yaw(yaw,0); 
       
    Track_Fun(y0, y1, 'Y');  // Y 走完
    // 2) 矫正角度
    while(abs_yaw(yaw-0) > err_yaw)
       Set_yaw(yaw,0); 
       
    Track_Fun(x0+1, x1, 'X');  // X 走完剩下的
    }
    else {      // X- 方向   (6,1) ——> (4,2)
    Track_Fun(x0, x1, 'X');  // X 先走
    // 3) 矫正角度
    while(abs_yaw(yaw-0) > err_yaw)
       Set_yaw(yaw,0); 
       
    Track_Fun(y0, y1, 'Y');  // Y 走完
    }
  }
  else
  {  // 7，4  4，6 
    Track_Fun(y0, y1, 'Y');  // Y 先走
    // 4) 矫正角度
    while(abs_yaw(yaw-0) > err_yaw)
       Set_yaw(yaw,0); 
       
    Track_Fun(x0, x1, 'X');  // Y 走完
   
  } 
}


/*--------------------------------------*
  四向循迹函数：前进 （xy1-xy0） 个格子
  （调用前小车应处于该方向的巡线位置。【Track_Fun(2,3,X)】）
   xy0 ——> xy1 ,dir = X,Y  
   V3.0的bug: 车身倾斜时，将计数错误！！  （车上倾斜时，两侧前侧循迹不能同时被触发）
   V4.0更改：寻迹计数改为单边触发，同时计数中加入累计位移矫正。
  --------------------------------------*/
  // 【平均 (290-300)*0.001 = 0.29-0.30    位移: 0.28-0.32】  寻迹一完整格的距离。
  float moves1 = 0.26 ;  // 判定为正常计数的累计行驶距离  0.28—0.40
  float moves2 = 0.34 ;
  float moves0 = 0.15 ;    // 判定错误计数
  
void Track_Fun(int xy0, int xy1, char dir)
{   //  Track_Fun(1,2,'X')   Track_Fun(2,1,'X')
  float slow_v = 0.92*linear_v;   // 0.7 -- 0.85 // 减速停车效果 -- 减速后距离较远（或可去除） （同侧两个循迹触发）
  float slow_v2 = 0.10;   // 0.08 - 0.11 // 二级减速 -- 计数的同时进行减速 （或可替代 一级减速）
  float linearv = linear_v;     // 正常运行速度记录
  
  bool first_tag = 0;     // 第一次较短距离行进标志位
  
  // 须知： 此次使用的单个循迹模块，遇到黑线时状态为 1 
  float ad_move = 0.0210;   // 调整距离，需测试。    
  // 这个距离对于再启动时计数准确度有影响。
  
  if(xy0 == xy1)    // 无效指令判断
  {
    // 不执行任何动作 
    }
  else {
  switch (dir)
  {
  case 'X':
    if (xy0 < xy1)   // 【判断 +/-】   // 1-》3
    {
    Adj_Init(1);  // X+ 向寻迹前，调整车身位置 
    
      //  开始时减速slow_v2慢行一段，而后linear_v速前进。 （减小突然启动时的冲击）
    Move_step(ad_move, 0, 1, slow_v2);  // 向X+ 一小步跨过黑线，以防重复计数（黑线宽度0.02）
    float moves = ad_move;   //累计计数间隔走过的距离
    // const double dx = 0.001;   // 每次行进距离
    
    // xy0-->xy1 (如：2-->3)
    for (; xy0 < xy1; ){
      trackingX();   //X正向循迹
      moves = moves + 0.001;  // 累计行进距离

       // 减速操作：针对【最后一格】时，【车头前侧循迹到车中部靠前循迹】的这段距离。 （用以确保停车的精准度）
      if(digitalRead(S_FL) && digitalRead(S_FR) && abs_int(xy1-xy0)==1 )      // 减速操作
      {
        linear_v = slow_v;
      }


        if(digitalRead(S_RF) || digitalRead(S_LF))  // 车中间遇到黑线，根据累计行程进行判断
        {
        //first_tag 
        if(first_tag==0 && moves>moves0)
        {
          xy0++;     // 正常计数
          moves = 0; //清零累计行程
          first_tag = 1;
          }
         if(moves>=moves1 && moves<=moves2)   // 累计距离在容许区间内  
        {
          first_tag = 1;
          xy0++;     // 正常计数
          moves = 0; //清零累计行程
          }else if(moves <= moves0)   // <0.1 错误计数
          {
            // 不进行计数
            }else if( moves>moves2)  // >0.40 之前漏记了  
            {
              //int cc = 2*moves/(moves1+moves2);   //  0.9*2/(0.28+0.4) = 2.647 (取2)
              xy0 = xy0 + 2 ;     // 补 计数 
              moves = 0; //清零累计行程
              first_tag = 1;
              }
          
          
          if(abs(xy1-xy0)==0)    // 最后一小段距离，二级减速
              {
              linear_v = slow_v2;   // 二级减速
              //adjust_yaw(yaw,0); // 到达“十”字中心后矫正一下角度
              }
              
          // 作用：这个效果 “十” ，使小车可以继续四向循迹，即 黑线也同时落在左右两侧循迹之间。
          moveTo(ad_move, 0, 1);  // 向X+ 一小步跨过黑线，以防重复计数（黑线宽度0.02）
          
        }
        
      }
       moves = 0; //清零累计行程  
    }
    else
    {
      Adj_Init(2);  // X- 向寻迹前，调整车身位置 
      
       //  开始时减速slow_v2慢行一段，而后linear_v速前进。 （减小突然启动时的冲击）
    Move_step(ad_move, 0, 2, slow_v2);  // 向X- 一小步跨过黑线，以防重复计数（黑线宽度0.02）
       float moves = ad_move;   //累计计数间隔走过的距离
    // const double dx = 0.001;   // 每次行进距离
    
    // xy0-->xy1 (如：3-->2)
    for (; xy1 < xy0; ){
      tracking_X();   //X负向循迹
       moves = moves + 0.001;  // 累计行进距离
    
      // 减速操作：针对【最后一格】时，【车尾后侧循迹到车中部靠后循迹】的这段距离。 （用以确保停车的精准度）
      if(digitalRead(S_BL) && digitalRead(S_BR) && abs_int(xy1-xy0)==1 )      // 减速操作
      {
       linear_v = slow_v;
      }
      
      
        if(digitalRead(S_RB) || digitalRead(S_LB))  // 车中间遇到黑线
        {
          //first_tag 
        if(first_tag==0 && moves>moves0)
        {
          xy1++;     // 正常计数
          moves = 0; //清零累计行程
          first_tag = 1;
          }
         if(moves>=moves1 && moves<=moves2)   // 累计距离在容许区间内  
        {
          xy1++;     // 正常计数
          moves = 0; //清零累计行程
          first_tag = 1;
          }else if(moves <= moves0)   // <0.1 错误计数
          {
            // 不进行计数
            }else if( moves>moves2)  // >0.40 之前漏记了  
            {
             // int cc = 2*moves/(moves1+moves2);   //  0.9*2/(0.28+0.4) = 2.647 (取2)
              xy1 = xy1 + 2 ;     // 补 计数 
              moves = 0; //清零累计行程
              first_tag = 1;
              }
              
          //xy1++;  
          if(abs(xy1-xy0)==0)    // 最后一小段距离，二级减速
              linear_v = slow_v2;   // 二级减速
              
          // 作用：这个效果 “十” ，使小车可以继续四向循迹。
          moveTo(ad_move, 0, 2);  // 向X- 一小步跨过黑线，以防重复计数（黑线宽度0.02）
      //    while(abs(yaw-0) > 0.8)
       //       Set_yaw(yaw,0); 
              
        }
      }
      moves = 0; //清零累计行程  
    }
    
    break; 

  case 'Y':
    if (xy0 < xy1)
    {
      Adj_Init(3);  // Y+ 向寻迹前，调整车身位置 
      
       //  开始时减速slow_v2慢行一段，而后linear_v速前进。 （减小突然启动时的冲击）
    Move_step(ad_move, 0, 3, slow_v2);  // 向Y+ 一小步跨过黑线，以防重复计数（黑线宽度0.02）
      //moveTo(ad_move, 0, 3);  // 向Y+ 一小步跨过黑线，以防重复计数（黑线宽度0.02）
         float moves = ad_move;   //累计计数间隔走过的距离
    // const double dx = 0.001;   // 每次行进距离
    
      // xy0-->xy1 (如：2-->3)
      for (; xy0 < xy1; ){
        trackingY();   //Y正向循迹
         moves = moves + 0.001;  // 累计行进距离

    
      // 减速操作：针对【最后一格】时，【车身左侧循迹到车前后部分靠左侧循迹】的这段距离。 （用以确保停车的精准度）
      if(digitalRead(S_LF) || digitalRead(S_LB) && abs_int(xy1-xy0)==1 )      // 减速操作
      {
       
        linear_v = slow_v;
      }
        
        if(digitalRead(S_FL) && digitalRead(S_BR))  // 车中间遇到黑线
        {
           //first_tag 
        if(first_tag==0 && moves>moves0)
        {
          xy0++;     // 正常计数
          moves = 0; //清零累计行程
          first_tag = 1;
          }
         if(moves>=moves1 && moves<=moves2)   // 累计距离在容许区间内  
        {
          xy0++;     // 正常计数
          moves = 0; //清零累计行程
          first_tag = 1;
          }else if(moves <= moves0)   // <0.1 错误计数
          {
            // 不进行计数
            }else if( moves>moves2)  // >0.40 之前漏记了  
            {
              //int cc = 2*moves/(moves1+moves2);   //  0.9*2/(0.28+0.4) = 2.647 (取2)
              xy0 = xy0 + 2 ;     // 补 计数 
              moves = 0; //清零累计行程
              first_tag = 1;
              }
              
          //xy0++;  
          if(abs(xy1-xy0)==0)    // 最后一小段距离，二级减速
              linear_v = slow_v2;   // 二级减速
              
          // 作用：这个效果 “十” ，使小车可以继续四向循迹，即 黑线也同时落在左右两侧循迹之间。
          moveTo(ad_move, 0, 3);  // 向Y+ 一小步跨过黑线，以防重复计数（黑线宽度0.02）
       //  while(abs(yaw-0) > 0.8)
      //        Set_yaw(yaw,0); 
              
        }
      }
      moves = 0; //清零累计行程  
    }
    else
    {
      Adj_Init(4);  // Y- 向寻迹前，调整车身位置 
      
       //  开始时减速slow_v2慢行一段，而后linear_v速前进。 （减小突然启动时的冲击）
    Move_step(ad_move, 0, 4, slow_v2);  // 向Y- 一小步跨过黑线，以防重复计数（黑线宽度0.02）
    float moves = ad_move;   //累计计数间隔走过的距离
    // const double dx = 0.001;   // 每次行进距离
    
      // xy0-->xy1 (如：3-->2)
      for (; xy1 < xy0; ){
        tracking_Y();   //Y负向循迹
         moves = moves + 0.001;  // 累计行进距离
  
      // 减速操作：针对【最后一格】时，【车身右侧循迹到车前后部分靠右侧循迹】的这段距离。 （用以确保停车的精准度）
      if(digitalRead(S_RF) && digitalRead(S_RB) && abs_int(xy1-xy0)==1 )      // 减速操作
      {  
        linear_v = slow_v;
      }
        
        if(digitalRead(S_FR) || digitalRead(S_BL))  // 车中间遇到黑线
        {
           //first_tag 
        if(first_tag==0 && moves>moves0)
        {
          xy1++;     // 正常计数
          moves = 0; //清零累计行程
          first_tag = 1;
          }
        if(moves>=moves1 && moves<=moves2)   // 累计距离在容许区间内  
        {
          xy1++;     // 正常计数
          moves = 0; //清零累计行程
          first_tag = 1;
          }else if(moves <= moves0)   // <0.1 错误计数
          {
            // 不进行计数
            }else if( moves>moves2)  // >0.40 之前漏记了  
            {
              //int cc = 2*moves/(moves1+moves2);   //  0.9*2/(0.28+0.4) = 2.647 (取2)
              xy1 = xy1 + 2 ;     // 补 计数 
              moves = 0; //清零累计行程
              first_tag = 1;
              }
              
          //xy1++;  
          if(abs(xy1-xy0)==0)    // 最后一小段距离，二级减速
              linear_v = slow_v2;   // 二级减速
              
          // 作用：这个效果 “十” ，使小车可以继续四向循迹。
          moveTo(ad_move, 0, 4);  // 向Y- 一小步跨过黑线，以防重复计数（黑线宽度0.02）
       //   while(abs(yaw-0) > 0.8)
       //       Set_yaw(yaw,0); 
              
        }
      }
      moves = 0; //清零累计行程  
    }

    break;
  
  default:
    break;
  }
}

  linear_v = linearv;    // 恢复正常速度
}


// trackingX()  tracking_X()  trackingY()  tracking_Y()

float yaw1 = 1.2;   // 触发矫正角
float yaw2 = 0.875;   // 矫正精度
/*--------------------------------------*
               前向寻迹行驶
  --------------------------------------*/
void trackingX() {
  
  const double dx = 0.001;      // 向前行进
  const double dw = 0.005;        // 矫正偏移
  if ((!digitalRead(S_FL) && !digitalRead(S_FR)) || (digitalRead(S_FL) && digitalRead(S_FR))) {
    moveTo(dx, 0, 1); //+X平移
  }
  else if (digitalRead(S_FL)) {  //前左传感器触发，车往右偏，需逆时针调整
    
    moveTo(0, dw, 6);   //逆时针
    moveTo(dx, 0, 1);
      
  }
  else if (digitalRead(S_FR)) {  //前右传感器触发，车往左偏，需顺时针调整
   
    moveTo(0, dw, 5);  //顺时针
    moveTo(dx, 0, 1);
    
  }

// 每次循迹后校准一下角度
if(abs_yaw(yaw-0) > yaw1)
{
    Set_yaw(yaw,0); 
    Set_yaw(yaw,0); 
}


}


/*--------------------------------------*
              后向寻迹行驶(平移)
  --------------------------------------*/
void tracking_X() {
  const double dx = 0.001;
  const double dw = 0.005;
  if ((!digitalRead(S_BL) && !digitalRead(S_BR)) || (digitalRead(S_BL) && digitalRead(S_BR))) {
    moveTo(dx, 0, 2); //-X平移
  }
  else if (digitalRead(S_BL)) {   //后左传感器触发，车往右偏，需逆时针调整
    moveTo(0,dw, 6);  //逆时针
    moveTo(dx, 0, 2);
  }
  else if (digitalRead(S_BR)) {   //后右传感器触发，车往左偏，需顺时针调整
    moveTo(0,dw, 5);  //顺时针
    moveTo(dx, 0, 2);
  }
  //linear_v=runingSpeed;

// 每次循迹后校准一下角度
if(abs_yaw(yaw-0) > yaw1)
{
  //while(abs(yaw-0) > yaw2)
    Set_yaw(yaw,0); 
    Set_yaw(yaw,0); 
}

}



/*--------------------------------------*
        左横向寻迹行驶  
  --------------------------------------*/
void trackingY() {
  const double dx = 0.001;
  const double dw = 0.005;
  //linear_v = 0.1;
  if ((!digitalRead(S_LB) && !digitalRead(S_LF)) || (digitalRead(S_LB) && digitalRead(S_LF))) {
    moveTo(dx, 0, 3); //+Y平移
  }
  else if (digitalRead(S_LB)) {
    moveTo(dx, 0, 6);  //逆时针
    moveTo(dx, 0, 3);
  }
  else if (digitalRead(S_LF)) {
    moveTo(dx, 0, 5); //顺时针
    moveTo(dx, 0, 3);
  }
  // linear_v = runingSpeed;

 // 每次循迹后校准一下角度
if(abs_yaw(yaw-0) > yaw1)
{
  //while(abs(yaw-0) > yaw2)
    Set_yaw(yaw,0); 
    Set_yaw(yaw,0); 
}

}



/*--------------------------------------*
              右横向寻迹行驶(平移)
  --------------------------------------*/
void tracking_Y() {
  const double dx = 0.001;
  const double dw = 0.005;
  
  //linear_v = 0.1;
  if ((!digitalRead(S_RB) && !digitalRead(S_RF)) || (digitalRead(S_RB) && digitalRead(S_RF))) {
    moveTo(dx, 0, 4); //-Y平移
  }
  else if (digitalRead(S_RF)) {
    moveTo(dx, 0, 6); //右前触发向左转
    moveTo(dx, 0, 4);
  }
  else if (digitalRead(S_RB)) {
    moveTo(dx, 0, 5); //右后触发向右转
    moveTo(dx, 0, 4);
  }
  // linear_v = runingSpeed;

// 每次循迹后校准一下角度
if(abs_yaw(yaw-0) > yaw1)
{
  //while(abs(yaw-0) > yaw2)
    Set_yaw(yaw,0); 
    Set_yaw(yaw,0); 
}

}

// ***************** 寻迹一定距离 ******************* //
//  这个没啥用，只是舍不得删除，哈哈哈。
/*--------------------------------------*
               前向寻迹行驶
  --------------------------------------*/
void trackingXX(float stepy) {
  
  const double dx = 0.001;      // 向前行进
  const double dw = 0.005;        // 矫正偏移
  int times = stepy/dx ;   // 需要持续寻迹的次数
  for(int i=0; i<times; i++)
  {
  if ((!digitalRead(S_FL) && !digitalRead(S_FR)) || (digitalRead(S_FL) && digitalRead(S_FR))) {
    moveTo(dx, 0, 1); //+X平移
  }
  else if (digitalRead(S_FL)) {  //前左传感器触发，车往右偏，需逆时针调整
    
    moveTo(0, dw, 6);   //逆时针
    moveTo(dx, 0, 1);
      
  }
  else if (digitalRead(S_FR)) {  //前右传感器触发，车往左偏，需顺时针调整
   
    moveTo(0, dw, 5);  //顺时针
    moveTo(dx, 0, 1);
    
  }

// 每次循迹后校准一下角度
if(abs(yaw-0) > yaw1)
{
  //while(abs(yaw-0) > yaw2)
    Set_yaw(yaw,0); 
    Set_yaw(yaw,0); 
}
  }

}


/*--------------------------------------*
              后向寻迹行驶(平移)
  --------------------------------------*/
void tracking_XX(float stepy) {
  const double dx = 0.001;
  const double dw = 0.005;
  
  int times = stepy/dx ;   // 需要持续寻迹的次数
  for(int i=0; i<times; i++)
  {
  if ((!digitalRead(S_BL) && !digitalRead(S_BR)) || (digitalRead(S_BL) && digitalRead(S_BR))) {
    moveTo(dx, 0, 2); //-X平移
  }
  else if (digitalRead(S_BL)) {   //后左传感器触发，车往右偏，需逆时针调整
    moveTo(0,dw, 6);  //逆时针
    moveTo(dx, 0, 2);
  }
  else if (digitalRead(S_BR)) {   //后右传感器触发，车往左偏，需顺时针调整
    moveTo(0,dw, 5);  //顺时针
    moveTo(dx, 0, 2);
  }
  //linear_v=runingSpeed;

// 每次循迹后校准一下角度
if(abs(yaw-0) > yaw1)
{
  //while(abs(yaw-0) > yaw2)
    Set_yaw(yaw,0); 
    Set_yaw(yaw,0); 
}
  }
}



/*--------------------------------------*
              左横向寻迹行驶
  --------------------------------------*/
void trackingYY(float stepy) {
  const double dx = 0.001;   // 每次行进距离
  const double dw = 0.005;

  int times = stepy/dx ;   // 需要持续寻迹的次数
  for(int i=0; i<times; i++)
  {
  const double dx = 0.001;
  const double dw = 0.01;
  //linear_v = 0.1;
  if ((!digitalRead(S_LB) && !digitalRead(S_LF)) || (digitalRead(S_LB) && digitalRead(S_LF))) {
    moveTo(dx, 0, 3); //+Y平移
  }
  else if (digitalRead(S_LB)) {
    moveTo(dx, 0, 6);  //逆时针
    moveTo(dx, 0, 3);
  }
  else if (digitalRead(S_LF)) {
    moveTo(dx, 0, 5); //顺时针
    moveTo(dx, 0, 3);
  }
  // linear_v = runingSpeed;

 // 每次循迹后校准一下角度
if(abs(yaw-0) > yaw1)
{
  //while(abs(yaw-0) > yaw2)
    Set_yaw(yaw,0); 
    Set_yaw(yaw,0); 
}
  }
  
}




/*--------------------------------------*
            右横向寻迹行驶(平移)
  --------------------------------------*/
void tracking_YY(float stepy) {

  const double dx = 0.001;   // 每次行进距离
  const double dw = 0.005;

  int times = stepy/dx ;   // 需要持续寻迹的次数
  for(int i=0; i<times; i++)
  {
  
  //linear_v = 0.1;
  if ((!digitalRead(S_RB) && !digitalRead(S_RF)) || (digitalRead(S_RB) && digitalRead(S_RF))) {
    moveTo(dx, 0, 4); //-Y平移
  }
  else if (digitalRead(S_RF)) {
    moveTo(dx, 0, 6); //右前触发向左转
    moveTo(dx, 0, 4);
  }
  else if (digitalRead(S_RB)) {
    moveTo(dx, 0, 5); //右后触发向右转
    moveTo(dx, 0, 4);
  }
  // linear_v = runingSpeed;

// 每次循迹后校准一下角度
if(abs(yaw-0) > yaw1)
{
  //while(abs(yaw-0) > yaw2)
    Set_yaw(yaw,0); 
    Set_yaw(yaw,0); 
}
  }

  
}


//************** 加入姿态角校准 ***************//
// 2021/2/8   by poao 

//  寻迹前进行微调
void Adj_Init(int dir)
{
  float small = 0.0005;   // 每次微调的距离
  float keep = linear_v;
  linear_v = 0.08;    // 设定微调时的速度
    
switch(dir)
{
  case 1:    // X+ 向寻迹的调整
      // 将往 +X 向行进，使用 FL/FR 进行寻迹     
    while(digitalRead(S_FL) != digitalRead(S_FR))   // 初始时，寻迹方向存在左偏或右偏
    {     // 碰黑线时为 1
      if(digitalRead(S_FL) == 1)  // 右侧偏移，需往左调整
        moveTo(small,0,3);   // +Y 向调整 1mm
      if(digitalRead(S_FR) == 1)  // 左侧偏移，需往右调整
        moveTo(small,0,4);   // -Y 向调整 1mm
      } 
  break;

  case 2:     // X- 向寻迹的调整
      // 将往 -X 向行进，使用 BL/BR 进行寻迹     
    while(digitalRead(S_BL) != digitalRead(S_BR))   // 初始时，寻迹方向存在左偏或右偏
    {     // 碰黑线时为 1
      if(digitalRead(S_BL) == 1)  // 右侧偏移，需往左调整
        moveTo(small,0,4);   // -Y 向调整 1mm
      if(digitalRead(S_BR) == 1)  // 左侧偏移，需往右调整
        moveTo(small,0,3);   // +Y 向调整 1mm
      } 
  break;

  case 3:     // Y+ 向寻迹的调整
    // 将往 +Y 向行进，使用 LB/LF进行寻迹     
    while(digitalRead(S_LB) != digitalRead(S_LF))   // 初始时，寻迹方向存在左偏或右偏
    {     // 碰黑线时为 1
      if(digitalRead(S_LB) == 1)  // 右侧偏移，需往左调整
        moveTo(small,0,2);   // -X 向调整 1mm
      if(digitalRead(S_LF) == 1)  // 左侧偏移，需往右调整
        moveTo(small,0,1);   // +X 向调整 1mm
      } 
  break;

  case 4:     // Y- 向寻迹的调整
  // 将往 -Y 向行进，使用 RB/RF进行寻迹     
    while(digitalRead(S_RB) != digitalRead(S_RF))   // 初始时，寻迹方向存在左偏或右偏
    {     // 碰黑线时为 1
      if(digitalRead(S_RB) == 1)  // 右侧偏移，需往左调整
        moveTo(small,0,2);   // -X 向调整 1mm
      if(digitalRead(S_RF) == 1)  // 左侧偏移，需往右调整
        moveTo(small,0,1);   // +X 向调整 1mm
      } 
  break;

  default :
  break;
  
  }

  linear_v = keep;   // 恢复原有速度
  
}
