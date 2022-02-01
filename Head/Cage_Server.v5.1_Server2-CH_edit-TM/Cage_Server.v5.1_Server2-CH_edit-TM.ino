/*
 * This script was created in an attempt to create an automated way to restrict food access to mice within their home cage.
 * 
 * Short story: At 7 AM a servo will move 180 degrees to close a gate, restricting food access. At 7 PM the servo will move -180 degrees to open the gate, allowing food access.
 * 
 * Long story: Each cage will be equipped with one NodeMCU and one servo. The NodeMCU is a WiFi-capable microcontroller development board that runs off the Arduino IDE (which uses C/C++). It is powered via a micro USB cable.
 * Food access needs to be restricted at nearly the same time every day to maintain experimental accuracy. The simplest solution would have been for each NodeMCU to access the time via an NTP server (online time server)
 * and keep track of that time using the NodeMCU's internal clock. However, our IT department didn't want approximately 100 devices intermittently connecting to our university's WiFi. So instead, only one NodeMCU was going
 * to connect to an NTP server. Once it had the time, it would create its own access point and server so that every other NodeMCU could connect to it, accessing the current time. However, an enterprise network (WiFi network
 * that requires both a username and password) is currently not supported in the NodeMCU's firmware. So connecting to our university's WiFi was no longer an option. Instead, time will be kept using an RTC (real-time clock).
 * 
 * 1) An RTC will be connected to a NodeMCU designated as the server.
 * 2) The server NodeMCU creates its own access point (that is not connected to the internet) and hosts a web server (that any device connected to the created access point can access).
 * 3) The web server will host the current time kept by the RTC, as well as the time each servo should open and close the gate (which will be referred to as an alarm).
 * 4) Every other NodeMCU will act as a client and connect to the server NodeMCU's access point and web server.
 * 5) Each client NodeMCU will read the current time and alarm times from the server and set its own internal clock and alarms to these values.
 * 6) If the client NodeMCU is unable to receive a valid value for any of these times, then it will attempt to reconnect to the server NodeMCU.
 * 7) Once all of the client NodeMCU's times have been set, it will wait 24 hours before attempting to reconnect to the server and update its times.
 *    The server NodeMCU's current time and alarm times can be reprogrammed by the user by logging into the server's access point and web server and submitting their own times. (This feature was implemented to avoid
 *    having to re-upload a new script to approximately 100 NodeMCUs every time the alarm time needed to be adjusted.)
 *    
 *    DS3231 Wiring:
 *    SCL/GPIO5/D1
 *    SDA/GPIO4/D2
 *    
 */
#include <Arduino.h>
//#include <Time.h> //This library doesn't appear to be necessary anymore. I think later versions of the TimeLib library made the Time library unnecessary.
#include <TimeLib.h>  //For time functions, such as setting the NodeMCU's internal clock.
//#include <Servo.h>  //Library required to control servos.
#include <TimeAlarms.h> //Library required to set alarms at specific times. This library is only intended to handle times from January 1, 1971 through January 19, 2038. If the NodeMCU's time is set to a value outside of this range, then the alarm will not set.
#include <ESP8266WiFi.h>  //Required to establish a WiFi connection.
#include <ESP8266WebServer.h> //This library may not be necessary anymore. I think ESP8266WiFi.h has its own way to set up web servers.
#include <Wire.h> //Required to start an I2C connection.
#include <RtcDS3231.h>  //Library required to use the DS3231 RTC.

const char* ssid = "Southwell_NodeMCU_Server2"; //The SSID of the access point that will be created.
const char* password = "12345678";  //The password of the access point that will be created.

RtcDS3231<TwoWire> rtcObject(Wire);
//Servo cageServo;
ESP8266WebServer server(80);  //The web server will be created on port 80 (default for HTTP).

void handleHomepage();
void handleServerTimes();
void MorningAlarm();
void EveningAlarm();

