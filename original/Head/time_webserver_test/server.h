#ifndef SERVER_H
#define SERVER_H
#include <Arduino.h>
#include "time.h"
#include <stdlib.h>
#include <ESP8266WebServer.h>

class serverHandle{
	private:
		//this is being designed so that the cages can be opened and closed x number of times a day
		char functions[3][10] = {"close", "open","opposite"};
		time_handler::t_object *changes;
		int change_amt;
		void timeBufferString(int index, char *buffer);
		char* timeBufferString(int index);
		void timeBufferString(char *buffer);
		char* timeBufferString();
	public:
    time_handler::t_object add_time;
    String makePage();
		serverHandle();
		void addTime(int hour, int minute, int second, int open);
		void addTime(time_handler::t_object ob);
		void editTime(int index, int hour, int minute, int second);
    void removeTime(int index);
		time_handler::t_object retrieveTime(int index);
		void freeClass(){if(change_amt > 0){free(changes);}}
		time_handler::t_object* checkTime(int interval=15);
    int get_changeamt(){return change_amt;}
    void correctTime(int hour, int minute, int second);
};

serverHandle::serverHandle()
	:change_amt(0)
{
  add_time.hour = 0;
  add_time.second = 0;
  add_time.minute = 0;
  }

void serverHandle::addTime(int hour, int minute, int second, int open){
	time_handler::t_object toAdd = time_handler::getTime(hour, minute, second, open);
	change_amt++;
	changes = (time_handler::t_object*)realloc(changes, sizeof(time_handler::t_object)*change_amt);
	changes[change_amt-1] = toAdd;
}

void serverHandle::addTime(time_handler::t_object ob){
	change_amt++;
	changes = (time_handler::t_object*)realloc(changes, sizeof(time_handler::t_object)*change_amt);
	changes[change_amt-1] = ob;
}
time_handler::t_object serverHandle::retrieveTime(int index){
	return changes[index];
}

void serverHandle::timeBufferString(int index, char *buffer){
	time_handler::t_object t = retrieveTime(index);
	sprintf(buffer, "Currently set to: %02d:%02d:%02d function: ", t.hour, t.minute, t.second, functions[t.open]);
}

char* serverHandle::timeBufferString(int index){
	time_handler::t_object t = retrieveTime(index);
	char *buffer = (char*)malloc(50);
	int n = sprintf(buffer, "Currently set to: %02d:%02d:%02d function: %s", t.hour, t.minute, t.second, functions[t.open]);
	buffer[n] = '\0';
	return buffer;
}

void serverHandle::timeBufferString(char* buffer){
	time_handler::t_object t = time_handler::getNow() + add_time;
	sprintf(buffer, "Current time: %02d:%02d:%02d", t.hour, t.minute, t.second);
}

char* serverHandle::timeBufferString(){
	time_handler::t_object t = time_handler::getNow() + add_time;
	char *buffer = (char*)malloc(28);
	int n = sprintf(buffer, "Current time: %02d:%02d:%02d", t.hour, t.minute, t.second);
	buffer[n] = '\0';
	return buffer;
}

time_handler::t_object* serverHandle::checkTime(int interval){
	time_handler::t_object now = time_handler::getNow() + add_time;
	for(int i = 0; i < change_amt; i++){
		if(changes[i] >= now && (changes[i]+(interval*60)) <= now){
			return &(changes[i]);
		}
	}
	return NULL;
}

String serverHandle::makePage(){
	String page = "<meta name='viewport' content='width=device-width, initial-scale=1'><form action='setServerTimes'><h2>Time:</h2><p>";
  char *currentTimeBuffer = timeBufferString();
	page += currentTimeBuffer;
	free(currentTimeBuffer);
	for(int i = 0; i < change_amt; i++){
		page += "</p><h2>Alarm "+String(i+1)+":</h2><input type='text' name ='timeh"+String(i)+"' placeholder='h' size=1>:<input type='text' name ='timem"+String(i)+"' placeholder='m' size=1>:<input type='text' name ='times"+String(i)+"' placeholder='s' size=1> <p>"; //This line allows the user to input their own time for the server's time.
		char *timBuffer = timeBufferString(i);
		page += timBuffer;
		free(timBuffer);
	}
  //this is the submit line for changing the server time
	page += "</p><br><br><input type='submit' value='Set times'></form>";
  //this is the line for correcting the server time
  //page += "<form action='correctTime'><h2>Correct Server Time:</h2><p><input type='text' name='correctionh' size=1>:<input type='text' name='correctionm' size=1>:<input type='text' name='corrections' size=1></p><br><br><input type='submit' value='Submit time'></form>";
  //this is the line for adding an alarm
  page += "</p><br><br><form action='addAlarm'><h2> Create Alarm: </h2><p><input type='text' name='addh' size=2>:<input type='text' name='addm' size=2>:<input type='text' name='adds' size=2>     Function: <select name='subject' id='subject'><option value='1' selected='selected'>Open</option><option value='0' selected='selected'> Close</option><option value='2' selected='selected'> Opposite</option></select><input type='submit' value='Create Alarm'></form>";
  //this is adding the remove alarm
  page += "</p><br><br><form action='removeAlarm'><h3> Remove alarm Alarm: </h3> <select name='removeSubject' id='removeSubject'>";
  for(int i = 0; i < change_amt; i++){
    page += "<option value='"+String(i)+"' selected='selected'>Alarm "+String(i+1)+"</option>";
  }
  page += "</select></p><br><input type='submit' value='Remove Alarm'></form>";
  return page;
}

void serverHandle::editTime(int index, int hour, int minute, int second){
	while(second >= 60){
		minute += 1;
		second -= 60;
	}
	while(minute >= 60){
		hour++;
		minute -= 60;
	}
	while(hour >= 24)
		hour -= 24;
	changes[index].hour = hour;
	changes[index].minute = minute;
	changes[index].second = second;
}

void serverHandle::correctTime(int hour, int minute, int second){
  add_time.hour = hour;
  add_time.minute = minute;
  add_time.second = second;
  time_handler::t_object now = time_handler::getNow();
  add_time = add_time - now;
}

void serverHandle::removeTime(int index){
  if(index >= change_amt){return;}
  time_handler::t_object holder[change_amt];
  for(int i = 0; i < change_amt; i++){holder[i] = changes[i];}
  change_amt--;
  changes = (time_handler::t_object*)realloc(changes, sizeof(time_handler::t_object)*change_amt);
  int counter = 0;
  for(int i = 0; i <= change_amt; i++){ if(i != index){changes[counter] = holder[i];counter++;}}
}

#endif
