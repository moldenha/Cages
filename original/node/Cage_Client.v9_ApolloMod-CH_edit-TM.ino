/*
   Anything redundant from Cage_server_final.ino was not included.
*/
//#include <Time.h>
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

const char* ssid     = "Southwell_NodeMCU";
const char* password = "12345678";
const char* host     = "192.168.4.1"; //The server NodeMCU's IP address (192.168.4.1 is default).
const int hostPort   = 80;  //The server NodeMCU's host port (80 is default).
char timeBuffer[50];

AccelStepper stepper1(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);

const int buttonPin = 0;
const int ledPin = 2;

const int alarmMotorDistance = 2048;  //4096 steps is one full stepper motor rotation in HALFSTEP two-phase mode

//Servo cageServo;
//int servoPin = 2;

void setup() {
  Serial.begin(115200);
  //Alarm.delay(500);

  WiFi.softAPdisconnect(true);
  WiFi.disconnect();

  stepper1.setMaxSpeed(1000);
  stepper1.setAcceleration(1000);
  //stepper1.setSpeed(1000);
  stepper1.moveTo(-4096 * 21); //4096 steps is a full revolution of the stepper motor.

  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  int var = 0;
  int motorInterval = 3000;  //The interval of time the stepper motor can be adjusted in milliseconds.
  while (var < motorInterval) {
    var++;
    delay(1);
    stepper1.disableOutputs();

    while (digitalRead(buttonPin) == LOW) {
      stepper1.run();
      yield();
    }
  }

  digitalWrite(ledPin, HIGH);

  //cageServo.detach();   //To try and stop the servo twitching that occurs when the board resets.

  //cageServo.attach(servoPin);
  //cageServo.write(0);

  /*Uncomment this section if setting an initial time for the alarms is desired. The alarms only work between January 1, 1971 through January 19, 2038, otherwise they will not set...
    setTime(1,2,3,4,5,6); //However, if the the NodeMCU's internal clock is not already set, then it defaults to 00:00:00 January 1, 1970. So an arbitrary, but valid, initial time must be set. Format: (hour,minute,second,day,month,year). (The correct time can be used to initially set it, but it will just reset each time the NodeMCU is reset or loses power.)
    Alarm.alarmRepeat(7,0,0,MorningAlarm);
    Alarm.alarmRepeat(19,0,0,EveningAlarm);
  */

  //pinMode(2,OUTPUT);  //Designates D4 (pin #2) as an output, which is connected to the onboard LED.

  sprintf(timeBuffer, "%02d:%02d:%02d %02d/%02d/%d", hour(), minute(), second(), month(), day(), year());
  Serial.println("");
  Serial.print("Time set to: ");
  Serial.println(timeBuffer); //These serial prints are here just to show the initial time and alarm values. That way it can be verified that the time and alarms did infact change after connecting to the server NodeMCU.

  Serial.print("Morning alarm set to: ");
  Serial.println(Alarm.read(0));

  Serial.print("Evening alarm set to: ");
  Serial.println(Alarm.read(1));
}

