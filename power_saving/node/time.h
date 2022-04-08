#ifndef TIME_H
#define TIME_H
#include <TimeLib.h>

#define OPEN 1 //function: open
#define CLOSE 0 //function: close
#define UNDEFINED 2 //function: opposite

namespace time_handler{
	struct t_object{
		//if open is true (1), then the cages are going to be opened when this time is hit
		//if open is false (0), then the cages are going to be closed with this time is hit;
		//if open is undefined(2), then the cages are just going to change their current position
		int second, minute, hour, open;
		t_object()
			:second(0), minute(0), hour(0)
			{}
		bool operator == (const t_object &ob){
			int total = (hour*60*60)+(minute*60)+second;
			int ob_total = (ob.hour*60*60) + (ob.minute*60)+second;
			return total == ob_total;	
		}
		bool operator <= (const t_object &ob){
			int total = (hour*60*60)+(minute*60)+second;
			int ob_total = (ob.hour*60*60) + (ob.minute*60)+second;
			return total <= ob_total;
		}
		bool operator >= (const t_object &ob){
			int total = (hour*60*60)+(minute*60)+second;
			int ob_total = (ob.hour*60*60) + (ob.minute*60)+second;
			return total >= ob_total;
		}
		bool operator > (const t_object &ob){
			int total = (hour*60*60)+(minute*60)+second;
			int ob_total = (ob.hour*60*60) + (ob.minute*60)+second;
			return total > ob_total;
		}
		bool operator < (const t_object &ob){
			int total = (hour*60*60)+(minute*60)+second;
			int ob_total = (ob.hour*60*60) + (ob.minute*60)+second;
			return total > ob_total;
		}
		//the amount of seconds to add to the interval
		t_object operator + (int seconds_to_add){
			t_object returning;
			returning.hour = hour;
			returning.minute = minute;
			returning.second = second;
			returning.open = open;
		}
		int seconds(){return (hour*60*60)+(minute*60)+second;}
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
	    t_object operator - (t_object &ob){
	      t_object returning;
	      returning.hour = this->hour - ob.hour;
	      returning.minute = this->minute - ob.minute;
	      returning.second = this->second = ob.second;
	      while(returning.second >= 60){
		returning.minute++;
		returning.second -= 60;
	      }
	      while(returning.minute >= 60){
		returning.hour++;
		returning.minute -= 60;
	      }
	      int h = returning.hour;
	      while (h >= 24){h -= 24;}
	      //Serial.println("set h");
	      returning.hour = h;
	      returning.open = this->open;
	      return returning;
	    }
		void operator += (t_object &ob){
			*this = (*this + ob);
		}
		
	};
  
	t_object getNow(){
    ////Serial.println("getting now");
		time_t t = now();
    ////Serial.println("got now");
		t_object returning;
    ////Serial.println("setting returning");
		returning.hour = hour(t);
		returning.minute = minute(t);
		returning.second = second(t);
		returning.open = -1;
		return returning;
	}
	t_object getTime(int hour, int minute, int second, int open){
		t_object returning;
		returning.hour = hour;
		returning.minute = minute;
		returning.second = second;
		returning.open = open;
		return returning;
	}
	t_object getTime(int seconds){
		////Serial.println("initiating the seconds");
		t_object returning;
		int hours, minutes;
    hours = 0;
    minutes = 0;
    ////Serial.println("seconds while");
		while(seconds > 60){
			minutes++;
			seconds -= 60;	
		}
    ////Serial.println("minutes while");
		while(minutes > 60){
			hours++;
			minutes -= 60;
		}
		    while(hours >= 24){
		      hours -= 24;
		    }
    ////Serial.println("setting");
		returning.second = seconds;
		returning.minute = minutes;
		returning.hour = hours;
    return returning;
	}
	t_object getTime(uint32_t seconds){
		t_object returning;
		int hours, minutes;
		while(seconds > 60){
			minutes++;
			seconds -= 60;	
		}
		while(minutes > 60){
			hours++;
			minutes -= 60;
		}
		returning.second = (int)seconds;
		returning.minute = minutes;
		returning.hour = hours;
	}

}

#endif
