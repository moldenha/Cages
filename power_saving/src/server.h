#ifndef SERVER_H
#define SERVER_H
#include <Arduino.h>
#include "time.h"
#include <stdlib.h>
#include <string.h>
#include <ESP8266WebServer.h>
#include "basics.h"

class serverHandle{
	private:
		//this is being designed so that the cages can be opened and closed x number of times a day
		const char functions[3][10] = {"close", "open","opposite"};
		my_vector<pairs<n_string, bool>> passed;
		n_string myIp;
		n_string curr_command;
		my_vector<n_string> servos;
		int servo_port = 90;
		my_vector<time_handler::t_object> changes;
		n_string timeBufferString(int index);
		n_string timeBufferString();
		void clearPassed(){passed.clear();}
	public:
    		time_handler::t_object add_time;
    		String makePage();
    		void addMyIp(const char *ip){myIp = ip;}
		serverHandle();
		void addTime(int hour, int minute, int second, int open);
		void editTime(int index, int hour, int minute, int second);
    		void removeTime(int index);
    		int get_changeamt(){return changes.size();}
    		void correctTime(int hour, int minute, int second);
		void openAll();
		bool open(const char *ip);
		void closeAll();
		bool close(const char *ip);
		void oppositeAll();
		bool opposite(const char *ip);
		void findServos(int lower = 100, int upper = 179);
		bool inServos(const char *ip);
		void addServo(const char *ip){servos.push_back(n_string(ip));}
		bool checkConnection(const char *ip);
		void sendAlarmsAll();
		bool sendAlarms(const char *ip);
		void sendCorrectTimeAll();
		bool sendCorrectTime(const char *ip);
		bool all_passed();
		bool sendCommand(const char* ip, n_string command);
		void check();
};

serverHandle::serverHandle()
{
  Serial.println("new server handle called");
  add_time.hour = 0;
  add_time.second = 0;
  add_time.minute = 0;
  add_time.open = 0;
}

void serverHandle::addTime(int hour, int minute, int second, int open){
	Serial.println("getting object");
	time_handler::t_object toAdd = time_handler::getTime(hour, minute, second, open);
	Serial.println("pushing back");
	changes.push_back(toAdd);
  Serial.println("sending alarms");
	sendAlarmsAll();
  Serial.println("sent alarms");
}

bool serverHandle::all_passed(){
	for(int i = 0; i < passed.size(); i++){
		if(passed.at(i).second == false)
			return false;
	}
	return true;
}

n_string serverHandle::timeBufferString(int index){
	time_handler::t_object t = changes.at(index);
	char *buffer = (char*)malloc(50);
	int n = sprintf(buffer, "Currently set to: %02d:%02d:%02d function: %s", t.hour, t.minute, t.second, functions[t.open]);
	buffer[n] = '\0';
	n_string returning(buffer);
	free(buffer);
	return returning;
}

n_string serverHandle::timeBufferString(){
	time_handler::t_object t = time_handler::getNow() + add_time;
	char *buffer = (char*)malloc(28);
	int n = sprintf(buffer, "Current time: %02d:%02d:%02d", t.hour, t.minute, t.second);
	buffer[n] = '\0';
	n_string returning(buffer);
	free(buffer);
	return returning;
}


