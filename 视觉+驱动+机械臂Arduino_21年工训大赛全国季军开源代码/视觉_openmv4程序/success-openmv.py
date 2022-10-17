# uart to arduino - By: Poao - 周二 2月 2 2021

# openmv：  1）获取二维码、物料放置位置顺序，并串口输出  QRx_XXX/WLx_XXX
#           2）解算机械臂在原料区的抓取顺序，并串口输出       CTx_XXX (X = 1,2,3)
#      是否需要识别色环的顺序？ 即粗加工、半成品区色环的顺序，然后对应放置？
#
#      TIPS:  以原料区为例，有【左-1|中-2|右-3】三个位置放置有物料，对应三个动作组.
#             而所谓抓取顺序，即为机械臂【执行抓取动作组的顺序】。
#             如 312，即为先抓右侧、再抓左侧、最后抓中间。

import sensor, image, time, math,lcd
from pyb import Pin, Timer
#屏幕 320*240
height = 120
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
#sensor.set_windowing(320,240))
sensor.skip_frames(time = 2000)
sensor.set_auto_gain(False) # must be turned off for color tracking
sensor.set_auto_whitebal(False) # must be turned off for color tracking
clock = time.clock()
red_block_x=1
green_block_x=2
blue_block_x=3

message=000






lcd.init() # Initialize the lcd screen.


threshold_index = 0 # 0 for red, 1 for green, 2 for blue

# Color Tracking Thresholds (L Min, L Max, A Min, A Max, B Min, B Max)
# The below thresholds track in general red/green/blue things. You may wish to tune them...
thresholds = [(30, 83, 47, 127, -128, 127), # generic_red_thresholds
              (19, 93, -128, -26, -128, 127), # generic_green_thresholds
              (0, 100, -20, 126, -128, -40)] # generic_blue_thresholds



#################### Openmv数据处理 ###################
# 直接向arduino发送处理好的数据
# 红-1 绿-2 蓝-3

PP_s = "123"        # 色环顺序（固定） 逆时针方向

# 需要获取的顺序
above_order=0     # 上层物料抓取顺序
bottom_order=0  # 下层物料抓取顺序    二维码

Aorder=0          # 上层物料放置顺序
Border=0      # 下层物料放置顺序


Pos1 = "000"     # 上层放置位置
Pos2 = "000"     # 下层放置位置

QRCode1 = "000"  # 二维码，原料区上层搬运顺序
QRCode2 = "000"  # 二维码，原料区下层搬运顺序





def find_min(blobs):
    min_size=1000000
    for blob in blobs:
        if blob[2]*blob[3] <= min_size:
            min_blob=blob
            min_size = blob[2]*blob[3]
    return min_blob

# 输入： QRCode|任务码  Pos|物料放置位置
# 求 QRCode在Pos中的顺序
def Index_find(QRCode,Pos):   # 解算出机械臂的动作顺序
    result = 0
    for i in range(3):
        cc =  Pos.find(QRCode[i])+1
        #print(cc)
        result = result*10 + cc
    return result

#Move1 = Index_find(QRCode1,Pos1)    # 解算出机械臂抓取上层物料的动作顺序
#Move2 = Index_find(QRCode2,Pos2)	# 解算出机械臂抓取下层物料的动作顺序


#################### UART TO ARDUINO ###################
# 导入串口
from pyb import UART
# 串口3  [TX-P4, RX-P5]
uart = UART(3,9600,timeout_char = 50)   # 100 可改

# 串口收发数据
recv_data = ""     # 串口接收的数据 【 CM+QR|扫描二维码、CM+WL|
QR_flag = 0       # 扫描二维码标志位
WL_flag = 0       # 获取上层物料放置顺序标志位


# 串口发送 【QR1_XXX\QR2_XXX  WL1_XXX\WL2_XXX  CT1_XXX\CT2_XXX】
#  对应 QR|任务码  WL|物料位置  CT|机械臂抓取顺序（依据QR&WL计算）


def Uart_recv():  # 串口接收数据
    global QR_flag   # 将变量声明为全局变量，如此才可改变其数值
    global WL_flag

    if (uart.any()):   # 更新串口接收数据
        recv_data = eval(str(uart.read()))

        print(recv_data)
        #uart.write(recv_data)
        if ("CM+" in recv_data) :
            print("Openmv has recved CMD data.")
            if ("+QR" in recv_data):
                QR_flag = 1
                print("Ready for QRcode task !")

            if ("+WL" in recv_data):
                WL_flag = 1
                print("Ready for WLpose task !")





