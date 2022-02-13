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



template <typename T>
struct my_vector{
	T *arr;
	size_t size = 0;
	bool freeable = false;
	
	void set_arr(T* new_arr, size_t newSize){
	//	std::cout<<"set_arr called "<<size<<" "<<newSize<<std::endl;
		clear();
		if(newSize != 0){
			//std::cout<<"not 0"<<std::endl;
			arr = (T*)malloc(sizeof(T)*newSize);
			for(int i = 0; i < newSize; i++){
				arr[i] = new_arr[i];
			}
			size = newSize;
			return;
		}
		arr = (T*)malloc(sizeof(T));
		size = 0;
		//std::cout<<"allocated"<<std::endl;
	}
	~my_vector(){size = 0; freeable=false;}
	my_vector():size(0),freeable(false){}
	size_t Size(){return size;}
	void push_back(T input)
	{
		if(Size() == 0 || size == 0){
			arr = (T*)malloc(sizeof(T));
			arr[0] = input;
			size++;
			freeable = true;
			return;	
		}
		size++;
		arr = (T*)realloc(arr, sizeof(T)*size);
		//if(detect_nstring::detect<T> == true){arr[size-1];}
		arr[size-1] = input;
	}
	void push_back(const T* input, size_t n_size){
		for(int i = 0; i < n_size; i++){
			push_back(input[i]);
		}
	}
	void erase(size_t index){
		size--;
		T* new_arr = (T*)malloc(sizeof(T)*size);
		int counter = 0;
		for(int i = 0; i <= size; i++){
			if(i != index){
				new_arr[counter] = arr[i];
				counter++;
			}
		}
		arr = (T*)malloc(sizeof(T)*size);
		for(int i = 0; i < size; i++){
			arr[i] = new_arr[i];
		}
		free(new_arr);
	}
	void pop_back(){erase(size-1);}
	T &at(size_t index){
		//if(index >= size){throw("out of bounds error");}
		return arr[index];	
	}
	bool in(T input){
		if(detect_chararray::detect<T> == true){
			for(int i = 0; i < size; i++){
				if(strcmp(input, arr[i]) == 0)
					return true;
			}
			return false;
		}
		for(int i = 0; i < size; i++){
			if(input == arr[i])
				return true;
		}
		return false;	
	}
	int index(T input){
		if(!in(input))
			return -1;
		if(detect_chararray::detect<T> == true){
			for(int i = 0; i < size; i++){
				if(strcmp(input, arr[i]) == 0)
					return i;
			}
			return -1;
		}
		for(int i = 0; i < size; i++){
			if(input == arr[i])
				return i;
		}
		return -1;
	}
	void erase(T input){
		if(!in(input)){return;}
		int index = index(input);
		size--;
		T* new_arr = (T*)malloc(sizeof(T)*size);
		int counter = 0;
		for(int i = 0; i <= size; i++){
			if(i != index){
				new_arr[counter] = arr[i];
				counter++;	
			}
		}
		set_arr(new_arr, size);
	}
	void erase_all(T input){
		while(in(input)){erase(input);}
	}
	void free_self(){if(freeable){free(arr);}}
	const T* n_my_vector(){return arr;}
	T &operator[](size_t index){return at(index);}
	void clear(){
		size = 0;
		arr = (T*)malloc(sizeof(T));
	}
};

