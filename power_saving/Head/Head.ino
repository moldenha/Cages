#include "server.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);
serverHandle handler;

const char* ssid = "Linksys05013"; //The SSID of the access point that will be created.
const char* password = "dny74smdxp";  //The password of the access point that will be created.
 
void setup() {
  Serial.begin(115200); //Begin Serial at 115200 Baud
  WiFi.begin(ssid, password);  //Connect to the WiFi network
  handler.addTime(7, 0, 0, 0);
  handler.addTime(19,30,0,1);
  while (WiFi.status() != WL_CONNECTED) {  //Wait for connection
      delay(500);
      Serial.println("Waiting to connect...");
  }
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //Print the local IP
  
  server.on("/", handle_index); //Handle Index page
  server.on("/setServerTimes", handle_setServerTimes);
  server.on("/correctTime", handle_timeCorrection);
  server.on("/addAlarm", handle_addAlarm);
  server.on("/removeAlarm", handle_removeAlarm);
  server.on("/findServos", handle_findServos);
  server.on("/openAll", handle_openAll);
  server.on("/closeAll", handle_closeAll);
  server.on("/oppositeAll", handle_oppositeAll);
  server.on("/addServo", handle_addServo);
  server.on("/sendAlarms", handle_sendAlarms);
  
  server.begin(); //Start the server
  Serial.println("Server listening");
}

void loop() {
  server.handleClient(); //Handling of incoming client requests
  handler.check();
}

void handle_index() {
  //Print Hello at opening homepage
  String page = handler.makePage();
  server.send(200, "text/html", page);
}

void handle_setServerTimes(){
  String message = " <meta http-equiv='refresh' content='1; URL=/' />";
  for(int i = 0; i < handler.get_changeamt(); i++){
    String hour, minute, second;
    String time_h = "timeh"+String(i);
    String time_m = "timem"+String(i);
    String time_s = "times"+String(i);
    hour = server.arg(time_h);
    minute = server.arg(time_m);
    second = server.arg(time_s);
    if(hour != "" && minute != "" && second != "")
      handler.editTime(i, hour.toInt(), minute.toInt(), second.toInt());
  }
  server.send(200, "text/html", message);
}

void handle_addServo(){
  String message = " <meta http-equiv='refresh' content='1; URL=/' />";
  String newIP = server.arg("servAdd");
  if(newIP != ""){
    handler.addServo(newIP.c_str());
  }
  server.send(200, "text/html", message);
}
void handle_timeCorrection(){
  String message = " <meta http-equiv='refresh' content='1; URL=/' />";
  String newCorrectionH = server.arg("correctionh");
  String newCorrectionM = server.arg("correctionm");
  String newCorrectionS = server.arg("corrections");
  if(newCorrectionH != "" && newCorrectionM != "" && newCorrectionS != ""){
    Serial.println("setting new start time");
    handler.correctTime(newCorrectionH.toInt(), newCorrectionM.toInt(), newCorrectionM.toInt());
  }
  server.send(200, "text/html", message);
}

void handle_addAlarm(){
  String message = " <meta http-equiv='refresh' content='1; URL=/' />";
  handler.addTime(server.arg("addh").toInt(), server.arg("addm").toInt(), server.arg("adds").toInt(), server.arg("subject").toInt());
  server.send(200, "text/html", message);
}

void handle_removeAlarm(){
  String message = " <meta http-equiv='refresh' content='1; URL=/' />";
  handler.removeTime(server.arg("removeSubject").toInt());
  server.send(200, "text/html", message);
}

void handle_findServos(){
  String message = " <meta http-equiv='refresh' content='1; URL=/' />";
  Serial.println("finding servos");
  handler.findServos(100, 170);
  server.send(200, "text/html", message);
}
void handle_openAll(){
  String message = " <meta http-equiv='refresh' content='1; URL=/' />";
  handler.openAll();
  server.send(200, "text/html", message);
}

void handle_sendAlarms(){
  String message = " <meta http-equiv='refresh' content='1; URL=/' />";
  handler.sendAlarmsAll();
  server.send(200, "text/html", message);
}
void handle_closeAll(){
  String message = " <meta http-equiv='refresh' content='1; URL=/' />";
  handler.closeAll();
  server.send(200, "text/html", message);
}
void handle_oppositeAll(){
  String message = " <meta http-equiv='refresh' content='1; URL=/' />";
  handler.oppositeAll();
  server.send(200, "text/html", message);
}
