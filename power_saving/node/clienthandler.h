#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H
#include <string.h>
#include <ESP8266WiFi.h>
#include <AccelStepper.h>
#include <stdlib.h>
#include "basics.h"
#include "time.h"
//the new way in which this process is going to work:
//



//even though it is named client, it really works a server
//this is a server that accepts commands from the main web server
//it then acts upon these commands by opening, closing, oppositing the cage
//or it sends the "i am here" response

class clientHandler{
	private:
		AccelStepper *stepper1;
		WiFiServer *server;
	  	const int alarmMotorDistance = 2048;
		time_handler::t_object add_time;
		my_vector<time_handler::t_object> alarms;
		//10 minutes
		const int stop_delay_before_cages = 10;
		//5 minutes
		const int time_between_delays = 5;
	public:
		clientHandler(AccelStepper *_stepper1, WiFiServer *_server);
		//this variable is important because in between each delay it will be open to commands for 5 minutes
		time_handler::t_object last_delay;
		void handleClients();
		void handleClient(WiFiClient client);
		void handleCommand(n_string c, WiFiClient client);
		void correct_time(n_string c);
		void open();
		void close();
		void opposite();
		time_handler::t_object currentTime();
		void add_alarm(n_string c);
		int getDelay();
		bool do_delay();
		time_handler::t_object *nextAlarm();
		bool pastAllAlarms();
		void remake_alarms(n_string c);
		void handle_alarm_commands();
    void mark_last_delay();
    n_string client_read(WiFiClient client);
    n_string n_str(time_handler::t_object input){
      char *buffer = (char*)malloc(15);
      int n = sprintf(buffer, "%02d:%02d:%02d", input.hour, input.minute, input.second);
      n_string returning(buffer);
      free(buffer);
      return returning;
    }
};

clientHandler::clientHandler(AccelStepper *_stepper1, WiFiServer *_server)
	:stepper1(_stepper1),
	server(_server)
{
	add_time.hour = 0;
	add_time.second = 0;
	add_time.minute = 0;
  last_delay.second = 0;
  last_delay.minute = 0;
  last_delay.hour = 0;
}

void clientHandler::handleClients(){
  //i tried making a pointer list to hold all clients
  //this would have worked on a normal OS, however the boards
  //seem to be too obselete to handle more than one client at a time
  //there also should not be more than one client at a given time
  //therefore this should not be a problem
	WiFiClient client = server->available();
	if(client){
		//Serial.println("got client");
		handleClient(client);
	}
}

//this checks if all the alarms for the day have already happened
//if they have this returns true
//important for finding the next alarm
bool clientHandler::pastAllAlarms(){
	//returns false if there are no scheduled alarms
	if(alarms.size() == 0){	return false; }
	//gets current time
	time_handler::t_object now = currentTime();
	//gets the seconds
	int now_seconds = now.seconds();
	for(int i = 0; i < alarms.size(); i++){
		int alarm_seconds = alarms.at(i).seconds();
		//if there is one that the next time is later than right now, return false
		if((alarm_seconds - now_seconds) > 0)
			return false;	
	}
	//condition never met return true
	return true;
}