struct n_string{
	private:
		my_vector<char> line;
	public:
		static const size_t npos = -1;
		~n_string(){line.size=0;clear();}
		n_string(){line.size=0;clear();}
		n_string(n_string &obj){
			clear();
			line.size=0;
			const char* input = obj.c_str();
			line.push_back(input, strlen(input));
		}
		n_string(const char* input){clear();line.size=0;line.push_back(input, strlen(input));}
		n_string(const char* input, int length){clear();line.size=0;line.clear();line.push_back(input,length);}
		n_string(char input){clear();line.size=0;line.push_back(input);}
		char &at(size_t index){return line.at(index);}
		const char* c_str(){return line.n_my_vector();}
		void push_back(const char* input){
			line.push_back(input, strlen(input));
		}
		n_string operator+(const char* input){
			n_string cpy(*this);
			cpy.push_back(input);
			return cpy;
		}
		void operator+=(n_string &input){line.push_back(input.c_str(), strlen(input.c_str()));}
		n_string operator+(n_string &input){
			n_string cpy(*this);
			cpy.push_back(input.c_str());
			return cpy;
		}
		void operator+=(const char* input){line.push_back(input, strlen(input));}
		void operator+=(char input){line.push_back(input);}
		void operator+=(n_string obj){line.push_back(obj.c_str(), strlen(obj.c_str()));}
		bool operator==(n_string &obj){return strcmp(c_str(), obj.c_str()) == 0;}
		bool operator==(const char* obj){return strcmp(c_str(), obj) == 0;}
		void operator=(const char* obj){line.set_arr(strdup(obj), strlen(obj));}
		void operator=(char* obj){line.set_arr(obj, strlen(obj));}
		void operator=(n_string obj){line.set_arr(strdup(obj.c_str()), strlen(obj.c_str()));}
		//void operator=(const n_string obj){line.set_arr(strdup(obj.c_str()), strlen(obj.c_str()));}
		char &operator[](size_t index){return at(index);}
		void clear(){line.size=0;*this = "";}
		void zero(){line.size=0;}
		size_t size(){return line.Size();}
		void free_self(){line.free_self();}
		my_vector<n_string> split(char input){
			my_vector<n_string> returning;
			n_string current;
			for(int i = 0; i < line.Size(); i++){
				if(current.size() != 0 && line.at(i) == input){
					returning.push_back(current);
					current = "";
				}
				else{
					current += line.at(i);
				}
			}
			if(current.size() > 0)
				returning.push_back(current);
			current.free_self();
			return returning;
		}
		my_vector<n_string> split(n_string input){
			my_vector<n_string> returning;
			n_string current;
			n_string copy(*this);
			size_t found = copy.find(input);
			size_t current_pos = 0;
			while(found != npos){
				for(;current_pos < found; current_pos++){
					current += copy.at(current_pos);
				}
				current_pos += input.size();
				returning.push_back(current);
				current.clear();
				copy.erase(0, current_pos);
				found = copy.find(input);
				current_pos = 0;
			}
			while(current_pos < copy.size()){
				current += copy.at(current_pos);
				current_pos++;
			}
			if(current.size() > 0){returning.push_back(current);}
			current.free_self();
			copy.free_self();
			return returning;
		}
		my_vector<n_string> split(const char* input){
			n_string in(input);
			std::cout<<in.c_str()<<std::endl;
			my_vector<n_string> returning = split(in);
			in.free_self();
			return returning;		
		}
		size_t find(n_string substr){
			for(int i = 0; i < size(); i++){
				n_string so_far;
				if((substr.size() + i) >= size())
					break;
				for(int j = 0; j < substr.size(); j++){
					so_far += at(j+i);
				}
				if(so_far == substr){
					so_far.free_self();
					return i;
				}
				//std::cout<<so_far.c_str()<<" "<<substr.c_str()<<std::endl;
				so_far.free_self();
			}
			return npos;
		}
		size_t find(const char* substr){
			n_string sub(substr);
			size_t returning = find(sub);
			sub.free_self();
			return returning;	
		}
		void erase(size_t start, size_t end){
			for(int i = start; i <= end; i++){
				line.erase(start);
			}
		}
};

template<typename T1, typename T2>
struct pairs{
	T1& first;
	T2& second;
	pairs(T1 _first, T2 _second)
		:first(_first),
		second(_second){}
	~pairs(){}
	pairs<T1, T2> &operator=(const pairs<T1, T2> &obj){first = obj.first; second = obj.second;}
};

std::ostream& operator<< (std::ostream& outs, n_string* obj){
	return outs<<obj->c_str();
}
std::ostream& operator<< (std::ostream& outs, n_string& obj){
	return outs<<obj.c_str();	
}

void free_my_vector_str(my_vector<n_string> in){
	for(int i = 0; i < in.Size(); i++){
		in.at(i).free_self();
	}
	in.free_self();
}
namespace standard_string{
n_string to_n_string(int number){
	int n = log10(number) + 1;
    	int i;
    	char *numberArray = (char*)calloc(n, sizeof(char));
   	for (i = n-1; i >= 0; --i, number /= 10)
    	{
        	numberArray[i] = (number % 10) + '0';
    	}
    	n_string num(numberArray);	
	free(numberArray);
	return num;
}
}

#endif