String serverHandle::makePage(){
	String page = "<meta name='viewport' content='width=device-width, initial-scale=1'><form action='setServerTimes'><h2>Time:</h2><p>";
	n_string currentTimeBuffer = timeBufferString();
	page += currentTimeBuffer.c_str();
	for(int i = 0; i < changes.size(); i++){
		page += "</p><h2>Alarm "+String(i+1)+":</h2><input type='text' name ='timeh"+String(i)+"' placeholder='h' size=1>:<input type='text' name ='timem"+String(i)+"' placeholder='m' size=1>:<input type='text' name ='times"+String(i)+"' placeholder='s' size=1> <p>"; //This line allows the user to input their own time for the server's time.
		n_string timBuffer = timeBufferString(i);
		page += timBuffer.c_str();
	}
  //this is the submit line for changing the server time
	page += "</p><br><br><input type='submit' value='Set times'></form>";
  //this is the line for correcting the server time
  //page += "<form action='correctTime'><h2>Correct Server Time:</h2><p><input type='text' name='correctionh' size=1>:<input type='text' name='correctionm' size=1>:<input type='text' name='corrections' size=1></p><br><br><input type='submit' value='Submit time'></form>";
  //this is the line for adding an alarm
  page += "</p><br><br><form action='addAlarm'><h2> Create Alarm: </h2><p><input type='text' name='addh' size=2>:<input type='text' name='addm' size=2>:<input type='text' name='adds' size=2>     Function: <select name='subject' id='subject'><option value='1' selected='selected'>Open</option><option value='0' selected='selected'> Close</option><option value='2' selected='selected'> Opposite</option></select><input type='submit' value='Create Alarm'></form>";
  //this is adding the remove alarm
  page += "</p><br><br><form action='removeAlarm'><h3> Remove alarm Alarm: </h3> <select name='removeSubject' id='removeSubject'>";
  for(int i = 0; i < changes.size(); i++){
    page += "<option value='"+String(i)+"' selected='selected'>Alarm "+String(i+1)+"</option>";
  }
  page += "</select></p><br><input type='submit' value='Remove Alarm'></form>";
  page += "</p><p><br><form action='findServos'><input type='submit' value='Find Nodes'></form><br><br><form action='openAll'><input type='submit' value='Open All'></form><br><br><form action='closeAll'><input type='submit' value='Close All'></form><br><br><form action='oppositeAll'><input type='submit' value='Opposite All'></form></p>";
  page += "<p><br><form action='addServo'><h3> Add Known Cage IP: </h3><p><input type='text' name='servAdd' size=13>   <input type='submit' value='Add Cage'></form></p>";
  page += "<p><br><h2> Cage Numbers and their IP adddresses: </h2>";
  for(int i = 0; i < servos.size(); i++){
  	page += "<br> Cage "+String(i+1)+" "+String(servos.at(i).c_str());
  }
  page += "</p>";
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
	changes.at(index).hour = hour;
	changes.at(index).minute = minute;
	changes.at(index).second = second;
	sendAlarmsAll();
}

void serverHandle::correctTime(int hour, int minute, int second){
  add_time.hour = hour;
  add_time.minute = minute;
  add_time.second = second;
  time_handler::t_object now = time_handler::getNow();
  add_time = add_time - now;
}

void serverHandle::removeTime(int index){
	changes.erase((size_t)index);
	sendAlarmsAll();       
}


void serverHandle::openAll(){
	curr_command = "open";
	clearPassed();
	for(int i = 0; i <servos.size(); i++){
		passed.push_back({servos.at(i), open(servos.at(i).c_str())});
	}
}

bool serverHandle::open(const char *host){
	WiFiClient client;
	if(client.connect(host, servo_port)){
		Serial.print("connected to ");
		Serial.println(host);
		client.print("open");
		client.stop();
		return true;
	}
	Serial.print("unable to connect");
	return false;
}

void serverHandle::closeAll(){
	curr_command = "close";
	clearPassed();
	for(int i = 0; i <servos.size(); i++){
		passed.push_back({servos.at(i), close(servos.at(i).c_str())});
	}

}

bool serverHandle::close(const char *host){
	WiFiClient client;
	if(client.connect(host, servo_port)){
		Serial.print("connected to ");
		Serial.println(host);
		client.print("close");
		client.stop();
		return true;
	}
	Serial.print("unable to connect");
	return false;
}

void serverHandle::oppositeAll(){
	curr_command = "opposite";
	clearPassed();
	for(int i = 0; i <servos.size(); i++){
		passed.push_back({servos.at(i), opposite(servos.at(i).c_str())});
	}
}