# 主循环
while(True):
    clock.tick()
    img = sensor.snapshot()

    Uart_recv() # 串口接收（接收arduino发送的指令）


    if(QR_flag):   # QR_flag
        # 1）进行二维码识别 放置相关函数
        #print("Start QRcode task !")

        # 放这里 放这里 放这里 放这里 放这里 放这里
        img.lens_corr(1.8) # strength of 1.8 is good for the 2.8mm lens.
        light = Timer(2, freq=50000).channel(1, Timer.PWM, pin=Pin("P6"))
        light.pulse_width_percent(10) # 控制亮度 0~100##############################################
        for code in img.find_qrcodes():
            print(code.payload())
            if message!=code.payload():   # MESSAGE = 0; 判断是否获取到二维码数据
           #    message=code.payload()
                #print("The order is:%s" %( message))
                String=list(str(code.payload()))
                above_order=String[0]+String[1]+String[2]
                bottom_order=String[4]+String[5]+String[6]

                # 2）当二维码识别成功后 【处理数据，发送数据】
                uart.write("QR_"+above_order+bottom_order+"\r\n")
                # time.sleep(50)  # 延时50ms
                # uart.write("QR2_"+bottom_order+"\r\n")
                print("QR_"+above_order+bottom_order+"\r\n")
                # time.sleep(50)  # 延时50ms
                #print("QR1_"+above_order)
                #print("QR2_"+bottom_order)

                # 3）复位 QR_flag
                QR_flag = 0
                print("QRcode task done!")

        img.draw_string(0,0,str(above_order)+"\r+\r"+str(bottom_order),color=(255,0,0),scale = 7,x_spacing=-16,y_spacing=-21)
        light.pulse_width_percent(0) # 控制亮度 0~100##############################################


    if(WL_flag): # 识别物料    WL_flag
        #uart.write("WL_"+Pos1+Pos2+"\r\n")

        clock.tick()
        light = Timer(2, freq=50000).channel(1, Timer.PWM, pin=Pin("P6"))
        light.pulse_width_percent(100) # 控制亮度 0~100
        img = sensor.snapshot()
        for r in img.find_blobs([thresholds[0]],roi=[0,140,320,100],pixels_threshold=200, area_threshold=200, merge=True):
            # These values depend on the blob not being circular - otherwise they will be shaky.

            # These values are stable all the time.
            img.draw_rectangle(r.rect())
            img.draw_cross(r.cx(), r.cy())
            # Note - the blob rotation is unique to 0-180 only.
            img.draw_keypoints([(r.cx(), r.cy(), int(math.degrees(r.rotation())))], size=20)
        #print(clock.fps())
            red_block_x = r.cx()

        for g in img.find_blobs([thresholds[1]],roi=[0,140,320,100], pixels_threshold=200, area_threshold=200, merge=True):
            # These values depend on the blob not being circular - otherwise they will be shaky.

            # These values are stable all the time.
            img.draw_rectangle(g.rect())
            img.draw_cross(g.cx(), g.cy())
            # Note - the blob rotation is unique to 0-180 only.
            img.draw_keypoints([(g.cx(), g.cy(), int(math.degrees(g.rotation())))], size=20)
        #print(clock.fps())
            green_block_x = g.cx()

        for b in img.find_blobs([thresholds[2]],roi=[0,140,320,100], pixels_threshold=200, area_threshold=200, merge=True):
            # These values depend on the blob not being circular - otherwise they will be shaky.

            # These values are stable all the time.
            img.draw_rectangle(b.rect())
            img.draw_cross(b.cx(), b.cy())
            # Note - the blob rotation is unique to 0-180 only.
            img.draw_keypoints([(b.cx(), b.cy(), int(math.degrees(b.rotation())))], size=20)
            blue_block_x = b.cx()
            #if g.cx()==None:
             #   print(1)
            #elif g.cx()!=None：
            if int(red_block_x)<int(green_block_x) and int(green_block_x)<int(blue_block_x):
                if Border!=123:
                        #print("上层物料顺序为：红绿蓝")
                        #print(red_block_x,green_block_x,blue_block_x)
                        #print(blob.h(),blob.w() )
                        Border=123
            elif int(blue_block_x)<int(green_block_x) and int(green_block_x)<int(red_block_x):
                if Border!=321:
                        #print("上层物料顺序为：蓝绿红")
                        #print(red_block_x,green_block_x,blue_block_x)
                        #print(blob.h(),blob.w() )
                        Border=321
            elif int(red_block_x)<int(blue_block_x) and int(blue_block_x)<int(green_block_x):
                if Border!=132:
                        #print("上层物料顺序为：红蓝绿")
                        #print(red_block_x,green_block_x,blue_block_x)
                        #print(blob.h(),blob.w() )
                        Border=132
            elif int(green_block_x)<int(blue_block_x) and int(blue_block_x)<int(red_block_x):
                if Border!=231:
                        #print("上层物料顺序为：绿蓝红")
                        # print(red_block_x,green_block_x,blue_block_x)
                        #print(blob.h(),blob.w() )
                        Border=231
            elif int(blue_block_x)<int(red_block_x) and int(red_block_x)<int(green_block_x):
                if Border!=312:
                        #print("上层物料顺序为：蓝红绿")
                        #print(red_block_x,green_block_x,blue_block_x)
                        #print(blob.h(),blob.w() )
                        Border=312
            elif int(green_block_x)<int(red_block_x) and int(red_block_x)<int(blue_block_x):
                if Border!=213:
                        #print("上层物料顺序为：绿红蓝")
                        #print(red_block_x,green_block_x,blue_block_x)
                        #print(blob.h(),blob.w() )
                        Border=213
            #print(Aorder)########################################得到下层物料
                #if int(red_block_x)>int(blue_block_x):
                 #   print(1)

                #a = int(green_block_x) - int(red_block_x)
                #b = int(red_block_x) - int(blue_block_x)
                #c = int(green_block_x) - int(blue_block_x)
                #print("%d,%d,%d,%d,%d,%d"%(int(red_block_x),int(green_block_x),int(blue_block_x),a,b,c))
                #print(Aorder)
            if (Border==123) or (Border==132) or (Border==213) or (Border==231) or (Border==312) or (Border==321):
                for r in img.find_blobs([thresholds[0]],roi=[0,0,320,100],pixels_threshold=200, area_threshold=200, merge=True):
                    # These values depend on the blob not being circular - otherwise they will be shaky.

                    # These values are stable all the time.
                    img.draw_rectangle(r.rect())
                    img.draw_cross(r.cx(), r.cy())
                    # Note - the blob rotation is unique to 0-180 only.
                    img.draw_keypoints([(r.cx(), r.cy(), int(math.degrees(r.rotation())))], size=20)
                    #print(clock.fps())
                    red_block_x = r.cx()

                for g in img.find_blobs([thresholds[1]],roi=[0,0,320,100], pixels_threshold=200, area_threshold=200, merge=True):
                    # These values depend on the blob not being circular - otherwise they will be shaky.

                    # These values are stable all the time.
                    img.draw_rectangle(g.rect())
                    img.draw_cross(g.cx(), g.cy())
                    # Note - the blob rotation is unique to 0-180 only.
                    img.draw_keypoints([(g.cx(), g.cy(), int(math.degrees(g.rotation())))], size=20)
                    #print(clock.fps())
                    green_block_x = g.cx()

                for b in img.find_blobs([thresholds[2]],roi=[0,0,320,100], pixels_threshold=200, area_threshold=200, merge=True):
                    # These values depend on the blob not being circular - otherwise they will be shaky.

                    # These values are stable all the time.
                    img.draw_rectangle(b.rect())
                    img.draw_cross(b.cx(), b.cy())
                    # Note - the blob rotation is unique to 0-180 only.
                    img.draw_keypoints([(b.cx(), b.cy(), int(math.degrees(b.rotation())))], size=20)
                    blue_block_x = b.cx()

                    if int(red_block_x)<int(green_block_x) and int(green_block_x)<int(blue_block_x):
                        if Aorder!=123:
                                #print("上层物料顺序为：红绿蓝")
                                #print(red_block_x,green_block_x,blue_block_x)
                                #print(blob.h(),blob.w() )
                                Aorder=123
                    elif int(blue_block_x)<int(green_block_x) and int(green_block_x)<int(red_block_x):
                        if Aorder!=321:
                                #print("上层物料顺序为：蓝绿红")
                                #print(red_block_x,green_block_x,blue_block_x)
                                #print(blob.h(),blob.w() )
                                Aorder=321
                    elif int(red_block_x)<int(blue_block_x) and int(blue_block_x)<int(green_block_x):
                        if Aorder!=132:
                                #print("上层物料顺序为：红蓝绿")
                                #print(red_block_x,green_block_x,blue_block_x)
                                #print(blob.h(),blob.w() )
                                Aorder=132
                    elif int(green_block_x)<int(blue_block_x) and int(blue_block_x)<int(red_block_x):
                        if Aorder!=231:
                                #print("上层物料顺序为：绿蓝红")
                                # print(red_block_x,green_block_x,blue_block_x)
                                #print(blob.h(),blob.w() )
                                Aorder=231
                    elif int(blue_block_x)<int(red_block_x) and int(red_block_x)<int(green_block_x):
                        if Aorder!=312:
                                #print("上层物料顺序为：蓝红绿")
                                #print(red_block_x,green_block_x,blue_block_x)
                                #print(blob.h(),blob.w() )
                                Aorder=312
                    elif int(green_block_x)<int(red_block_x) and int(red_block_x)<int(blue_block_x):
                        if Aorder!=213:
                                #print("上层物料顺序为：绿红蓝")
                                #print(red_block_x,green_block_x,blue_block_x)
                                #print(blob.h(),blob.w() )
                                Aorder=213

                #if int(Aorder)!=int(Border):
                    ##print("%dand%d"%(int(Aorder),int(Border)))
                    #Above=Aorder
                    #Below=Border

                print("%dand%d"%(int(Aorder),int(Border)))
                if int(Aorder)!= 0 and int(Border)!=0 :
                    print("OK!")

                    Pos1 = str(Aorder)
                    Pos2 = str(Border)

                    # 2）当物料识别成功后 【处理数据，发送数据】
                    print("WL_"+Pos1+Pos2)
                    uart.write("WL_"+Pos1+Pos2+"\r\n")
                    WL_flag = 0;
                    print("Done! ")
                    light.pulse_width_percent(0)

#######出现问题（1）：312+213








