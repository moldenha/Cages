#include <Arduino.h>
#include "time.h"
#include <TimeLib.h>

void setup(){
  Serial.begin(115200);
  Serial.println();
}
void loop(){
  time_handler::t_object t = time_handler::getNow();
  char buf[50];
  sprintf(buf, "%02d:%02d:%02d", t.hour, t.minute, t.second);
  Serial.print(buf);
  delay(1000);
}
