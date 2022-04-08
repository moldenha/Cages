#ifndef BASICS_H
#define BASICS_H
//#include "n_string.h"
#include <stdlib.h>
#include <string.h>
#include <iostream>


//arduino by default enables c++11, therefore constexpr and other types allocated in c++11 are allowed to be used for this project
template <typename T>
struct my_vector;

typedef struct n_string n_string;

template<class T>struct tag_t{};
template<class T>constexpr tag_t<T> tag{};
namespace detect_character {
  template<class T, class...Ts>
  constexpr bool is_charlike(tag_t<T>, Ts&&...){ return false; }
  constexpr bool is_charlike( tag_t<char> ){ return true; }
  constexpr bool is_charlike( tag_t<wchar_t> ){ return true; }
  // ETC
  template<class T>
  constexpr bool detect=is_charlike(tag<T>); // enable ADL extension
}
namespace detect_chararray {
  template<class T, class...Ts>
  constexpr bool is_charlike(tag_t<T>, Ts&&...){ return false; }
  constexpr bool is_charlike( tag_t<char*> ){ return true; }
  constexpr bool is_charlike( tag_t<const char*> ){ return true; }
  //constexpr bool is_charlike( tag_t<n_string> ){ return true; }
  // ETC
  template<class T>
  constexpr bool detect=is_charlike(tag<T>); // enable ADL extension
}
namespace detect_nstring{
	template<class T, class...Ts>
	constexpr bool is_charlike(tag_t<T>, Ts&&...){return false;}
	constexpr bool is_charlike(tag_t<n_string>){return true;}

	template<class T>
	constexpr bool detect=is_charlike(tag<T>);
}


//regular pointer version

template<typename T>
struct my_vector{
	private:
		T* arr;
		size_t Size;
		size_t maxlen;
		void free_self(){
			if(arr != NULL){
				delete[] arr;
			}
		}
	public:
		static const size_t npos = -1;
		~my_vector(){free_self();}
		my_vector()
			:arr(new T [1]),
			Size(0),
			maxlen(1)
		{}
		my_vector(const my_vector<T> &obj)
			:arr(new T [obj.maxlen]),
			Size(obj.Size),
			maxlen(obj.maxlen)
		{
			std::copy(obj.arr, obj.arr + obj.maxlen, arr);
		}
		my_vector(const my_vector<T>&& o)
		       :arr(o.arr),
	       		Size(o.Size),
	 		maxlen(o.maxlen)		
		{}
		void erase(size_t index){
			if(index >= Size)
				return;
			Size--;
			maxlen = Size+1;
			auto newdata = new T [maxlen];
			int counter = 0;
			for(int i = 0; i < maxlen; i++){
				if(i != index){
					newdata[counter] = arr[i];
					counter++;
				}
			}
			if(arr){
				delete[] arr;
			}
			arr = newdata;
		}
		void pop_back(){erase(Size-1);}
		void clear(){do{pop_back();}while(Size != 0);}
		size_t size(){return Size;}
		T& at(size_t index){return arr[index];}
		T& operator[](size_t index){return at(index);}
		void push_back(const T& input){
			if(Size >= maxlen){
				maxlen *= 2;
				auto newdata = new T [maxlen];
				std::copy(arr, arr + Size, newdata);
				if(arr){
					delete[] arr;
				}
				arr = newdata;
			}
			arr[Size++] = input;	
		}
		void copy(my_vector<T> input){
			clear();
			for(int i = 0; i < input.size(); i++){
				push_back(input.at(i));
			}	
		}
		size_t where(T input){
			for(int i = 0; i < size(); i++){
				if(at(i) == input)
					return i;
			}
			return npos;
		}
		bool in(T input){return (where(input) != npos);}
		void erase(T input){erase(where(input));}
		void erase_all(T input){
			size_t position = where(input);
			do{
				erase(position);
				position = where(input);
			}while(position != npos);
		}
};


