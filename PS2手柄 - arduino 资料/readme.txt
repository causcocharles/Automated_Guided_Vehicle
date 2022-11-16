PS手柄 - arduino 通信

1、准备好PS2手柄及arduino UNO R3控制板

2、安装好arduino IDE 后，将PS2X_lib.zip解压到C:\Users\Administrator\Documents\Arduino\libraries文件夹中！

3、按照连接图连接好接收器与控制板（关键一步，基本上都是在这里出错导致无法通信）

4、烧写例程

5、手柄安装上电池后，打开开关 - 电源红灯亮，如果绿灯未亮，点击MODE按钮！打开后，手柄与接收器会自动配对，成功配对后手柄红绿灯常亮，接收器灯常亮。

6、打开串口监视器，点击各个按钮，确定数据通信正常！

如果不正常，检查上面 2、3、4、5步骤！！