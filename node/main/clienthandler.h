#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H
#include <string.h>
#include <ESP8266WiFi.h>
#include <AccelStepper.h>
#include <stdlib.h>


//even though it is named client, it really works a server
//this is a server that accepts commands from the main web server
//it then acts upon these commands by opening, closing, oppositing the cage
//or it sends the "i am here" response
class clientHandler{
	private:
		AccelStepper *stepper1;
		WiFiServer *server;
    		const int alarmMotorDistance = 2048;
		const int openPosition = -2048;
		const int closedPosition = -4096;
		const int maxSpeed = 400;
	public:
		clientHandler(AccelStepper *_stepper1, WiFiServer *_server);
		void handleClients();
		void handleClient(WiFiClient client);
		void handleCommand(char c, WiFiClient client);
		void open();
		void close();
		void opposite();
};

clientHandler::clientHandler(AccelStepper *_stepper1, WiFiServer *_server)
	:stepper1(_stepper1),
	server(_server)
{
}

void clientHandler::handleClients(){
  	//i tried making a pointer list to hold all clients
  	//this would have worked on a normal OS, however the boards
  	//seem to be too obselete to handle more than one client at a time
  	//there also should not be more than one client at a given time
  	//therefore this should not be a problem
	WiFiClient client = server->available();
	if(client){
    		Serial.println("got client");
    		handleClient(client);
	}
}

void clientHandler::handleClient(WiFiClient client){
	while(client.connected()){
    			if(client.available()){
      			char c = client.read();
      			Serial.print("got command ");
      			Serial.print(c);
      			Serial.println(" from client");
      			handleCommand(c, client);
    		}
	}
	
}

void clientHandler::handleCommand(char c, WiFiClient client){
	if(c == 'o'){open();}
	else if(c == 'c'){close();}
	else if(c == 'p'){opposite();}
	else{client.print("Here");}
	
}

void clientHandler::open(){
	stepper1->setMaxSpeed(maxSpeed);
	stepper1->moveTo(openPosition);
	while (stepper1->distanceToGo() != 0) {
		//Serial.println("E1");
		stepper1->run();
		yield();
	}
}

void clientHandler::close(){
  	stepper1->setMaxSpeed(maxSpeed);
  	stepper1->moveTo(closedPosition);
  	while (stepper1->distanceToGo() != 0) {
    		//Serial.println("E1");
    		stepper1->run();
    		yield();
  	}
}

void clientHandler::opposite(){
	stepper1->setMaxSpeed(maxSpeed);
	if(stepper1->currentPosition() == openPosition){stepper1->moveTo(closedPosition);}
	else if(stepper1->currentPosition() == closedPosition){stepper1->moveTo(openPosition);}
	else{stepper1->moveTo(stepper1->currentPosition()-alarmMotorDistance);}
	while (stepper1->distanceToGo() != 0) {
		//Serial.println("E1");
		stepper1->run();
		yield();
	}
}

#endif
