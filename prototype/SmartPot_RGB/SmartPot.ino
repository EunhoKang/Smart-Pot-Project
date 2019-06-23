#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

int songDelays[]={
  1300,
  2680,
  3700,
};
int songDelaysCount=3;

//3색 RBG 설정
int r=0;
int g=0;
int b=0;
int sensorPin = A1; // Sensor ouput
int revPin=A2;
int sensorValue = 0; // Sensor Data
//초음파 센서 핀설정
int trigPin = 9;
int echoPin = 8;
//mp3출력핀 설정
SoftwareSerial mySoftwareSerial(10, 11); 
DFRobotDFPlayerMini myDFPlayer;
//토양센서 설정
int soil = A0;
//미세먼지 센서(4,5)

void setup() {
    Serial.begin (9600);
    mySoftwareSerial.begin(9600);
    Serial.println();
    Serial.println(F("DFRobot DFPlayer Mini Demo"));
    Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
    if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
      Serial.println(F("Unable to begin:"));
      Serial.println(F("1.Please recheck the connection!"));
      Serial.println(F("2.Please insert the SD card!"));
      while(true);
    }
    Serial.println(F("DFPlayer Mini online."));
    
    myDFPlayer.setTimeOut(500);
    myDFPlayer.volume(30); //0~30-----------------------------------------
    //초음파 센서 핀설정
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    delay(1000);

}

void rgb(int r, int g, int b){
  analogWrite(3,r);
  analogWrite(5,g);
  analogWrite(6,b);
}

void change_color(){
  int sensorValue= analogRead(sensorPin);
  int SoundColor=constrain(sensorValue,300,500);
  SoundColor=map(SoundColor,300,500,10,255);
  int val=analogRead(revPin);

  if(val<180){
    rgb(r,SoundColor,SoundColor);
  }else if(val>=180 && val<360){
    rgb(r,g,SoundColor);
  }else if(val>=360 && val<540){
    rgb(SoundColor,g,SoundColor);
  }else if(val>=540 && val<720){
    rgb(SoundColor,g,b);
  }else if(val>=720 && val<900){
    rgb(SoundColor,SoundColor,b);
  }else if(val>=900){
    rgb(255,255,255);
  }
}

//초음파로 터치 인식, 인식이 되는지 안되는지 TF반환
int get_touched(){
  float duration, distance;
  digitalWrite(trigPin, HIGH);
  delay(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = ((float)(340 * duration) / 10000) / 2;
  delay(50);
  if(distance>0 && distance < 10)
  {
    Serial.println("pot touched");
    return 1;
  }
  return 0;
}

// 토양 수분센서로부터 측정된 값을 읽습니다. 음원에 따른 딜레이
int soil_mois(){
  //int value = analogRead(soil);
  Serial.print("read sensor value : ");
  int val=analogRead(soil);
  Serial.println(val);
  val=constrain(val,0,700);
  val=map(val,0,700,6,0);
  val=constrain(val,1,5);
  myDFPlayer.playFolder(11,val);
  return 7000;
}

int simple_msg(){
  Serial.println("Sample msg");
  myDFPlayer.playFolder(10,1);
  return 3000;
}

int music_question(){
  Serial.println("If you want to listen to music, touch again.");
  myDFPlayer.playFolder(10,2);
  return 4000;
}

int goto_music(){
  Serial.println("goto music");
  myDFPlayer.playFolder(10,3);
  return 4000;
}

int turn_off(){
  Serial.println("sleep");
  myDFPlayer.volume(30);
  myDFPlayer.pause();
  myDFPlayer.playFolder(10,4);
  return 4000;
}

int music_player(int delay_input){
  myDFPlayer.pause();
  myDFPlayer.playFolder(12,delay_input+6);

  return songDelays[delay_input];
}

int msgCount=-1;
int PendCount=0;
int stateAlarm=0;
int songState=0;
int waitforNext=0;
int isPlaying=0;

void loop() {
  if(msgCount==-1){
    rgb(0,0,0);
    myDFPlayer.volume(30);
    if(get_touched()){
      rgb(80,254,255);
      delay(simple_msg());
      msgCount++;
    }else{
      delay(100);
    }
  }else if(msgCount==0){
    rgb(255,0,250);
    if(stateAlarm==0){
      delay(soil_mois());
      delay(music_question());
      stateAlarm=1;
    }
    if(get_touched()){
      rgb(255,255,0);
      delay(goto_music());
      msgCount++;
      stateAlarm=0;
    }else{
      delay(100);
      PendCount++;
      if(PendCount>50){
        rgb(0,0,0);
        delay(turn_off());
        PendCount=0;
        msgCount--;
        stateAlarm=0;
      }
    }
  }else if(msgCount==1){
    myDFPlayer.volume(25);
    if(waitforNext<=0){
      if(isPlaying==0){
        waitforNext=music_player(songState % songDelaysCount);
        isPlaying=1;
      }else{
        songState++;
        isPlaying=0;
      }
    }else{
      waitforNext--;
    }
    if(get_touched()){
      rgb(0,0,0);
      delay(turn_off());
      msgCount-=2;      
      stateAlarm = 0;
      waitforNext=0;
      isPlaying=0;
    }
    change_color();
  }
}
