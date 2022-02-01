#include "clienthandler.h"
#include <ESP8266WiFi.h>
//#include <Servo.h>
#include <AccelStepper.h>

const char* ssid = "Linksys05013"; //The SSID of the access point that will be created.
const char* password = "dny74smdxp";  //The password of the access point that will be created.
 

WiFiServer server(90);
#define HALFSTEP 8
#define motorPin1  14     // IN1 on the ULN2003 driver 1 (D5)
#define motorPin2  12     // IN2 on the ULN2003 driver 1 (D6)
#define motorPin3  13     // IN3 on the ULN2003 driver 1 (D7)
#define motorPin4  15     // IN4 on the ULN2003 driver 1 (D8)

const int buttonPin = 0;
const int ledPin = 2;

AccelStepper stepper1(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);

const int alarmMotorDistance = 2048;

clientHandler handler(&stepper1, &server);

void setup(){
  Serial.begin(115200);
  Serial.println("moving");
  stepper1.setMaxSpeed(1000);
  stepper1.setAcceleration(1000);
  //stepper1.setSpeed(1000);
  stepper1.moveTo(-4096 * 21);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {  //Wait for connection
      delay(500);
      Serial.println("Waiting to connect...");
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("server started on local IP");
}

void loop(){
  handler.handleClients();
}
