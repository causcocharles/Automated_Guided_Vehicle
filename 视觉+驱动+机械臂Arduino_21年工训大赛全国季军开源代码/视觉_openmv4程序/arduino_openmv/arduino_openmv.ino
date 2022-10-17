/********************************************
//  tips: 使用mega2560的硬件串口一与openmv进行通讯
//  参考文章： https://zhuanlan.zhihu.com/p/367614669
//  联系方式： 知乎 “Poao”
//                    2021/4/25  —— by Poao
**********************************************/

/************ openmv-uart1 *************/
// 发送  CM+XX   XX = QR|扫描二维码 、 WL|获取物料放置位置
// 接收  QR_XXXXXX  WL_XXXXXX   (X = 1、2、3)
// 对应 QR|任务码  WL|物料位置           
String WL1_pose;    // 上层物料放置位置
String WL2_pose;    // 下层物料放置位置
String QR1_code;    // 上层物料任务码
String QR2_code;    // 上层物料任务码

char uart_recv[64];     // 串口缓存数据

void setup() {
    /*--------------硬件串口初始化----------------*/
  Serial.begin(115200);

  //uart1 ---  to openmv4 [TX-P4, RX-P5]
  Serial1.begin(9600);

// 这是需要调用的函数    【包括：给openmv发送指令，等待openmv传回数据】
   Get_message('Q');     // 获取二维码
   Get_message('P');     // 获取物料位置 
}

void loop() {
  // put your main code here, to run repeatedly:

}
