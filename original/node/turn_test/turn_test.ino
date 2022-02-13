#include <TimeLib.h>
#include <TimeAlarms.h>
#include <ESP8266WiFi.h>
//#include <Servo.h>
#include <AccelStepper.h>


#define HALFSTEP 8
#define motorPin1  14     // IN1 on the ULN2003 driver 1 (D5)
#define motorPin2  12     // IN2 on the ULN2003 driver 1 (D6)
#define motorPin3  13     // IN3 on the ULN2003 driver 1 (D7)
#define motorPin4  15     // IN4 on the ULN2003 driver 1 (D8)

const int buttonPin = 0;
const int ledPin = 2;

AccelStepper stepper1(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);

const int alarmMotorDistance = 2048;

void setup(){
  Serial.begin(115200);
  Serial.println("moving");
  stepper1.setMaxSpeed(1000);
  stepper1.setAcceleration(1000);
  //stepper1.setSpeed(1000);
  stepper1.moveTo(-4096 * 21);
}

void loop(){
  //keep it at -2048 and -4096
  if(stepper1.currentPosition() == -2048){
    Serial.print("closing ");
    close();
  }
  else if(stepper1.currentPosition() == -4096){
    Serial.print("opening ");
    open();
  }
  else{
    Serial.print("opposite ");
    open();
  }
  delay(5000);
 //
}

void open(){
  stepper1.setMaxSpeed(400);
  stepper1.moveTo(-2048);
  while (stepper1.distanceToGo() != 0) {
    //Serial.println("E1");
    stepper1.run();
    yield();
  }
  Serial.println("opened");
}

void close(){
  stepper1.setMaxSpeed(400);
  stepper1.moveTo(-4096);
  while (stepper1.distanceToGo() != 0) {
    //Serial.println("E1");
    stepper1.run();
    yield();
  }
  Serial.println("closed");
}
