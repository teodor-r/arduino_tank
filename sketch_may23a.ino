
#include <AFMotor.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include "GyverTimer.h"
#include <string.h>    // подключаем библиотеку
#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>

//#include <AsyncStream.h>
 
  
GTimer bodyTimer(MS);
GTimer towerTimer(MS);
GTimer gunTimer(MS);

int stepG = 1;
int stepT = 1;
const char *row = "FBLRS";
 
Servo tower;
int minT = 0, maxT = 180;
Servo gun;
int minG = 0, maxG = 90;

AF_DCMotor motor1(3); // Левый мотор
AF_DCMotor motor2(4); // Правый мотор

int spFB=0, spLR=0;
int tow_dir = 0, gun_dir = 0;
boolean tow_flag =1 , gun_flag = 1;
boolean flag = false;
 
SoftwareSerial BTserial (0, 1);
 
int TEST_SPEED = 240;
int servoPinT = 9;
int servoPinG = 10;

//Для диодов и лазера
int GREEN =3 , YELLOW = 8, RED = 5, LASER =7;

int BEEP;

typedef enum Speed {
    low = 60,
    mid = 120,
    high = 250
};
Speed speed = high;

//MOVE, PREPARE, FIRE
String status = "";

void setup() {
  bodyTimer.setInterval(150);
  towerTimer.setInterval(200);
 
  pinMode (GREEN, OUTPUT);
  pinMode (YELLOW, OUTPUT);
  pinMode (RED, OUTPUT);
  pinMode (LASER, OUTPUT);
  tower.attach (9);
  gun.attach (10);

  // tower.write(90);
  // gun.write(90);

  //tower.writeMicroseconds (1500);
  //gun.writeMicroseconds (1500);
  
  Serial.begin(9600);
  pinMode (BEEP, OUTPUT);
  BTserial.begin (9600);
  motor1.setSpeed (speed);
  motor2.setSpeed (speed);
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  status = "MOVE";
}

void body_stop () {
  spFB = 0;
  spLR = 0;
  motor1.setSpeed (0);
  motor1.run (RELEASE);
  motor2.setSpeed (0);
  motor2.run (RELEASE);
}
void gun_stop () {
  gun_dir = 0;
}
void tower_stop () {
  tow_dir = 0;
}

void mv(int spFB, int spLR)
{
  if(spFB < 0 && spLR < 0)
  {
     motor1.setSpeed (abs(spFB));
     motor2.setSpeed (abs(spLR));
     motor1.run (BACKWARD);
     motor2.run (FORWARD);
  }
    if(spFB < 0 && spLR > 0)
  {
     motor1.setSpeed (abs(spFB));
     motor2.setSpeed (abs(spLR));
     motor1.run (BACKWARD);
     motor2.run (BACKWARD);
  }
    if(spFB >0  && spLR < 0)
  {
     motor1.setSpeed (abs(spFB));
     motor2.setSpeed (abs(spLR));
     motor1.run (FORWARD);
     motor2.run (FORWARD);
  }
    if(spFB > 0 && spLR > 0)
  {
     motor1.setSpeed (abs(spFB));
     motor2.setSpeed (abs(spLR));
     motor1.run (FORWARD);
     motor2.run (BACKWARD);
  }
  if(spFB == 0 && spLR > 0)
  {
     motor2.setSpeed (spLR);
     motor1.setSpeed (spLR);
     motor2.run (BACKWARD);
     motor1.run (BACKWARD);
  }
    if(spFB == 0 && spLR == 0)
  {
     motor2.setSpeed (spLR);
     motor1.setSpeed (spLR);
     motor2.run (RELEASE);
     motor1.run (RELEASE);
  }
      if(spFB == 0 && spLR < 0)
  {
     motor2.setSpeed (abs(spLR));
     motor1.setSpeed (abs(spLR));
     motor2.run (FORWARD);
     motor1.run (FORWARD);
  }
    if(spFB >0 && spLR == 0)
  {
     motor2.setSpeed (spFB);
     motor1.setSpeed (spFB);
     motor2.run (FORWARD);
     motor1.run (BACKWARD);
  }
      if(spFB < 0 && spLR == 0)
  {
     motor2.setSpeed (abs(spFB));
     motor1.setSpeed (abs(spFB));
     motor2.run (BACKWARD);
     motor1.run (FORWARD);
  }
}
 
void mv_gun (int tow_dir, int gun_dir) {
  if ((tow_dir == 0) && (gun_dir == 0))
    return;
  if (tow_dir == -1)
    tn_left();
  else if (tow_dir == 1)
    tn_right();
  if (gun_dir == -1)
    gun_down();
  else if (gun_dir == 1)
    gun_up();
}

