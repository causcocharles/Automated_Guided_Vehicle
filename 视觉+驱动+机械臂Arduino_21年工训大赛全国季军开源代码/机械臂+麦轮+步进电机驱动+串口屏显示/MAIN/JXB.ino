
// 包含两部分内容：  机械臂抓取顺序计算  ||  机械臂执行动作的判断函数

//  part1 -- 计算顺序    
// void Compute_xj();

inline int Compute(char* str1, char* str2)  // 解算 str1 in str2 的顺序  -- 基本函数
{    // 返回整型的顺序  
//   Serial.println(F("jxxxxx"));
  int sum = 0;
   for(int i=0; i<3;i++)
  {
    char cc = str1[i];
    char* xx = strchr(str2, cc);    // 搜寻cc在str2中顺序
    int pos = xx - str2 +1;         // 取得所需顺序 1、2、3...
    sum = sum*10 + pos;             // 计算顺序对应的整型数值
    }
    return sum;                     // 返回值
  }
  
//   计算相关顺序： SS1_order、SS2_order、XX1_order、XX2_order
inline  void Compute_xj()     // 直接调用
{
//  Serial.println(F("Ready for xj"));
 Serial.println(F("Ready for xj"));
  // 1、将 string 转换为 char*      【char* xx = string.c_str();】  
//  char* SeHuan = SeH_pose.c_str();     // 色环顺序
  char* WL1 = WL1_pose.c_str();        // 上层物料顺序
  char* WL2 = WL2_pose.c_str();        // 下层物料顺序
  char* QR1 = QR1_code.c_str();        // 任务码1顺序
  char* QR2 = QR2_code.c_str();        // 任务码2顺序
  
  // 2、解算顺序，并保存为String
  int aa = Compute(QR1,WL1);
  int bb = Compute(QR1,SeHuan);

  int cc = Compute(QR2,WL2);
  int dd = Compute(QR2,SeHuan);

// 这里为何如此简单粗暴？ 请前文系列文章找答案 —— 当时被逼无奈
   A[0]= aa/100+'0';   // 123/100 = 1
   A[1]= aa/10%10+'0';   // 123/10%10 = 2
   A[2]= aa%10+'0';   // 123%10 = 3
   A[3] ='\0';
   B[0]= bb/100+'0';   // 123/100 = 1
   B[1]= bb/10%10+'0';   // 123/10%10 = 2
   B[2]= bb%10+'0';   // 123%10 = 3
   B[3] ='\0';
   C[0]= cc/100+'0';   // 123/100 = 1
   C[1]= cc/10%10+'0';   // 123/10%10 = 2
   C[2]= cc%10+'0';   // 123%10 = 3
   C[3] ='\0';
   D[0]= dd/100+'0';   // 123/100 = 1
   D[1]= dd/10%10+'0';   // 123/10%10 = 2
   D[2]= dd%10+'0';   // 123%10 = 3
   D[3] ='\0';

  Serial.print(F("SS2_order: "));
  Serial.println(A);  Serial.println(aa);
  Serial.print(F("SS2_order: "));
  Serial.println(B);Serial.println(bb);
  Serial.print(F("XX1_order: "));
  Serial.print(C);Serial.println(cc);
  Serial.print(F("XX2_order: "));
  Serial.println(D);Serial.println(dd);
  
//  Serial.println("XX1_order: "+ cc + "     XX2_order: "+ dd);
  }


