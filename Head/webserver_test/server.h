#ifndef SERVER_H
#define SERVER_H
#include <Arduino.h>
#include "time.h"
#include <stdlib.h>
#include <ESP8266WebServer.h>

class server{
	private:
		//this is being designed so that the cages can be opened and closed x number of times a day
		time_handler::t_object *changes;
		int change_amt;
		void timeBufferString(int index, char *buffer);
		char* timeBufferString(int index);
		void timeBufferString(char *buffer);
		char* timeBufferString();
	public:
    String makePage();
		server();
		void addTime(int hour, int minute, int second, int open);
		void addTime(time_handler::t_object ob);
		void editTime(int index, int hour, int minute, int second);
		time_handler::t_object retrieveTime(int index);
		void freeClass(){if(change_amt > 0){free(changes);}}
		time_handler::t_object* checkTime(int interval=15);
   int get_changeamt(){return change_amt;}
};

server::server()
	:change_amt(0)
{}

void server::addTime(int hour, int minute, int second, int open){
	time_handler::t_object toAdd = time_handler::getTime(hour, minute, second, open);
	change_amt++;
	changes = (time_handler::t_object*)realloc(changes, sizeof(time_handler::t_object)*change_amt);
	changes[change_amt-1] = toAdd;
}

void server::addTime(time_handler::t_object ob){
	change_amt++;
	changes = (time_handler::t_object*)realloc(changes, sizeof(time_handler::t_object)*change_amt);
	changes[change_amt-1] = ob;
}
time_handler::t_object server::retrieveTime(int index){
	return changes[index];
}

void server::timeBufferString(int index, char *buffer){
	time_handler::t_object t = retrieveTime(index);
	sprintf(buffer, "Currently set to: %02d:%02d:%02d", t.hour, t.minute, t.second);
}

char* server::timeBufferString(int index){
	time_handler::t_object t = retrieveTime(index);
	char *buffer = (char*)malloc(28);
	int n = sprintf(buffer, "Currently set to: %02d:%02d:%02d", t.hour, t.minute, t.second);
	buffer[n] = '\0';
	return buffer;
}

void server::timeBufferString(char* buffer){
	time_handler::t_object t = time_handler::getNow();
	sprintf(buffer, "Current time: %02d:%02d:%02d", t.hour, t.minute, t.second);
}

char* server::timeBufferString(){
	time_handler::t_object t = time_handler::getNow();
	char *buffer = (char*)malloc(28);
	int n = sprintf(buffer, "Current time: %02d:%02d:%02d", t.hour, t.minute, t.second);
	buffer[n] = '\0';
	return buffer;
}

time_handler::t_object* server::checkTime(int interval){
	time_handler::t_object now = time_handler::getNow();
	for(int i = 0; i < change_amt; i++){
		if(changes[i] >= now && (changes[i]+(interval*60)) <= now){
			return &(changes[i]);
		}
	}
	return NULL;
}

String server::makePage(){
	String page = "<meta name='viewport' content='width=device-width, initial-scale=1'><form action='setServerTimes'><h2>Time:</h2><p>";
  char *currentTimeBuffer = timeBufferString();
	page += currentTimeBuffer;
	free(currentTimeBuffer);
	for(int i = 0; i < change_amt; i++){
		page += "</p><input type='text' name ='timeh"+String(i)+"' placeholder='h' size=1>:<input type='text' name ='timem"+String(i)+"' placeholder='m' size=1>:<input type='text' name ='times"+String(i)+"' placeholder='s' size=1> <h2>Alarm "+String(i+1)+":</h2><p>"; //This line allows the user to input their own time for the server's time.
		char *timBuffer = timeBufferString(i);
		page += timBuffer;
		free(timBuffer);
	}
	page += "<br><br><input type='submit' value='Set times'></form>";
  return page;
}

void server::editTime(int index, int hour, int minute, int second){
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

#endif