void setup() {
  Serial.begin(115200); //Using a baud rate of 115200 because it seems to be the most common?
  Serial.println();

  WiFi.softAPdisconnect(true);  //Disconnects any potential access point being produced. The NodeMCU is weird and may spontaneously produce an access point, despite re-flashing the memory.
  WiFi.disconnect();  //Disconnects any WiFi connection that may have remained from previous scripts. Sometimes a WiFi connection isn't fully dropped when resetting the NodeMCU.
  
////////////////////Setting the clocks////////////////////
  rtcObject.Begin();  //Starts I2C connection with the RTC.

  RtcDateTime compiledTime = RtcDateTime(__DATE__, __TIME__); //This is the time the script was last compiled. The script is always compiled right before upload, so the compiled time is usually within 20 seconds of the actual time.

  if (!rtcObject.IsDateTimeValid()) { //If the RTC time is not a valid time for whatever reason (RTC power is interrupted or RTC was reset), the RTC is set to the compiled time.
    rtcObject.SetDateTime(compiledTime);
    Serial.println("RTC time was not valid; set time to compiled.");
  }

  if (!rtcObject.GetIsRunning()) {  //If the RTC is not actively running, the RTC is switched on.
    rtcObject.SetIsRunning(true);
    Serial.println("RTC was not actively running; starting now.");
  }

  RtcDateTime rightNow = rtcObject.GetDateTime(); //Takes a look at the currently set RTC time.
  if (rightNow < compiledTime) {  //If the RTC time is older than the compiled time (RTC time has a lower value than compiled time), then the RTC is set to compiled time.
    rtcObject.SetDateTime(compiledTime);
    Serial.println("RTC time is older than compile time; set time to compiled.");
  }
  else if (rightNow >= compiledTime) {  //If the compiled time is older than or equal to the RTC time, then the RTC time is kept.
    Serial.println("RTC time is at/newer than compile time; will not adjust time.");
  }
  
  RtcDateTime currentEpochTime = rtcObject.GetDateTime().Epoch32Time(); //The DS3231 counts time as the number of seconds since January 1, 2000 00:00:00. The NodeMCU's internal clock begins at January 1, 1970 00:00:00 (known as Unix Epoch time). For this reason, all RTC time converted to NodeMCU time must have an addition 30 years worth of seconds added to it via Epoch32Time().
  setTime(currentEpochTime);  //Sets the NodeMCU's internal clock time. Format: (hour,minute,second,day,month,year) or (seconds since 1/1/1970 00:00:00).
////////////////////////////////////////////////////////////////////////////////
  
  //WiFi.softAP(ssid, password, 1, 0, 8);  //Creates an access point using the previously defined SSID and password.
  IPAddress ip(1,2,3,4);
  IPAddress gateway(1,2,3,1);
  IPAddress subnet(255,255,255,0);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(ssid, password, 6, 0, 8);

  Serial.print("Created access point: ");
  Serial.println(ssid);

  //Serial.print("Server IP address: ");
  //Serial.println(WiFi.softAPIP());  //Prints the IP address needed to access the created web server in the serial monitor (default is 192.168.4.1).
  delay(1000);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.on("/", handleHomepage); //Associates a handler function to the designated HTML path.
  server.on("/setServerTimes", handleServerTimes);

  server.begin(); //Creates the web server.
  Serial.println("Server ready...");

  //cageServo.attach(2);  //Assigns the servo to the D0 pin (pin #16) on the NodeMCU.
  //cageServo.write(0); //Sets the initial position of the servo to 0 degrees.

  Alarm.alarmRepeat(19,30,0,MorningAlarm); //The alarm number (discussed later) is established when the alarm is initially set, so it's set with an arbitrary negative number to avoid unnecessary triggering of the alarm. Format: (hour,minute,second,alarmName) or (timeInTotalSeconds,alarmName). (Setting the initial time of an alarm must always be after setTime() of a valid year. Otherwise the alarm will not set.)
  Alarm.alarmRepeat(7,0,0,EveningAlarm); //Same, but for evening alarm.
}
////////////////////The web server's homepage////////////////////
void handleHomepage() { //Handler function for the web server's homepage.
  int morningHours = Alarm.read(0)/3600;  //Alarm.read() gives the value of the alarm's currently set time in seconds. The following simple mathematic functions turn the total seconds into hours, minutes, and seconds.
  int morningRemainder = Alarm.read(0)%3600;
  int morningMinutes = morningRemainder/60;
  int morningSeconds = morningRemainder%60;
  
  int eveningHours = Alarm.read(1)/3600;  //Alarms are identified as numbers, beginning at 0. The order of the alarm numbers is dependent on the order in which they're set. (Example: if the evening alarm time is set before the morning alarm time, then the evening alarm becomes #0 until the next NodeMCU reset.)
  int eveningRemainder = Alarm.read(1)%3600;
  int eveningMinutes = eveningRemainder/60;
  int eveningSeconds = eveningRemainder%60;
  
  RtcDateTime currentTime = rtcObject.GetDateTime();
  char rtcBuffer[50];  //Displays the RTC's current time.
  sprintf(rtcBuffer,"Currently set to: %02d:%02d:%02d %02d/%02d/%d",
    currentTime.Hour(),
    currentTime.Minute(),
    currentTime.Second(),
    currentTime.Month(),
    currentTime.Day(),
    currentTime.Year());

  char morningBuffer[50]; //Displays the morning alarm's time.
  sprintf(morningBuffer,"Currently set to: %02d:%02d:%02d",morningHours,morningMinutes,morningSeconds);

  char eveningBuffer[50]; //Displays the evening alarm's time.
  sprintf(eveningBuffer,"Currently set to: %02d:%02d:%02d",eveningHours,eveningMinutes,eveningSeconds);

  RtcDateTime currentEpochTime = rtcObject.GetDateTime().Epoch32Time();

  String page = ""; //Simple HTML script to display the server's current RTC, morning alarm, and evening alarm times.
  page = "<meta name='viewport' content='width=device-width, initial-scale=1'><form action='setServerTimes'><h2>Time:</h2><p>";

  if (!rtcObject.IsDateTimeValid()) { //If the RTC loses power or the time resets for whatever reason...
    page += "<strong>RTC is no longer running! Check the battery or connection if resetting the server doesn't fix this issue.</strong>"; //then this message will be displayed on the homepage.
  }
  else {  //If the RTC's power issue is fixed or the RTC was always running correctly, then the currently set time will be displayed on the homepage and is accessable to the client NodeMCUs.
    page += rtcBuffer;
    page += "<span hidden name='currentEpochTime'>";  //All <span> tags are hidden from the user and are only used to convey information to the client NodeMCUs.
    page += currentEpochTime; //The RTC's set time displayed as Unix Epoch time for the client NodeMCU to set its internal clock to.
    page += "</span>";
  }
  
  page += "</p><input type='text' name ='timehh' placeholder='hh' size=1>:<input type='text' name ='timemm' placeholder='mm' size=1>:<input type='text' name ='timess' placeholder='ss' size=1> <input type='text' name ='datemo' placeholder='month' size=1>/<input type='text' name ='dateda' placeholder='day' size=1>/<input type='text' name ='dateye' placeholder='year' size=1><h2>First alarm:</h2><p>"; //This line allows the user to input their own time for the server's time.
  
  if (Alarm.read(0)<=-1) {  //If an alarm is not set, the Alarm.read() returns a value of -1.
    page += "<strong>First alarm not set!</strong>";  //This message is displayed when the morning alarm is not set.
  }
  else {
    page += morningBuffer;  //If it is set, then the morning alarm's set time is displayed.
  }
  
  page += "<span hidden name='hiddenAlarmRead0'>";
  page += Alarm.read(0);  //This is the information the client NodeMCUs use to set their own morning alarms.
  page += "</span></p><input type='text' name ='morninghh' placeholder='hh' size=1>:<input type='text' name ='morningmm' placeholder='mm' size=1>:<input type='text' name ='morningss' placeholder='ss' size=1><br><h2>Second alarm:</h2><p>"; //This line allows the user to input their own time for the evening alarm's time.

  if (Alarm.read(1)<=-1) {
    page += "<strong>Second alarm not set!</strong>";  //This message is displayed when the evening alarm is not set.
  }
  else {
    page += eveningBuffer;  //If it is set, then the evening alarm's set time is displayed.
  }
  
  page += "<span hidden name='hiddenAlarmRead1'>";
  page += Alarm.read(1);  //This is the information the client NodeMCUs use to set their own evening alarms.
  page += "</span></p><input type='text' name ='eveninghh' placeholder='hh' size=1>:<input type='text' name ='eveningmm' placeholder='mm' size=1>:<input type='text' name ='eveningss' placeholder='ss' size=1><br><br><input type='submit' value='Set times'></form>"; //This line allows the user to input their own time for the evening alarm's time.
  
  server.send(200, "text/html", page);  //The client receives the previously defined HTML page when visiting the URL defined in this page's handle.
}
////////////////////////////////////////////////////////////////////////////////

