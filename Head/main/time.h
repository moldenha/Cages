#ifndef TIME_H
#define TIME_H
#include <TimeLib.h>

#define OPEN 1 //function: open
#define CLOSE 0 //function: close
#define UNDEFINED 2 //function: opposite

//this is a namespace to handle all the time parts
namespace time_handler{
	//structure object, similar to a class except by default public
	//there will be a new instance of this structure created everytime that time needs to be handled
	typedef struct t_object{
		//if open is true (1), then the cages are going to be opened when this time is hit
		//if open is false (0), then the cages are going to be closed with this time is hit;
		//if open is undefined(2), then the cages are just going to change their current position
		int second, minute, hour, open;
		//check if they are the same
		bool operator == (const t_object &ob){
			int total = (hour*60*60)+(minute*60)+second;
			int ob_total = (ob.hour*60*60) + (ob.minute*60)+second;
			return total == ob_total;	
		}
		//check if the objects are <=
		bool operator <= (const t_object &ob){
			int total = (hour*60*60)+(minute*60)+second;
			int ob_total = (ob.hour*60*60) + (ob.minute*60)+second;
			return total <= ob_total;
		}
		//check if the objects are >=
		bool operator >= (const t_object &ob){
			int total = (hour*60*60)+(minute*60)+second;
			int ob_total = (ob.hour*60*60) + (ob.minute*60)+second;
			return total >= ob_total;
		}
		//check if the objects are >
		bool operator > (const t_object &ob){
			int total = (hour*60*60)+(minute*60)+second;
			int ob_total = (ob.hour*60*60) + (ob.minute*60)+second;
			return total > ob_total;
		}
		//check if the objects are <
		bool operator < (const t_object &ob){
			int total = (hour*60*60)+(minute*60)+second;
			int ob_total = (ob.hour*60*60) + (ob.minute*60)+second;
			return total > ob_total;
		}
		//add to objects of a t_object instance
		//this will be used in correcting the times
		t_object operator + (t_object &ob){
      			t_object returning;
      			returning.hour = hour + ob.hour;
      			returning.minute = minute + ob.minute;
      			returning.second = second + ob.second;
      			while(returning.second >= 60){
        			returning.minute++;
        			returning.second -= 60;
      			}
      			while(returning.minute >= 60){
        			returning.hour++;
        			returning.minute -= 60;
      			}
      			while(returning.hour >= 24)
        			returning.hour -= 24;
      				returning.open = open;
      				return returning;
    			}
		//this is a subtraction of two time objects
    		t_object operator - (t_object &ob){
      			t_object returning;
      			returning.hour = hour - ob.hour;
      			returning.minute = minute - ob.minute;
      			returning.second = second - ob.second;
      			while(returning.second >= 60){
        			returning.minute++;
        			returning.second -= 60;
      			}
      			while(returning.minute >= 60){
        			returning.hour++;
        			returning.minute -= 60;
      			}
      			while(returning.hour >= 24)
        			returning.hour -= 24;
      			returning.open = open;
      			return returning;
    			}
	} t_object;
  	//this function gets the current time in relation to the set time by the chip
	t_object getNow(){
		time_t t = now();
		t_object returning;
		returning.hour = hour(t);
		returning.minute = minute(t);
		returning.second = second(t);
		returning.open = -1;
		return returning;
	}
	//this returns a new t_object instance of the specified time
	t_object getTime(int hour=7, int minute=0, int second=0, int open=-1){
		t_object returning;
		returning.hour = hour;
		returning.minute = minute;
		returning.second = second;
		returning.open = open;
		return returning;
	}

}

#endif