int current_angleT = tower.read();
int current_angleG = gun.read();

// rotate tower

void tn_right () {
  towerTimer.start();
  if(minT <= current_angleT + stepT <= maxT) {
    current_angleT += stepT;
    tower.write(current_angleT);
  }
  tow_dir = 0;
}

void tn_left () {
  towerTimer.start();
  if(minT <= current_angleT - stepT <= maxT) {
    current_angleT -= stepT;
    tower.write(current_angleT);
  }
  tow_dir = 0;
}

void gun_up () {
  gunTimer.start();
  if(minG <= current_angleG + stepG <= maxG) {
    current_angleG += stepG;
    gun.write(current_angleG);
    
  }
  gun_dir = 0;
}

void gun_down () {
  gunTimer.start();
  if(minG <= current_angleG - stepG <= maxG) {
    current_angleG -= stepG;
    gun.write(current_angleG);
  }
  gun_dir = 0;
}

char lst_butt="";
char crnt_butt = "";

char sep [2]=":"; 
const byte numChars = 40;
String receivedChars= "";   // an array to store the received data
boolean newData = false;

void recvWithEndMarker() {
    char rc;
    while (BTserial.available() > 0 && newData == false) {
        rc = BTserial.read();
        if (rc != 'F')
          receivedChars += rc;
        else {
          Serial.println(receivedChars);
          char buf[40];
          strcpy(buf,receivedChars.c_str());
          delimateString(spLR, spFB, tow_dir, gun_dir, buf, flag);
          receivedChars = "";
        }
    }
}

void delimateString(int &spLR, int &spFB, int &tow_dir, int &gun_dir, char* cm, boolean &fire_flag)
{
    char *istr; 
    istr = strtok (cm,sep); 
    spLR = atoi(istr);
    Serial.println(spLR,DEC);

    istr = strtok (NULL,sep); 
    spFB = atoi(istr);
    Serial.println(spFB,DEC);
    if(spLR!=0 || spFB !=0 )
      bodyTimer.reset();
    
    istr = strtok (NULL, sep);
    tow_dir = atoi (istr);

    istr = strtok (NULL, sep);
    gun_dir = atoi (istr);

    if(tow_dir!=0 && tow_flag!=1)
      tow_dir =0;
    if(gun_dir!=0  && gun_flag!=1)
      gun_dir = 0;
 

    istr = strtok (NULL,sep);
    fire_flag = (strcmp (istr, "f") == 0);

    newData = false;
}

void change_status () {
  if ((status == "MOVE") && (flag))
    status = "PREPARE";
  else if ((status == "PREPARE") && (flag))
    status = "FIRE";
}

void diod_light () {
  if (status == "MOVE") {
    analogWrite (YELLOW, HIGH);
    analogWrite (RED, HIGH);
    analogWrite (GREEN, HIGH);
    analogWrite (LASER, HIGH);
  }
  else if (status == "PREPARE") {
    //analogWrite (GREEN, LOW);
    //analogWrite (RED, LOW);
    analogWrite (YELLOW, HIGH);
  }
  else if (status == "FIRE") {
    //analogWrite (GREEN, LOW);
    //analogWrite (YELLOW, LOW);
    analogWrite (RED, HIGH);
  }
}

void fire () {
  int reload_time = 10000, fire_time = 1000;
  analogWrite (BEEP, 50);
  analogWrite (LASER, HIGH);
  delay (fire_time);
  analogWrite (BEEP, 0);
  analogWrite (LASER, LOW);
  //Like a reload
  delay (reload_time);
}

void loop () {
  if (bodyTimer.isReady()){
    body_stop();
    bodyTimer.reset();
  } 
  if (towerTimer.isReady()){
    tow_flag= 1;
    towerTimer.reset();
    towerTimer.stop();
  } 
   if (gunTimer.isReady()){
    gun_flag= 1;
    gunTimer.reset();
    gunTimer.stop();
  } 

  recvWithEndMarker();
  change_status();
  diod_light();
  mv(spFB, spLR);
  mv_gun (tow_dir, gun_dir);
  // if (status == "MOVE")
  //   mv(spFB, spLR);
  // if (status == "PREPARE") {
  //   full_stop();
  //   mv_gun (tow_dir, gun_dir);
  // }
  // if (status == "FIRE") {
  //   fire();
  //   flag = false;
  //   status = "MOVE";
  // }
}
