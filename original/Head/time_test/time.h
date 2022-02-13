#ifndef TIME_H
#define TIME_H
#include <TimeLib.h>

#define OPEN 1
#define CLOSE 0
#define UNDEFINED -1

namespace time_handler{

	typedef struct t_object{
		//if open is true (1), then the cages are going to be opened when this time is hit
		//if open is false (0), then the cages are going to be closed with this time is hit;
		//if open is undefined(-1), then the cages are just going to change their current position
		int second, minute, hour, open;
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
		/*t_object operator + (int seconds_to_add){
			t_object returning;
			returning.hour = hour;
			returning.minute = minute;
			returning.second = second;
			returning.open = open;
		}*/
	} t_object;

	t_object getNow(){
		time_t t = now();
		t_object returning;
		returning.hour = hour(t);
		returning.minute = minute(t);
		returning.second = second(t);
		returning.open = -1;
		return returning;
	}
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