////////////////////The web server's server.arg page////////////////////
void handleServerTimes() {  //Handler function that controls the time data sent by the user.
  String message = "";  //Very simple HTML script informing the user their data has been sent. Contains a button that returns them to the homepage to verify the server is using the correct times.
  message = "<meta name='viewport' content='width=device-width, initial-scale=1'><h2>Server times set</h2><a href='/'><button>Return to homepage</button></a>";
  
  String timehh, timemm, timess, datemo, dateda, dateye, hh0, mm0, ss0, hh1, mm1, ss1;  //Declaring the various server.arg() as strings because some commands wouldn't work otherwise.
  timehh = server.arg("timehh");  //User inputted hours. Allowable inputs range from 0 - 24. If the user inputs a value outside of any of these ranges, then the numbers will rollover. (Example: inputting a value of 25 for hours will count as a 1.)
  timemm = server.arg("timemm");  //User inputted minutes. Allowable inputs range from 0 - 60. Negative numbers count as valid values for any of these inputs. (Example: inputting a value of -3 for minutes will count as the previous hour and 57 minutes.)
  timess = server.arg("timess");  //User inputted seconds. Allowable inputs range from 0 - 60.
  datemo = server.arg("datemo");  //User inputted month. Allowable inputs range from 0 - 12.
  dateda = server.arg("dateda");  //User inputted day. Allowable inputs range from 0 - 31.
  dateye = server.arg("dateye");  //User inputted year. Allowable inputs range from 2000 ~ 2106.
  hh0 = server.arg("morninghh");  //User inputted morning alarm hours.
  mm0 = server.arg("morningmm");  //User inputted morning alarm minutes.
  ss0 = server.arg("morningss");  //User inputted evening alarm seconds.
  hh1 = server.arg("eveninghh");  //User inputted evening alarm hours.
  mm1 = server.arg("eveningmm");  //User inputted evening alarm minutes.
  ss1 = server.arg("eveningss");  //User inputted evening alarm seconds.
  
  int morningTotalSeconds, eveningTotalSeconds;
  morningTotalSeconds = ((hh0.toInt() * 3600) + (mm0.toInt() * 60) + ss0.toInt());  //The user's inputted time values are converted to total seconds (needed when adjusting the alarm's time).
  eveningTotalSeconds = ((hh1.toInt() * 3600) + (mm1.toInt() * 60) + ss1.toInt());  //Each string has to be converted to an integer via toInt().

  RtcDateTime currentTime = rtcObject.GetDateTime();  //The following if statements are used to allow the user to only adjust the time or date values, while leaving the other values the same. (There is probably a more efficient way to do this. I would have liked to make it so that if ANY input is left blank, then that input will remain the same and ONLY the inputs with a value are changed, but I don't know how to do that other than having a ton of if statments.)

  if (server.arg("timehh")!= "" && server.arg("timemm")!= "" && server.arg("timess")!= "" && server.arg("dateye")!= "" && server.arg("datemo")!= "" && server.arg("dateda")!= "") { //If ALL of the time and date inputs have a user-inputted value...
    RtcDateTime newServerTime = RtcDateTime(dateye.toInt(),datemo.toInt(),dateda.toInt(),timehh.toInt(),timemm.toInt(),timess.toInt()); //then set the RTC clock to those values.
    rtcObject.SetDateTime(newServerTime);
    RtcDateTime currentEpochTime = rtcObject.GetDateTime().Epoch32Time(); //Sets the NodeMCU's internal clock to the newly updated RTC clock time.
    setTime(currentEpochTime);
  }
  if (server.arg("timehh")!= "" && server.arg("timemm")!= "" && server.arg("timess")!= "" && server.arg("dateye")== "" && server.arg("datemo")== "" && server.arg("dateda")== "") { //If ALL of the time inputs have a user-inputted value and ALL of the date inputs are left blank...
    RtcDateTime setOnlyTime = RtcDateTime(currentTime.Year(),currentTime.Month(),currentTime.Day(),timehh.toInt(),timemm.toInt(),timess.toInt()); //then set the RTC clock with the RTC clock's current date and set the RTC clock's time to the newly inputted values.
    rtcObject.SetDateTime(setOnlyTime);
    RtcDateTime currentEpochTime = rtcObject.GetDateTime().Epoch32Time();  //Sets the NodeMCU's internal clock to the newly updated RTC clock time.
    setTime(currentEpochTime);
  }
  if (server.arg("timehh")== "" && server.arg("timemm")== "" && server.arg("timess")== "" && server.arg("dateye")!= "" && server.arg("datemo")!= "" && server.arg("dateda")!= "") { //If ALL of the date inputs have a user-inputted value and ALL of the time inputs are left blank...
    RtcDateTime setOnlyDate = RtcDateTime(dateye.toInt(),datemo.toInt(),dateda.toInt(),currentTime.Hour(),currentTime.Minute(),currentTime.Second()); //then set the RTC clock with the RTC clock's current time and set the RTC clock's date to the newly inputted values.
    rtcObject.SetDateTime(setOnlyDate);
    RtcDateTime currentEpochTime = rtcObject.GetDateTime().Epoch32Time();  //Sets the NodeMCU's internal clock to the newly updated RTC clock time.
    setTime(currentEpochTime);
  }
  
  if (server.arg("morninghh")!= "" && server.arg("morningmm")!= "" && server.arg("morningss")!= "" && morningTotalSeconds >= 0) { //If ALL of the morning alarm inputs have a user-inputted value and the total of all those values are greater than or equal to 0, then set the morning alarm using those values.
    if (Alarm.read(0)<=-1) {  //The possible range these daily alarms can be set to are 0 (00:00:00) through 86399 (23:59:59). If the value returned is -1, then that means the alarm is not currently set.
      Alarm.alarmRepeat(hh0.toInt(),mm0.toInt(),ss0.toInt(),MorningAlarm);  //If the alarm is being set for the first time in the script, then Alarm.alarmRepeat() has to be used. Format: (hour,minute,second,alarmName) or (timeInTotalSeconds,alarmName).
      if (Alarm.read(0)<=-1) {  //If the morning alarm still reads as not being set, then the negative value is probably a user input, and thus, has already set the alarm, so use Alarm.write() instead. (May seem unnecessary, but I want all bases covered in case someone computer illiterate sets the alarm to a negative number.)
        Alarm.write(0,morningTotalSeconds);
      }
    }
    else {
      Alarm.write(0,morningTotalSeconds); //If the alarm has already been initially set, then Alarm.write() has to be used for subsequent time adjustments. Format: (timeInTotalSeconds,alarmIDNumber).
    }
  }
  
  if (server.arg("eveninghh")!= "" && server.arg("eveningmm")!= "" && server.arg("eveningss")!= "" && eveningTotalSeconds > -1) { //Sets the evening alarm using the same statements as the morning alarm.
    if (Alarm.read(1)<=-1) {  //If the evening alarm is not currently set...
      if (Alarm.read(0)<=-1) {  //and the morning alarm is also not currently set, then that means the evening alarm would become alarm #0...
        Alarm.alarmRepeat(0,0,-1,MorningAlarm); //so the morning alarm is initially set to a negative number to establish it as alarm #0 and the subsequently set evening alarm as alarm #1.
      }
      Alarm.alarmRepeat(hh1.toInt(),mm1.toInt(),ss1.toInt(),EveningAlarm);
      if (Alarm.read(1)<=-1) {
        Alarm.write(1,eveningTotalSeconds);
      }
    }
    else {
      Alarm.write(1,eveningTotalSeconds);
    }
  }

  server.send(200, "text/html", message); //The client receives the previously defined HTML page when visiting the URL defined in this page's handle.
}
////////////////////////////////////////////////////////////////////////////////