time_handler::t_object *clientHandler::nextAlarm(){
	//returns the NULL pointer if there are no scheduled alarms
	if(alarms.size() == 0){return NULL;}
	//gets the current time
	time_handler::t_object now = currentTime();
	//gets the current time in the amount of seconds since the start of the day
	int now_seconds = now.seconds();
	//sets the default next alarm to the first alarm in the list
	//then it checks if there is an earlier one
	time_handler::t_object *current = &alarms.at(0);
	for(int i = 0; i < alarms.size(); i++){
		//gets the amount of seconds since the start of the day till the next alarm
		int alarm_seconds = alarms.at(i).seconds();
		//if all the alarms for the day have already been done
		if(pastAllAlarms()){
			//get the difference of the alarm currently checking
			int dif = alarm_seconds - now_seconds;
			//get the difference of the alarm that is currently next
			int curDif = current->seconds() - now_seconds;
			//if the difference is less, then make the new next the alarm currently being checked
			if(abs(dif) < abs(curDif)){
				current = &alarms.at(i);
			}
		}
		//otherwise, if the alarm has not passed
		else if((alarm_seconds - now_seconds) > 0){
			//get the difference of the alarm currently checking
			int dif = alarm_seconds - now_seconds;
			//get the difference of the alarm that is currently next
			int curDif = current->seconds() - now_seconds;
			//if the alarm currently next, make the next one being the one being checked
			if(curDif < 0)
				current = &alarms.at(i);
			//otherwise if this one is closer make this one the next one
			else if( dif < curDif)
				current = &alarms.at(i);
		}
	}
	return current;
}

int clientHandler::getDelay(){
	//if there are no alarms set yet, then there will be no delay
	//it will be waiting for an alarm before there is a delay
	if(alarms.size() == 0){
	  return 0;
	  }
	//gets the next alarm
	time_handler::t_object *next = nextAlarm();
	//re-itterates if there are no alarms no delay
	if(next == NULL){
    //Serial.println("was null");
	  return 0;
	}
	//gets current time
  //Serial.println("got next alarm");
	time_handler::t_object now = currentTime();
	int now_seconds = now.seconds();
	int next_seconds = next->seconds();
	//gets the difference in seconds between now and the next alarm
	int diff = now_seconds - next_seconds;
	//now it checks if it is in the next 10 minutes (or last 10 minutes)
	//if so, there is no delay
	diff = abs(diff);
  //Serial.print("diff is: ");
  //Serial.println(diff);
	if((diff/60) <= stop_delay_before_cages){
    //Serial.print(diff/60);
    //Serial.print(" ");
    //Serial.print(stop_delay_before_cages);
    //Serial.print(" ");
    //Serial.println("returning 0");
		return 0;
   }
	//otherwise, return half the time between now and then and that's how long the delay will be for
  int output = (int)diff/2;
  //Serial.print("returning: ");
  //Serial.println(output);
	return output;
}

n_string clientHandler::client_read(WiFiClient client){
  n_string returning;
  while(true){
    int re = client.read();
    if(re == -1){break;}
    char input = re;
    returning += input;
  }
  return returning;
}

void clientHandler::handleClient(WiFiClient client){
	while(client.connected()){
    		if(client.available() > 0){
      			//get the string command
			n_string c = client_read(client);
      			//Serial.print("got command ");
      			//Serial.print(c.c_str());
      			//Serial.println(" from client");
			//handle the string command
			my_vector<n_string> commands;
			n_string command;
			for(int i = 0; i < c.size(); i++){
				if(c.at(i) == '\n'){
					commands.push_back(command);
					command.clear();
				}
				else{
					command.push_back(c.at(i));
				}
			}
			if(command.size() > 0)
				commands.push_back(command);
      			for(int i = 0; i < commands.size(); i++)
				handleCommand(commands.at(i).c_str(), client);
			//free the allocated space to store the command
    		}
	}
}

//this function takes the current actual time from the server, then translates it to
//a "readable" version so that it can tell the time
void clientHandler::correct_time(n_string c){
	//the current time in seconds is seperated by a space that is before the command correct_time
  n_string num;
  bool space = false;
  for(int i = 0; i < c.size(); i++){
    if(c.at(i) == ' '){
      space = true;
    }
    else if(space == true){
      num += c.at(i);
    }
  }
	int seconds = atoi(num.c_str());
	time_handler::t_object corrected = time_handler::getTime(seconds);
	//put into the add_time variable
	time_handler::t_object t = time_handler::getNow();
	add_time = (corrected - t);
  return;
}

//gets the current time
time_handler::t_object clientHandler::currentTime(){
	time_handler::t_object now = time_handler::getNow();
	now += add_time;
	return now;
}


