#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT11

int songDelays[] = {
  22,
  2200,
  2200,
};
int songDelaysCount = 3;

//온습도 센서 설정
DHT dht(DHTPIN, DHTTYPE);
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
  dht.begin();
  mySoftwareSerial.begin(9600);
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true);
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.setTimeOut(500);
  myDFPlayer.volume(20); //0~30-----------------------------------------
  //초음파 센서 핀설정
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  delay(1000);

}

//초음파로 터치 인식, 인식이 되는지 안되는지 TF반환
int get_touched() {
  float duration, distance;
  digitalWrite(trigPin, HIGH);
  delay(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = ((float)(340 * duration) / 10000) / 2;
  delay(100);
  if (distance > 0 && distance < 10)
  {
    Serial.println("pot touched");
    return 1;
  }
  return 0;
}

// 토양 수분센서로부터 측정된 값을 읽습니다. 음원에 따른 딜레이
int soil_mois() {
  //int value = analogRead(soil);
  Serial.print("read sensor value : ");
  int val = analogRead(soil);
  Serial.println(val);
  val = constrain(val, 0, 700);
  val = map(val, 0, 700, 6, 0);
  val = constrain(val, 1, 5);
  myDFPlayer.playFolder(11, val);
  return 7000;
}

int get_temp() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  if (isnan(humidity) || isnan(temperature) ) {
    Serial.println("Failed to read from DHT sensor!");
    return 0;
  }
  Serial.print(temperature); Serial.print(" *C, ");
  Serial.print(humidity); Serial.println(" %");
  int tpa = (int)constrain(temperature, 15, 35);
  myDFPlayer.playFolder(13, tpa);
  delay(4000);
  int hdi = constrain(humidity, 1, 100);
  hdi = (int)(map(hdi, 1, 100, 1, 20));
  myDFPlayer.playFolder(13, hdi + 50);
  return 4000;
}


int simple_msg() {
  Serial.println("Sample msg");
  myDFPlayer.pause();
  myDFPlayer.playFolder(10, 1);
  return 3000;
}

int music_question() {
  Serial.println("If you want to listen to music, touch again.");
  myDFPlayer.playFolder(10, 2);
  return 4000;
}

int goto_music() {
  Serial.println("goto music");
  myDFPlayer.playFolder(10, 3);
  return 4000;
}

int turn_off() {
  Serial.println("sleep");
  myDFPlayer.volume(20);
  myDFPlayer.pause();
  myDFPlayer.playFolder(10, 4);
  return 4000;
}

int music_player(int delay_input) {
  myDFPlayer.pause();
  myDFPlayer.playFolder(12, delay_input + 3);
  Serial.print(delay_input);
  Serial.print(" ");
  Serial.println(songDelays[delay_input]);
  return songDelays[delay_input];
}

int msgCount = -1;
int PendCount = 0;
int stateAlarm = 0;
int songState = 0;
int waitforNext = 0;
int isPlaying = 0;

void loop() {
  if (msgCount == -1) {
    if (get_touched()) {
      myDFPlayer.volume(20);
      delay(simple_msg());
      msgCount++;
    } else {
      delay(100);
    }
  } else if (msgCount == 0) {
    if (stateAlarm == 0) {
      delay(get_temp());
      delay(soil_mois());
      delay(music_question());
      stateAlarm = 1;
    }
    if (get_touched()) {
      delay(goto_music());
      msgCount++;
      stateAlarm = 0;
    } else {
      delay(100);
      PendCount++;
      if (PendCount > 50) {
        delay(turn_off());
        PendCount = 0;
        msgCount--;
        stateAlarm = 0;
      }
    }
  } else if (msgCount == 1) {
    myDFPlayer.volume(10);
    if (waitforNext <= 0) {
      if (isPlaying == 0) {
        waitforNext = music_player(songState % songDelaysCount);
        isPlaying = 1;
      } else {
        songState++;
        isPlaying = 0;
      }
    } else {
      waitforNext--;
    }
    if (get_touched()) {
      delay(turn_off());
      msgCount -= 2;
      stateAlarm = 0;
      waitforNext=0;
      isPlaying=0;
    }
  }
}