////////////////////Alarm functions////////////////////
void MorningAlarm() { //This alarm function will be called when the morning alarm's set time is reached.
  Serial.println("Morning alarm activated!");

  Alarm.delay(3000);
  ESP.restart();
  Serial.println("Shouldn't see this.");
  //cageServo.write(180); //Moves the servo to 180 degrees when the morning alarm is triggered.
  //Alarm.delay(500); //500ms delay to allow the servo time to move before continuting with the script. All delay() commands must be replaced with Alarm.delay() when using time alarms. 
}

void EveningAlarm() { //This alarm function will be called when the evening alarm's set time is reached.
  Serial.println("Evening alarm activated!");

  Alarm.delay(3000);
  ESP.restart();
  Serial.println("Shouldn't see this.");
  //cageServo.write(0); //Moves the servo to 0 degrees when the evening alarm is triggered.
  //Alarm.delay(500); //(These delays are probably unnecessary in this final iteration. Was needed during prototyping and stress testing.)
}
////////////////////////////////////////////////////////////////////////////////

void loop() { //The NodeMCU will perpetually run this loop when waiting for an alarm to reach its set time.
  unsigned long restartTimeout = millis();
  while (millis() - restartTimeout < 1800000) {   //1800000 = 30 minutes
    server.handleClient();  //Handler function to control how to handle each client that connects to the web server.
    Serial.print(WiFi.softAPgetStationNum()); //Returns the number of clients connected to the server's access point.
    Serial.print(" - Restarting ESP8266 when ");
    Serial.print((millis() - restartTimeout)/1000);
    Serial.println(" = 1800.");
    Alarm.delay(50); //Small delay for the sake of handleClient().
  }
  ESP.restart();
}