// part2 -- 机械臂抓取动作
// QR_shibie();
// Shibie_Wuliao();
//  动作组 3 + 3 +3 +3 +3 +3 +3（抓取放置动作值 21）     + 1+1  （扫描二维码+识别物料动作值 2）
//  void Shangceng_Yuanliaoqu_Zhua(String inString);               // 抓取上层物料
//  void Xiaceng_Yuanliaoqu_Zhua(String inString);                 // 抓取下层物料
//  void Fangdao_Cujiagongqu(String inString);                     // 放置到粗加工区
//  void Cujiagongqu_Zhuaqu(String inString);                      // 从粗加工区抓取
//  void Fangdao_Banchengpinqu_Shangceng(String inString);         // 上层物料放到半成品区
//  void Fangdao_Banchengpinqu_Xiaceng(String inString);           // 下层物料放到半成品区
/*
 SMQ          扫描二维码前  SMQ--扫描前 首拼
 KWL          识别物料顺序前  KWL--看物料 首拼
 FWH          识别物料顺序后，复位状态  FWH--复位后 首拼
 YLS_123_321  原料区上层物料抓取
 YLX_123_321  原料区上层物料抓取
 JGQ_123_321  粗加工区抓取物料放到小车上
 JGF_123_321  粗加工区物料放到色环
 BCS_123_321  半成品区上层物料抓取
 BCX_123_321  半成品区下层物料抓取
 */
 void QR_shibie(){
    
            myse.runActionGroup(22,1);  //第22组为识别二维码
  
     recv_OK(); // 完成动作后的反馈 
//      recv_OK(); // 完成动作后的反馈 
}
 void QR_shibiehao(){
    
            myse.runActionGroup(23,1);  //第23组为识别二维码
  
     recv_OK(); // 完成动作后的反馈 
      recv_OK(); // 完成动作后的反馈 
}
 void Shibie_Wuliao(){
    
            myse.runActionGroup(24,1);  //第24组为识别物料状态
      
       recv_OK(); // 完成动作后的反馈 
        recv_OK(); // 完成动作后的反馈 
}
void Shibie_WuliaoHou(){
    
            myse.runActionGroup(25,1);  //第25组为识别物料完成后，复位状态
      
       recv_OK(); // 完成动作后的反馈 
//       recv_OK(); // 完成动作后的反馈 
}