//this adds all the alarms sent to the server
//first it is the command
//then a space
//then the amount of seconds in the day until an alarm
//then the function (open : 1), (close : 0), (opposite : 2)
void clientHandler::add_alarm(n_string c){
  if(c.find(" ") == n_string::npos){
    //Serial.println("space not found");
  }
	my_vector<n_string> split = c.split(" ");
  //Serial.print("amount after split: ");
  //Serial.println(split.size());
	for(int i = 1; i < split.size(); i++){
		int seconds = atoi(split.at(i).c_str());
		i++;
		int mode = atoi(split.at(i).c_str());
		time_handler::t_object new_alarm = time_handler::getTime(seconds);
		new_alarm.open = mode;
		alarms.push_back(new_alarm);	
	}
}

//this is when new alarms need to be given to the cage
void clientHandler::remake_alarms(n_string c){
	alarms.clear();
	add_alarm(c);
  //Serial.print("alarm size: ");
  //Serial.println(alarms.size());
  for(int i = 0; i < alarms.size(); i++){
    //Serial.println(n_str(alarms.at(i)).c_str());
  }
}

//this function is called when the delay is 0
//a delay being 0 either means there are no delays
//or, this means it is time for another alarm command to be activated
//this decides which, and if it is time to activate one, which function to complete
void clientHandler::handle_alarm_commands(){
	time_handler::t_object *next = nextAlarm();
	if(next == NULL)
		return;
	if(next->open == 1){open();}
	if(next->open == 0){close();}
	if(next->open == 2){opposite();}
}

void clientHandler::handleCommand(n_string c, WiFiClient client){
	if(c.find("open") != n_string::npos){open();}
	else if(c.find("close") != n_string::npos){close();}
	else if(c.find("opposite") != n_string::npos){opposite();}
	else if(c.find("correct_time") != n_string::npos){
	    correct_time(c);
      time_handler::t_object n = currentTime();
      //Serial.print("new time: ");
      //Serial.println(n_str(n).c_str());
	  }	
	else if(c.find("add_alarm") != n_string::npos){add_alarm(c);}
	else if(c.find("remake_alarms") != n_string::npos){remake_alarms(c);}
	else{client.print("Here");}
	
}

void clientHandler::open(){
	stepper1->setMaxSpeed(400);
	stepper1->moveTo(-2048);
	while (stepper1->distanceToGo() != 0) {
		////Serial.println("E1");
		stepper1->run();
		yield();
	}
}

void clientHandler::close(){
  stepper1->setMaxSpeed(400);
  stepper1->moveTo(-4096);
  while (stepper1->distanceToGo() != 0) {
    ////Serial.println("E1");
    stepper1->run();
    yield();
  }
}

void clientHandler::opposite(){
	stepper1->setMaxSpeed(400);
	if(stepper1->currentPosition() == -2048){stepper1->moveTo(0);}
	else if(stepper1->currentPosition() == -4096){stepper1->moveTo(-2048);}
	else{stepper1->moveTo(stepper1->currentPosition()-alarmMotorDistance);}
	while (stepper1->distanceToGo() != 0) {
		////Serial.println("E1");
		stepper1->run();
		yield();
	}
}

void clientHandler::mark_last_delay(){last_delay = currentTime();}

bool clientHandler::do_delay(){
  int last_seconds = last_delay.seconds();
  //Serial.print("last delay seconds: ");
  //Serial.println(last_seconds);
  time_handler::t_object n = currentTime();
  int now_seconds = n.seconds();
  //Serial.print("now seconds: ");
  //Serial.println(now_seconds);
  int dif = now_seconds - last_seconds;
  //Serial.print("do delay dif: ");
  //Serial.println(dif);
  if(abs(dif/60) < time_between_delays){
    //Serial.println("returning false");
    return false;
  }
  //Serial.println("returning true");
  return true;
}

#endif
