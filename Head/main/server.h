#ifndef SERVER_H
#define SERVER_H
#include <Arduino.h>
#include "time.h"
#include <stdlib.h>
#include <string.h>
#include <ESP8266WebServer.h>

class serverHandle{
	private:
		//this is being designed so that the cages can be opened and closed x number of times a day
		char functions[3][10] = {"close", "open","opposite"};
		//char *myIp;
		char **servos;
		int servos_total = 0;
		int servo_port = 90;
		time_handler::t_object *changes;
		int change_amt;
		void timeBufferString(int index, char *buffer);
		char* timeBufferString(int index);
		void timeBufferString(char *buffer);
		char* timeBufferString();
		void addServo(const char *ip);
		void clearServos(){
			for(int i = 0; i < servos_total; i++){free(servos[i]);}
			if(servos_total > 0){free(servos);}
			servos_total= 0;
		}
		void reallocServos(){
			for(int i = 0; i < servos_total; i++){free(servos[i]);}
			if(servos_total > 0){servos = (char**)realloc(servos, 1);}
			servos_total= 0;
		}

		time_handler::t_object *last = NULL;
	public:
		//add time variable the time added to the current got time by the program
		//and then added so the correct time is displayed
    		time_handler::t_object add_time;
    		String makePage();
    		//void addMyIp(const char *ip){strcpy(myIp, ip);}
		serverHandle();
		void addTime(int hour, int minute, int second, int open);
		void addTime(time_handler::t_object ob);
		void editTime(int index, int hour, int minute, int second);
    		void removeTime(int index);
		time_handler::t_object retrieveTime(int index);
		void freeClass(){if(change_amt > 0){free(changes);}
		clearServos();}
		time_handler::t_object* checkTime(int interval=15);
    		int get_changeamt(){return change_amt;}
    		void correctTime(int hour, int minute, int second);
		void openAll();
		bool open(const char *ip);
		void closeAll();
		bool close(const char *ip);
		void oppositeAll();
		bool opposite(const char *ip);
		void findServos(int lower = 100, int upper = 79);
		bool inServos(const char *ip);
		bool checkConnection(const char *ip);
		void handleAlarms(int interval);
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
  time_handler::t_object nowInterval = now;
  nowInterval.minute + interval;
	for(int i = 0; i < change_amt; i++){
		if(changes[i] >= now && changes[i] <= nowInterval){
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
  page += "<form action='correctTime'><h2>Correct Server Time:</h2><p><input type='text' name='correctionh' size=1>:<input type='text' name='correctionm' size=1>:<input type='text' name='corrections' size=1></p><br><br><input type='submit' value='Submit time'></form>";
  //this is the line for adding an alarm
  page += "</p><br><br><form action='addAlarm'><h2> Create Alarm: </h2><p><input type='text' name='addh' size=2>:<input type='text' name='addm' size=2>:<input type='text' name='adds' size=2>     Function: <select name='subject' id='subject'><option value='1' selected='selected'>Open</option><option value='0' selected='selected'> Close</option><option value='2' selected='selected'> Opposite</option></select><input type='submit' value='Create Alarm'></form>";
  //this is adding the remove alarm
  page += "</p><br><br><form action='removeAlarm'><h3> Remove alarm Alarm: </h3> <select name='removeSubject' id='removeSubject'>";
  for(int i = 0; i < change_amt; i++){
    page += "<option value='"+String(i)+"' selected='selected'>Alarm "+String(i+1)+"</option>";
  }
  page += "</select></p><br><input type='submit' value='Remove Alarm'></form>";
  page += "</p><br><form action='findServos'><input type='submit' value='Find Nodes'></form><br><br><form action='openAll'><input type='submit' value='Open All'></form><br><br><form action='closeAll'><input type='submit' value='Close All'></form><br><br><form action='oppositeAll'><input type='submit' value='Opposite All'></form>";
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

void serverHandle::addServo(const char *ip){
	if(inServos(ip)){return;}
	servos_total++;
	Serial.println("allocating");
	servos = (char**)realloc(servos, sizeof(char*)*servos_total);
	Serial.println("setting");
	Serial.println(sizeof(servos));
	servos[servos_total-1] = (char*)malloc(sizeof(char)*strlen(ip));
	strcpy(servos[servos_total-1], ip);
}
void serverHandle::openAll(){
	for(int i = 0; i <servos_total; i++){
		bool returned = open(servos[i]);
	}
}

bool serverHandle::open(const char *host){
	WiFiClient client;
	if(client.connect(host, servo_port)){
		Serial.print("connected to ");
		Serial.println(host);
		client.print("o");
		client.stop();
		return true;
	}
	Serial.print("unable to connect");
	return false;
}

void serverHandle::closeAll(){
	for(int i = 0; i <servos_total; i++){
		bool returned = close(servos[i]);
	}

}

bool serverHandle::close(const char *host){
	WiFiClient client;
	if(client.connect(host, servo_port)){
		Serial.print("connected to ");
		Serial.println(host);
		client.print("c");
		client.stop();
		return true;
	}
	Serial.print("unable to connect");
	return false;
}

void serverHandle::oppositeAll(){
	for(int i = 0; i <servos_total; i++){
		bool returned = opposite(servos[i]);
	}

}

bool serverHandle::opposite(const char *host){
	WiFiClient client;
	if(client.connect(host, servo_port)){
		Serial.print("connected to ");
		Serial.println(host);
		client.print("p");
		client.stop();
		return true;
	}
	Serial.print("unable to connect");
	return false;
}

bool serverHandle::checkConnection(const char *ip){
	WiFiClient client;
	if(client.connect(ip, servo_port)){
		client.stop();
		return true;
	}
	return false;
}

void serverHandle::findServos(int lower, int upper){
	reallocServos();
	for(int i = lower; i <= upper; i++){
		String addr = "192.168.1."+String(i);
		Serial.print("trying ");
		Serial.println(addr);
		if(checkConnection(addr.c_str())){
			Serial.print("found: ");
			Serial.println(addr);
			addServo(addr.c_str());
		}
	}
}
bool serverHandle::inServos(const char *ip){
	for(int i = 0; i <servos_total; i++){
		if(strcmp(servos[i], ip) == 0)
			return true;
	}
	return false;
}

void serverHandle::handleAlarms(int interval){
	time_handler::t_object* checking = checkTime(interval);
	if(checking != NULL){
		if(checking->open == 2){
			if(last == NULL){
				last = checking;
				oppositeAll();
			}
			else if(last != NULL and ((*last) - (*checking)).minute > interval){
				last = checking;
				oppositeAll();
			}
		}
		if(checking->open == 1){
			openAll();
		}
		if(checking->open == 0){
			closeAll();
		}
	}
}

#endif