inline void Shangceng_Yuanliaoqu_Zhua(){     //函数名为  上层 原料 抓  原料区上层物料抓取
     Serial.println(F("Ready for YLS"));
     if(A[0]=='1' &&A[1]=='2' &&A[2]=='3' )
     {
      myse.runActionGroup(1,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(0,0);       
      myse.runActionGroup(2,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(1,0); 
      myse.runActionGroup(3,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(2,0);    
      }
     else  if(A[0]=='1' &&A[1]=='3' &&A[2]=='2' )
     {
      myse.runActionGroup(1,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(0,0);       
      myse.runActionGroup(3,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(1,0); 
      myse.runActionGroup(2,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(2,0);    
      }
      else if(A[0]=='2' &&A[1]=='3' &&A[2]=='1' )
     {
      myse.runActionGroup(2,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(0,0);       
      myse.runActionGroup(3,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(1,0); 
      myse.runActionGroup(1,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(2,0);    
      }
      else  if(A[0]=='2' &&A[1]=='1' &&A[2]=='3' )
     {
      myse.runActionGroup(2,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(0,0);       
      myse.runActionGroup(1,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(1,0); 
      myse.runActionGroup(3,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(2,0);    
      }
      else  if(A[0]=='3' &&A[1]=='1' &&A[2]=='2' )
     {
      myse.runActionGroup(3,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(0,0);       
      myse.runActionGroup(1,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(1,0); 
      myse.runActionGroup(2,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(2,0);    
      }
      else  if(A[0]=='3' &&A[1]=='2' &&A[2]=='1' )
     {
      myse.runActionGroup(3,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(0,0);       
      myse.runActionGroup(2,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(1,0); 
      myse.runActionGroup(1,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(2,0);    
      }
 
}

////num作为索引，判断执行的顺序。4，5，6是抓取顺序，8，9，10是放置顺序，4对应的8，5对应的9，6对应的10
inline void Xiaceng_Yuanliaoqu_Zhua(){
 Serial.println(F("Ready for YLX"));
 if(C[0]=='1' &&C[1]=='2' &&C[2]=='3' )
     {
      myse.runActionGroup(4,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(0,1);       
      myse.runActionGroup(5,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(1,1); 
      myse.runActionGroup(6,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(2,1);    
      }
     else  if(C[0]=='1' &&C[1]=='3' &&C[2]=='2' )
     {
      myse.runActionGroup(4,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(0,1);       
      myse.runActionGroup(6,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(1,1); 
      myse.runActionGroup(5,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(2,1);    
      }
      else if(C[0]=='2' &&C[1]=='3' &&C[2]=='1' )
     {
      myse.runActionGroup(5,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(0,1);       
      myse.runActionGroup(6,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(1,1); 
      myse.runActionGroup(4,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(2,1);    
      }
      else  if(C[0]=='2' &&C[1]=='1' &&C[2]=='3' )
     {
      myse.runActionGroup(5,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(0,1);       
      myse.runActionGroup(4,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(1,1); 
      myse.runActionGroup(6,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(2,1);    
      }
      else  if(C[0]=='3' &&C[1]=='1' &&C[2]=='2' )
     {
      myse.runActionGroup(6,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(0,1);       
      myse.runActionGroup(4,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(1,1); 
      myse.runActionGroup(5,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(2,1);    
      }
      else  if(C[0]=='3' &&C[1]=='2' &&C[2]=='1' )
     {
      myse.runActionGroup(6,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(0,1);       
      myse.runActionGroup(5,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(1,1); 
      myse.runActionGroup(4,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
       Fangdao_Chuwucao(2,1);    
      }
      
}


////num作为索引，判断执行的顺序。4，5，6是抓取顺序，8，9，10是放置顺序，4对应的8，5对应的9，6对应的10。num+4为4+4=8，5+4=9,6+4=10
inline void Fangdao_Chuwucao(int num,bool tag){       //放到_储物槽,此函数只与    Shangceng_Yuanliaoqu_Zhua()  、Xiaceng_Yuanliaoqu_Zhua(）配合，完成抓取和放置
    Serial.println(F("Fangdao CWC"));
    Serial.println(num);
    if(tag == 0)
    {
    switch(B[num])
    {
       case '1':
          myse.runActionGroup(7,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈       
            Serial.println(F("7"));
            break;
          case '2':
          myse.runActionGroup(8,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈 
            Serial.println(F("8"));
            break;
          case '3':
          myse.runActionGroup(9,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈 
            Serial.println(F("9")); 
            break;
          default:
            Serial.println(F("Fangdao CJG Wrong"));
            break;  
      }
    }
    else{
        switch(D[num])
        {
       case '1':
          myse.runActionGroup(7,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈       
            Serial.println(F("7"));
            break;
          case '2':
          myse.runActionGroup(8,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈 
           Serial.println(F("8"));
            break;
          case '3':
          myse.runActionGroup(9,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈 
            Serial.println(F("9")); 
            break;
          default:
       Serial.println(F("Fangdao CJG Wrong"));
            break;  
      }
          }       
}


//接下来两个函数完成了  将物料放到粗加工区  和  把物料从粗加工区搬到车上  的工作
inline void Fangdao_Cujiagongqu(bool tag){//放到粗加工区
  Serial.println(F("Fangdao CJG"));
  if(tag == 0){   
   if(QR1_code[0]=='1' &&QR1_code[1]=='2' &&QR1_code[2]=='3' )
     {
      myse.runActionGroup(10,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(11,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(12,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
     else  if(QR1_code[0]=='1' &&QR1_code[1]=='3' &&QR1_code[2]=='2' )
     {
      myse.runActionGroup(10,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(12,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(11,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else if(QR1_code[0]=='2' &&QR1_code[1]=='3' &&QR1_code[2]=='1' )
     {
      myse.runActionGroup(11,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(12,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(10,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else  if(QR1_code[0]=='2' &&QR1_code[1]=='1' &&QR1_code[2]=='3' )
     {
      myse.runActionGroup(11,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(10,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(12,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else  if(QR1_code[0]=='3' &&QR1_code[1]=='1' &&QR1_code[2]=='2' )
     {
      myse.runActionGroup(12,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(10,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(11,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else  if(QR1_code[0]=='3' &&QR1_code[1]=='2' &&QR1_code[2]=='1' )
     {
      myse.runActionGroup(12,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(11,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(10,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
  }
  else{
    if(QR2_code[0]=='1' &&QR2_code[1]=='2' &&QR2_code[2]=='3' )
     {
      myse.runActionGroup(10,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(11,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(12,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
     else  if(QR2_code[0]=='1' &&QR2_code[1]=='3' &&QR2_code[2]=='2' )
     {
      myse.runActionGroup(10,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(12,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(11,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else if(QR2_code[0]=='2' &&QR2_code[1]=='3' &&QR2_code[2]=='1' )
     {
      myse.runActionGroup(11,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(12,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(10,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else  if(QR2_code[0]=='2' &&QR2_code[1]=='1' &&QR2_code[2]=='3' )
     {
      myse.runActionGroup(11,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(10,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(12,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else  if(QR2_code[0]=='3' &&QR2_code[1]=='1' &&QR2_code[2]=='2' )
     {
      myse.runActionGroup(12,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(10,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(11,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else  if(QR2_code[0]=='3' &&QR2_code[1]=='2' &&QR2_code[2]=='1' )
     {
      myse.runActionGroup(12,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(11,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(10,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
    
    }
  }


inline void Cujiagongqu_Zhuaqu(bool tag){

  if(tag == 0){   
   if(QR1_code[0]=='1' &&QR1_code[1]=='2' &&QR1_code[2]=='3' )
     {
      myse.runActionGroup(13,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(14,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(15,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
     else  if(QR1_code[0]=='1' &&QR1_code[1]=='3' &&QR1_code[2]=='2' )
     {
      myse.runActionGroup(13,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(15,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(14,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else if(QR1_code[0]=='2' &&QR1_code[1]=='3' &&QR1_code[2]=='1' )
     {
      myse.runActionGroup(14,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(15,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(13,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else  if(QR1_code[0]=='2' &&QR1_code[1]=='1' &&QR1_code[2]=='3' )
     {
      myse.runActionGroup(14,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(13,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(15,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else  if(QR1_code[0]=='3' &&QR1_code[1]=='1' &&QR1_code[2]=='2' )
     {
      myse.runActionGroup(15,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(13,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(14,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else  if(QR1_code[0]=='3' &&QR1_code[1]=='2' &&QR1_code[2]=='1' )
     {
      myse.runActionGroup(15,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(14,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(13,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
  }
  else{
    if(QR2_code[0]=='1' &&QR2_code[1]=='2' &&QR2_code[2]=='3' )
     {
      myse.runActionGroup(13,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(14,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(15,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
     else  if(QR2_code[0]=='1' &&QR2_code[1]=='3' &&QR2_code[2]=='2' )
     {
      myse.runActionGroup(13,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(15,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(14,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else if(QR2_code[0]=='2' &&QR2_code[1]=='3' &&QR2_code[2]=='1' )
     {
      myse.runActionGroup(14,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(15,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(13,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else  if(QR2_code[0]=='2' &&QR2_code[1]=='1' &&QR2_code[2]=='3' )
     {
      myse.runActionGroup(14,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(13,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(15,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else  if(QR2_code[0]=='3' &&QR2_code[1]=='1' &&QR2_code[2]=='2' )
     {
      myse.runActionGroup(15,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(13,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(14,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else  if(QR2_code[0]=='3' &&QR2_code[1]=='2' &&QR2_code[2]=='1' )
     {
      myse.runActionGroup(15,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(14,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(13,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
    
    }
     
}


//将物料放到半成品区
inline void Fangdao_Banchengpinqu_Shangceng(){
  
  if(QR1_code[0]=='1' &&QR1_code[1]=='2' &&QR1_code[2]=='3' )
     {
      myse.runActionGroup(16,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(17,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(18,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
     else  if(QR1_code[0]=='1' &&QR1_code[1]=='3' &&QR1_code[2]=='2' )
     {
      myse.runActionGroup(16,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(18,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(17,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else if(QR1_code[0]=='2' &&QR1_code[1]=='3' &&QR1_code[2]=='1' )
     {
      myse.runActionGroup(17,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(18,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(16,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else  if(QR1_code[0]=='2' &&QR1_code[1]=='1' &&QR1_code[2]=='3' )
     {
      myse.runActionGroup(17,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(16,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(18,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else  if(QR1_code[0]=='3' &&QR1_code[1]=='1' &&QR1_code[2]=='2' )
     {
      myse.runActionGroup(18,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(16,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(17,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else  if(QR1_code[0]=='3' &&QR1_code[1]=='2' &&QR1_code[2]=='1' )
     {
      myse.runActionGroup(18,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(17,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(16,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      
   
}
inline void Fangdao_Banchengpinqu_Xiaceng(){

  if(QR2_code[0]=='1' &&QR2_code[1]=='2' &&QR2_code[2]=='3' )
     {
      myse.runActionGroup(19,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(20,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(21,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
     else  if(QR2_code[0]=='1' &&QR2_code[1]=='3' &&QR2_code[2]=='2' )
     {
      myse.runActionGroup(19,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(21,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(20,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else if(QR2_code[0]=='2' &&QR2_code[1]=='3' &&QR2_code[2]=='1' )
     {
      myse.runActionGroup(20,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(21,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(19,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else  if(QR2_code[0]=='2' &&QR2_code[1]=='1' &&QR2_code[2]=='3' )
     {
      myse.runActionGroup(20,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(19,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(21,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else  if(QR2_code[0]=='3' &&QR2_code[1]=='1' &&QR2_code[2]=='2' )
     {
      myse.runActionGroup(21,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(19,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(20,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      else  if(QR2_code[0]=='3' &&QR2_code[1]=='2' &&QR2_code[2]=='1' )
     {
      myse.runActionGroup(21,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(20,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      myse.runActionGroup(19,1);  //运行1号动作组
            recv_OK(); // 完成动作后的反馈 
            recv_OK(); // 完成动作后的反馈
      }
      
     
}