struct n_string{
	private:
		char* main;
		size_t Size;
		void reallocate(size_t new_size){
			if(new_size == 0){
				Size = 0;
				main = (char*)realloc(main, sizeof(char));
				main[0] = '\0';
				return;
			}
			Size = new_size;
			main = (char*)realloc(main, sizeof(char)*(Size+1));
			main[Size] = '\0';
		}
		void free_self(){
			if(main != NULL){
				delete main;
			}
		}
	public:
		static const size_t npos = -1;
		//constructors:
		//(None)
		//(const char* input)
		//(const n_string &)
		n_string()
			:main(new char),
			Size(0)
		{clear();}
		~n_string(){free_self();}
		n_string(const char* input)
			:main(new char),
			Size(0)
		{
			reallocate(strlen(input));
			strcpy(main, input);	
		}
		n_string(const n_string &obj)
			:main(new char[obj.Size]),
			Size(obj.Size)
		{
			strcpy(main, obj.main);
		}
		size_t size(){return Size;}
		const char* c_str(){return main;}
		void clear(){reallocate(0);}
		void copy(const char* input){
			//std::cout<<"reallocating to size "<<strlen(input)<<std::endl;
			reallocate(strlen(input));strcpy(main, input);
		}
		char& at(size_t index){return main[index];}
		n_string substr(size_t start, size_t end){
			n_string returning;
			if(end > size()){end = size();}
			for(int i = start; i < end; i++){
				returning += at(i);
			}
			return returning;
		}
		void push_back(char input){reallocate(size()+1);main[size()-1] = input;}
		void push_back(const char* input){for(int i = 0; i < strlen(input); i++){push_back(input[i]);}}
		void push_back(n_string input){push_back(input.c_str());}
		void operator = (const char* input){copy(input);}
		void operator = (n_string input){copy(input.c_str());}
		void operator = (char input){reallocate(1);main[0] = input;}
		char& operator[](size_t index){return at(index);}
		bool operator== (const char* input){return(strcmp(main, input) == 0);}
		bool operator== (n_string input){return *this == input.c_str();}
		void operator+= (const char* input){push_back(input);}
		void operator+= (char input){push_back(input);}
		void operator+= (n_string input){push_back(input);}

		size_t find(char input){
			for(int i = 0; i < size(); i++){
				if(at(i) == input){
					return i;
				}
			}
			return npos;
		}
		size_t find(const char* input){
			for(int i = 0; i < size(); i++){
				if(at(i) == input[0]){
					n_string sub = substr(i, (i+strlen(input)));
					if(sub == input)
						return i;	
				}
			}
			return npos;
		}
		size_t find(n_string input){return find(input.c_str());}
		bool in(n_string input){return (find(input) != npos);}
		bool in(const char* input){return (find(input) != npos);}
		bool in(char input){return (find(input) != npos);}
		void erase(size_t index){
			n_string set;
			for(int i = 0; i < size(); i++){
				if(i != index)
					set += at(i);
			}
			copy(set.c_str());
		}
		void erase(size_t start, size_t end){
			for(int i = start; i < end; i++){
				erase(start);
			}	
		}
		my_vector<n_string> split(const char* input){
			my_vector<n_string> returning;
			n_string c(c_str());
			n_string current;
			size_t position = c.find(input);
			while(position != npos){
				current = c.substr(0, position);
				returning.push_back(current);
				c.erase(0, (position+strlen(input)));
				position = c.find(input);
			}
			if(c.size() > 0){
				returning.push_back(c);
			}
			return returning;	
		}
		my_vector<n_string> split(n_string input){
			return split(input.c_str());	
		}
		my_vector<n_string> split(char input){
			char* sp = new char[1];
			sp[0] = input;
			my_vector<n_string> returning = split(sp);
			delete[] sp;
			return returning;
		}
};	

template<typename T1, typename T2>
struct pairs{
	T1 first;
	T2 second;	
};

namespace standard_string{
	int decimal_places(float input){
		int num_of_decimals = 0;
		float input_copy = input;
		while(input_copy != (int)input_copy){
			num_of_decimals++;
			input_copy *= 10;
		}
		return num_of_decimals;
	}
	int count_digit(float number) {
		int num = (int)number;
		if(num == 0)
			return 1;
		return int(log10(num) + 1);
	}
	n_string to_n_string(float number){
		n_string num;
		if(number == 0){
			n_string zero("0");
			return zero;	
		}
		if(number < 0){
			num += "-";
			number *= -1;
		}
		int dec_places = decimal_places(number);
		int places = count_digit(number);
		char buffer[dec_places+places+1];
		int ret = snprintf(buffer, sizeof buffer, "%f", number);
		if (ret < 0) {
			n_string failure("FAILED");
			return failure;	
		}
		num += buffer;
		return num;	
	}
	n_string to_n_string(int number){
		bool abs = false;
		if(number == 0)
			return n_string("0");
		n_string returned = to_n_string((float)number);
		my_vector<n_string> sp = returned.split('.');
		return sp.at(0);
	}
}

#endif