void serverHandle::sendAlarmsAll(){
	Serial.println("send all alarms called");
	curr_command = "remake_alarms";
  Serial.println("set cur command");
	for(int i = 0; i < changes.size(); i++){
    Serial.println("first add");
		curr_command += " ";
    Serial.println("second add");
		curr_command += standard_string::to_n_string(changes.at(i).seconds());
		Serial.println("third add");
		curr_command += " ";
		Serial.println("fourth add");
    Serial.println(changes.at(i).open);
    Serial.println("putting into n_string");
    n_string open_string = standard_string::to_n_string(changes.at(i).open);
    Serial.println("made into n_stirng");
    Serial.println(open_string.c_str());
		curr_command += standard_string::to_n_string(changes.at(i).open);
	  Serial.println("fifth add");
	}
	Serial.println("finished first for loop");
	clearPassed();
	Serial.println("cleared");
	n_string s =  standard_string::to_n_string(servos.size());
	Serial.print("Size: ");
	Serial.println(s.c_str());
	for(int i = 0; i < servos.size(); i++){
		passed.push_back({servos.at(i), sendAlarms(servos.at(i).c_str())});
	}
	Serial.println("finished second for loop");
}

bool serverHandle::sendAlarms(const char *ip){
	WiFiClient client;
	if(client.connect(ip, servo_port)){
		Serial.print("connected to ");
		Serial.println(ip);
		n_string command = "remake_alarms";
		for(int i = 0; i < changes.size(); i++){
			n_string space(" ");
			n_string a = standard_string::to_n_string(changes.at(i).seconds());
			n_string b = standard_string::to_n_string(changes.at(i).open);
			command += space.c_str();
			command += a.c_str();
			command	+= space.c_str();
			command += b.c_str();
		}
		client.print(command.c_str());
		client.stop();
		return true;
	}
	Serial.print("unable to connect");
	return false;
}

bool serverHandle::sendCommand(const char *ip, n_string command){
	WiFiClient client;
	if(client.connect(ip, servo_port)){
		Serial.print("connected to ");
		Serial.println(ip);
		client.print(command.c_str());
		client.stop();
		return true;
	}
	Serial.print("unable to connect");
	return false;

}
void serverHandle::sendCorrectTimeAll(){
	curr_command = "correct_time ";
	clearPassed();
	for(int i = 0; i < servos.size(); i++){
		passed.push_back({servos.at(i), sendCorrectTime(servos.at(i).c_str())});
	}
}

bool serverHandle::sendCorrectTime(const char *ip){
	WiFiClient client;
	if(client.connect(ip, servo_port)){
		Serial.print("connected to ");
		Serial.println(ip);
		n_string command = "correct_time ";
		time_handler::t_object t = time_handler::getNow() + add_time;
		command += standard_string::to_n_string(t.seconds());
		client.print(command.c_str());
		client.stop();
		return true;
	}
	Serial.print("unable to connect");
	return false;
}

bool serverHandle::opposite(const char *host){
	WiFiClient client;
	if(client.connect(host, servo_port)){
		Serial.print("connected to ");
		Serial.println(host);
		client.print("opposite");
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
	for(int i = lower; i <= upper; i++){
		String addr = "192.168.1."+String(i);
		Serial.print("trying ");
		Serial.println(addr);
		if(checkConnection(addr.c_str()) && inServos(addr.c_str()) == false){
			Serial.print("found: ");
			Serial.println(addr);
			servos.push_back(n_string(addr.c_str()));
		}
	}
}
bool serverHandle::inServos(const char *ip){
	for(int i = 0; i <servos.size(); i++){
		if(strcmp(servos.at(i).c_str(), ip) == 0)
			return true;
	}
	return false;
}

void serverHandle::check(){
	if(all_passed()){return;}
	if(curr_command.find("correct_time") != n_string::npos){
		n_string copy(curr_command);
		time_handler::t_object t = time_handler::getNow() + add_time;
		copy += standard_string::to_n_string(t.seconds());
		for(int i = 0; i < passed.size(); i++){
			if(!passed.at(i).second){sendCommand(passed.at(i).first.c_str(), copy);}
		}
		return;
	}
	for(int i = 0; i < passed.size(); i++){
		if(!passed.at(i).second){sendCommand(passed.at(i).first.c_str(), curr_command);}
	}
}
#endif
