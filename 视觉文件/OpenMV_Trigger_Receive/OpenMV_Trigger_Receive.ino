#include <Ticker.h>
#include <string.h>
#define TXD1 27
#define RXD1 14

char str[10], begin, end, openMV_flag = 0, i, counter = 0;
int arr_mission[6], count_m = 0;
int arr_practical[6], count_p = 0;
int8_t mission_wait = 0, message_wait = 0;
int stage = 0;
Ticker timer1;

void String_To_Sequece(char* s, int *a);
void timerISR(void);

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200,SERIAL_8N1,RXD1,TXD1);
  timer1.attach_ms(10, timerISR);
  pinMode()
  delay(1000);
}

void loop() {
  switch(stage){
    case 0: MissionRequest(); break;
    case 1: delay(3000); stage++; break;
    case 2: MessageRequest(); break;
    case 3: Serial.println("Success!"); stage++; break;
  }

}


void MissionRequest(void){
  if(!mission_wait){
    mission_wait = 1;
    Serial1.write(1);
  }
  else if(mission_wait && end == 'p'){
    stage++;
    String_To_Sequece(str, arr_mission);
    mission_wait = 0;
    openMV_flag = 0;
    i = 0;
    counter = 0;
    Serial.print("Qrcode Received:");
    for(int j = 0; j < 6; j++){
      Serial.print(arr_mission[j]);
    }
    Serial.println("");
    while(Serial1.read() >= 0){} // 清空串口缓存
  }
}

void WaitForMission(void){
  if(mission_wait){
    counter++;
    if(counter>100){
      // Serial1.write(1);
      counter = 0;
    }
    if(Serial1.available()){
      if(!openMV_flag && (begin = Serial1.read()) == 'q'){
        openMV_flag = 1;
        i = 0;
      }
      else if(openMV_flag && i < 7){
        str[i++] = Serial1.read();
      }
      else if(openMV_flag && i == 7){
        end = Serial1.read();
        str[7] = '\0';
      }
    }
  }
}

void MessageRequest(void){
  if(!message_wait){
    message_wait = 1;
    Serial1.write(2);
  }
  else if(message_wait && end == 'y'){
    stage++;
    String_To_Sequece(str, arr_practical);
    Serial.print("Message Received:");
    for(int j = 0; j < 6; j++){
      Serial.print(arr_practical[j]);
    }
    Serial.println("");
    message_wait = 0;
    openMV_flag = 0;
    i = 0;
    counter = 0;
    while(Serial1.read() >= 0){} // 清空串口缓存
  }
}

void WaitForMessage(void){
  if(message_wait){
    counter++;
    if(counter>100){
      // Serial1.write(2);
      counter = 0;
    }
    if(Serial1.available()){
      if(!openMV_flag && (begin = Serial1.read()) == 'x'){
        openMV_flag = 1;
        i = 0;
      }
      else if(openMV_flag && i < 7){
        str[i++] = Serial1.read();
      }
      else if(openMV_flag && i == 7){
        end = Serial1.read();
        str[7] = '\0';
      }
    }
  }
}

void timerISR(void){
  WaitForMission();
  WaitForMessage();
}

void String_To_Sequece(char* s, int *a){
	for(int i = 0; i < 3; i++){
		a[i] = s[i] - '0';
	} 
	for(int i = 4; i < 7; i++){
		a[i-1] = s[i] - '0';
	} 
}