////////////////////Beginning of void loop////////////////////
void loop() {
  //long randomNumber;
  //randomNumber = random(1,60000); //Random number between 1 and 59,999.
  //delay(randomNumber);  //Delays the rest of void loop by the random number that was generated. This is done to avoid a WiFi traffic jam in the event all the client NodeMCU's are turned on at the same time (ex: power surge).

  WiFi.disconnect();  //This WiFi disconnect is here to ensure no WiFi connection is still present before attempting a new WiFi connection. Like in the instance the client isn't able to connect to the web server, so the void loop is restarted.
  //digitalWrite(2,LOW);  //Turns the onboard LED on. (The LED has inverted logic for some reason.)

  Serial.print("Connecting to ");
  Serial.println(ssid);

  ////////////////////Connecting to the server NodeMCU's access point////////////////////
  WiFi.begin(ssid, password); //Connects to the server NodeMCU's created access point.

  unsigned long wifiTimeout = millis(); //millis() returns the number of seconds the NodeMCU has been powered on.
  while (WiFi.status() != WL_CONNECTED) { //The NodeMCU will be stuck in this loop until a WiFi connection is established.
    if (Alarm.read(0) != -1 && Alarm.read(1) != -1) { //Unless both the morning and evening alarms are already set...
      if (millis() - wifiTimeout > 30000) { //then the NodeMCU will only attempt to connect to WiFi for 30 seconds before giving up.
        Serial.println();
        Serial.println("WiFi not responding. Since both morning and evening alarms are already set up, won't recheck server alarm times.");
        break;  //The NodeMCU will break out of the while loop and proceed to the end of the void loop, causing it to wait 24 hours before attempting to reconnect to the WiFi.
      }
    }
    Alarm.delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {  //Once the WiFi connect is finally established, the NodeMCU can continue.
    Serial.println();
    Serial.println("WiFi connected.");
    Serial.print("Client IP address: ");
    Serial.println(WiFi.localIP()); //The client NodeMCU's IP address.

    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////Connecting to the server NodeMCU's web server////////////////////
    Serial.print("Connecting to server: ");
    Serial.println(host);

    WiFiClient client;  //Establishes the client NodeMCU as a client.
    client.setTimeout(10);  //Due to the nature of the readString() command that is used later, a timeout should be set to avoid long connection times. (I don't fully understand it.) Reading the data from the server takes less than 1 second with this timeout, but if none is set, then the connection lasts approximately 5 seconds. I want the connection to be as quick as possible to avoid WiFi traffic jams with all the different clients.

    if (!client.connect(host, hostPort)) { //If the client NodeMCU is unable to connect to the web server, then the client attempts to reconnect by restarting at the void loop.
      Serial.print("Connection to ");
      Serial.print(host);
      Serial.println(" failed.");
      return;
    }

    String url = "/"; //This is the URL for the server's homepage the client will connect to.
    String getRequest;  //Simple GET request to ask the server for its data.
    getRequest = "GET " + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n";
    client.print(getRequest);

    unsigned long serverTimeout = millis(); //Similar to the WiFi timeout earlier, the client has 10 seconds to establish a connection with the server before returning to the beginning of the void loop and attempting to reconnect.
    while (client.available() == 0) {
      if (millis() - serverTimeout > 10000) {
        Serial.println("Connection timed out!");
        client.stop();
        return;
      }
    }
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////Established web server connection////////////////////
    while (client.available()) { //Once the client establishes a connecton to the web server...
      String line = client.readString();  //readString() works but readStringUntil('\r') doesn't, for some reason, despite readStringUntil('\r') being the way more popular option. Although readString() is currently working perfectly, my fear is that it may not continue to work once an unforseen situation occurs.

      //Each time value that the client NodeMCU needs to set its respective time is hidden in an HTML <span> tag within the server's HTML homepage.
      //Each <span> tag looks like this: <span hidden name='currentEpochTime'>123456789</span>
      //indexOf() returns the character number value of a specific character or sequence of characters. Format: ("charactersYouWantToFind",indexValueToStartAt). (Example: calling indexOf('l',5) on the phrase "Hello world" would return a value of 10, because that is how many characters deep the first "l" since character 5 is found. If the 5 parameter was omitted, the value returned would have been 3.)
      int currentEpochTimeIndex = line.indexOf("currentEpochTime"); //The first (and only) instance of "currentEpochTime" in the data sent back from the server is in the <span> tag right before the value we need.
      int currentEpochTimeClosingBracket = line.indexOf('>', currentEpochTimeIndex); //Finds the first instance of '>' starting at "currentEpochTime", which is one character directly before the target value.
      int currentEpochTimeOpeningBracket = line.indexOf('<', currentEpochTimeIndex); //Finds the first instance of '<' starting at "currentEpochTime", which is one character directly after the target value.
      String currentEpochTime = line.substring(currentEpochTimeClosingBracket + 1, currentEpochTimeOpeningBracket); //substring() returns what is written between the two character counts you input. Format: {startReadingAtThisCharacter,stopReadingAtThisCharacter). (If the +1 wasn't added to currentEpochTimeClosingBracket, then the actual > before the time value would be included.)

      int hiddenAlarm0Index = line.indexOf("hiddenAlarmRead0");
      int hiddenAlarm0ClosingBracket = line.indexOf('>', hiddenAlarm0Index);
      int hiddenAlarm0OpeningBracket = line.indexOf('<', hiddenAlarm0Index);
      String hiddenAlarmRead0 = line.substring(hiddenAlarm0ClosingBracket + 1, hiddenAlarm0OpeningBracket);

      int hiddenAlarm1Index = line.indexOf("hiddenAlarmRead1");
      int hiddenAlarm1ClosingBracket = line.indexOf('>', hiddenAlarm1Index);
      int hiddenAlarm1OpeningBracket = line.indexOf('<', hiddenAlarm1Index);
      String hiddenAlarmRead1 = line.substring(hiddenAlarm1ClosingBracket + 1, hiddenAlarm1OpeningBracket);

      Serial.println(line); //Prints the entire client.readString() line.

      Serial.print("Server time: ");
      Serial.println(currentEpochTime); //The following serial prints are just to verify that the time data collected from the server is valid and accurate.

      Serial.print("hiddenAlarmRead0: ");
      Serial.println(hiddenAlarmRead0);

      Serial.print("hiddenAlarmRead1: ");
      Serial.println(hiddenAlarmRead1);

      ////////////////////Setting the client NodeMCU current time and alarm times////////////////////
      setTime(currentEpochTime.toInt());  //Sets the NodeMCU's internal clock to the current time obtained from the server NodeMCU.

      if (hiddenAlarmRead0.toInt() != -1) { //If the morning alarm value from the server NodeMCU is NOT -1 (not set), then continue...
        if (Alarm.read(0) == -1) {
          Alarm.alarmRepeat(hiddenAlarmRead0.toInt(), MorningAlarm);
          Serial.println("Initial morning alarm set!");
        }
        else {
          Alarm.write(0, hiddenAlarmRead0.toInt());
          Serial.println("Subsequent morning alarm set!");
        }
      }

      if (hiddenAlarmRead1.toInt() != -1) { //If the evening alarm value from the server NodeMCU is NOT -1 (not set), then continue...
        if (Alarm.read(1) == -1) {
          Alarm.alarmRepeat(hiddenAlarmRead1.toInt(), EveningAlarm);
          Serial.println("Initial evening alarm set!");
        }
        else {
          Alarm.write(1, hiddenAlarmRead1.toInt());
          Serial.println("Subsequent evening alarm set!");
        }
      }
    }
    ////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////
    client.stop();  //Disconnects the client-web server connection.
  }

  WiFi.disconnect();  //Disconnects the client from the server NodeMCU's access point, regardless of whether the client NodeMCU has set any of its times. This is to allow the other clients an opportunity to connect and receive their times.

  Serial.println("Server and WiFi connections closed.");

  sprintf(timeBuffer, "%02d:%02d:%02d %02d/%02d/%d", hour(), minute(), second(), month(), day(), year());
  Serial.print("Time set to: ");
  Serial.println(timeBuffer); //The following serial prints are just to verify that the times have/have not been changed from the serial prints at the beginning of the script.

  Serial.print("Morning alarm set to: ");
  Serial.println(Alarm.read(0));

  Serial.print("Evening alarm set to: ");
  Serial.println(Alarm.read(1));

  if (Alarm.read(0) <= - 1 || Alarm.read(1) <= -1) { //If EITHER of the alarms are still not set, then the client NodeMCU waits 60 seconds before restarting to the beginning of the void loop to reconnect to the WiFi and obtain correct times.
    if (Alarm.read(0) <= -1 && Alarm.read(1) > -1) Serial.println("Morning alarm not set!");
    if (Alarm.read(0) > -1 && Alarm.read(1) <= -1) Serial.println("Evening alarm not set!");
    if (Alarm.read(0) <= -1 && Alarm.read(1) <= -1) Serial.println("Both morning and evening alarms not set!");
    Serial.println("Will retry server connection in 60 seconds...");
    ESP.deepSleep(60e6);
    //Alarm.delay(60000);
    //return;
  }

  if ((((hour() * 3600) + (minute() * 60) + second() - Alarm.read(0)) >= 0) && ((hour() * 3600) + (minute() * 60) + second() - Alarm.read(0)) <= 900) {
    Serial.println("Morning alarm activated!");

    Serial.println(stepper1.currentPosition());
    Serial.println(stepper1.currentPosition() + alarmMotorDistance);

    stepper1.setMaxSpeed(400);
    stepper1.moveTo(stepper1.currentPosition() + alarmMotorDistance);

    while (stepper1.distanceToGo() != 0) {
      //Serial.println("M1");
      stepper1.run();
      yield();
    }
    /*if (stepper1.distanceToGo() == 0) {
      //Serial.println("M2");
      //stepper1.moveTo((-stepper1.currentPosition())/2);
      }*/
    Serial.println("Morning alarm completed.");
    stepper1.disableOutputs();
  }

  if ((((hour() * 3600) + (minute() * 60) + second() - Alarm.read(1)) >= 0) && ((hour() * 3600) + (minute() * 60) + second() - Alarm.read(1)) <= 900) {
    Serial.println(stepper1.currentPosition());
    Serial.println(stepper1.currentPosition() - alarmMotorDistance);

    stepper1.setMaxSpeed(400);
    stepper1.moveTo(stepper1.currentPosition() - alarmMotorDistance);

    while (stepper1.distanceToGo() != 0) {
      //Serial.println("E1");
      stepper1.run();
      yield();
    }
    /*if (stepper1.distanceToGo() == 0) {
      Serial.println("E2");
      stepper1.moveTo((-stepper1.currentPosition())/2);
      }*/
    Serial.println("Evening alarm completed.");
    stepper1.disableOutputs();
  }

  while (((hour() * 3600) + (minute() * 60) + second() - Alarm.read(0)) >= -600 && ((hour() * 3600) + (minute() * 60) + second() - Alarm.read(0)) <= 0) {
    Serial.println(((hour() * 3600) + (minute() * 60) + second()) - Alarm.read(0));
    Serial.println("Morning alarm will activate in less than 10 minutes. Staying awake until activation.");
    Alarm.delay(1000);
  }

  if (((hour() * 3600) + (minute() * 60) + second() - Alarm.read(0)) >= -840 && ((hour() * 3600) + (minute() * 60) + second() - Alarm.read(0)) <= 0) {
    Serial.println(((hour() * 3600) + (minute() * 60) + second()) - Alarm.read(0));
    Serial.println("Morning alarm will activate in less than 14 minutes. Going to sleep for 5 minutes.");
    ESP.deepSleep(300e6);
  }

  if (((hour() * 3600) + (minute() * 60) + second() - Alarm.read(0)) >= -2040 && ((hour() * 3600) + (minute() * 60) + second() - Alarm.read(0)) <= 0) {
    Serial.println(((hour() * 3600) + (minute() * 60) + second()) - Alarm.read(0));
    Serial.println("Morning alarm will activate in less than 34 minutes. Going to sleep for 10 minutes.");
    ESP.deepSleep(600e6);
  }

  if (((hour() * 3600) + (minute() * 60) + second() - Alarm.read(0)) >= -3840 && ((hour() * 3600) + (minute() * 60) + second() - Alarm.read(0)) <= 0) {
    Serial.println(((hour() * 3600) + (minute() * 60) + second()) - Alarm.read(0));
    Serial.println("Morning alarm will activate in less than 64 minutes. Going to sleep for 30 minutes.");
    ESP.deepSleep(1800e6);
  }

  while (((hour() * 3600) + (minute() * 60) + second() - Alarm.read(1)) >= -600 && ((hour() * 3600) + (minute() * 60) + second() - Alarm.read(1)) <= 0) {
    Serial.println(((hour() * 3600) + (minute() * 60) + second()) - Alarm.read(1));
    Serial.println("Evening alarm will activate in less than 10 minutes. Staying awake until activation.");
    Alarm.delay(1000);
  }

  if (((hour() * 3600) + (minute() * 60) + second() - Alarm.read(1)) >= -840 && ((hour() * 3600) + (minute() * 60) + second() - Alarm.read(1)) <= 0) {
    Serial.println(((hour() * 3600) + (minute() * 60) + second()) - Alarm.read(1));
    Serial.println("Evening alarm will activate in less than 14 minutes. Going to sleep for 5 minutes.");
    ESP.deepSleep(300e6);
  }

  if (((hour() * 3600) + (minute() * 60) + second() - Alarm.read(1)) >= -2040 && ((hour() * 3600) + (minute() * 60) + second() - Alarm.read(1)) <= 0) {
    Serial.println(((hour() * 3600) + (minute() * 60) + second()) - Alarm.read(1));
    Serial.println("Evening alarm will activate in less than 34 minutes. Going to sleep for 10 minutes.");
    ESP.deepSleep(600e6);
  }

  if (((hour() * 3600) + (minute() * 60) + second() - Alarm.read(1)) >= -3840 && ((hour() * 3600) + (minute() * 60) + second() - Alarm.read(1)) <= 0) {
    Serial.println(((hour() * 3600) + (minute() * 60) + second()) - Alarm.read(1));
    Serial.println("Evening alarm will activate in less than 64 minutes. Going to sleep for 30 minutes.");
    ESP.deepSleep(1800e6);
  }

  /*while (((hour()*3600)+(minute()*60)+second() - Alarm.read(1)) >= -330 && ((hour()*3600)+(minute()*60)+second() - Alarm.read(1)) <= 0) {
      Serial.println(((hour()*3600)+(minute()*60)+second()));
      Serial.println(Alarm.read(1));
      Serial.println(((hour()*3600)+(minute()*60)+second()) - Alarm.read(1));
      Alarm.delay(1000);
    }*/

  //Serial.println("Waiting 15 seconds for motor.");
  //Alarm.delay(15000);

  //digitalWrite(2,HIGH); //Turns the onboard LED off once the client NodeMCU has set both of its alarm times.
  Serial.println("Morning and evening alarm setup complete!");

  //Serial.println("Will recheck alarm times in 10 minutes...");
  //Alarm.delay(60000);  //The NodeMCU will wait 1 minute (value in milliseconds) before restarting the void loop and attempting to reconnect to the WiFi. This is done even if the alarm times are already set. That way the NodeMCU checks for the most up-to-date time every day.

  long randomNumber;
  randomNumber = random(1, 60e6);     //60 seconds = 1 minute
  unsigned long deepSleepDuration;
  deepSleepDuration = 3600e6;          //3600 seconds = 60 minutes

  Serial.println("Will recheck alarm times in ");
  Serial.print(randomNumber + deepSleepDuration);
  Serial.print(" microseconds.");
  ESP.deepSleep(randomNumber + deepSleepDuration);  //NodeMCU goes into deepSleep for 600 seconds (10 minutes).
}

////////////////////Alarm functions////////////////////
void MorningAlarm() {
  Serial.println("Morning alarm activated!");

  Serial.println(stepper1.currentPosition());
  Serial.println(stepper1.currentPosition() + alarmMotorDistance);

  stepper1.setMaxSpeed(400);
  stepper1.moveTo(stepper1.currentPosition() + alarmMotorDistance);

  while (stepper1.distanceToGo() != 0) {
    //Serial.println("M1");
    stepper1.run();
    yield();
  }
  /*if (stepper1.distanceToGo() == 0) {
    //Serial.println("M2");
    //stepper1.moveTo((-stepper1.currentPosition())/2);
    }*/
  Serial.println("Morning alarm completed.");
  stepper1.disableOutputs();
}

void EveningAlarm() {
  Serial.println("Evening alarm activated!");

  Serial.println(stepper1.currentPosition());
  Serial.println(stepper1.currentPosition() - alarmMotorDistance);

  stepper1.setMaxSpeed(400);
  stepper1.moveTo(stepper1.currentPosition() - alarmMotorDistance);

  while (stepper1.distanceToGo() != 0) {
    //Serial.println("E1");
    stepper1.run();
    yield();
  }
  /*if (stepper1.distanceToGo() == 0) {
    Serial.println("E2");
    stepper1.moveTo((-stepper1.currentPosition())/2);
    }*/
  Serial.println("Evening alarm completed.");
  stepper1.disableOutputs();
}
////////////////////////////////////////////////////////////////////////////////
