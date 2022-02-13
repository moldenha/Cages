#ifndef ARRAY_H
#define ARRAY_H
#include <stdlib.h>
#include <string.h>

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
namespace detect_charArray {
  template<class T, class...Ts>
  constexpr bool is_charlike(tag_t<T>, Ts&&...){ return false; }
  constexpr bool is_charlike( tag_t<char*> ){ return true; }
  constexpr bool is_charlike( tag_t<const char*> ){ return true; }
  // ETC
  template<class T>
  constexpr bool detect=is_charlike(tag<T>); // enable ADL extension
}


template <typename T>
struct array{
	private:
		T *arr;
		size_t size;
		bool freeable;
		void set_arr(T* new_arr, size_t newSize){
			size = newSize;
			if(size == 0){return;}	
			arr = (T*)malloc(sizeof(T)*newSize);
			for(int i = 0; i < size; i++){
				arr = new_arr[i];
			}
		}
	public:
		array():size(0),freeable(false){}
		size_t Size(){return size;}
		void push_back(T input)
		{
			if(size == 0){
				arr = (T*)malloc(sizeof(T));
				arr[0] = input;
				size++;	
			}
			else{
				size++;
				arr = (T*)realloc(arr, sizeof(T)*size);
				arr[size-1] = input;
			}
			freeable = true;
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
			set_arr(new_arr, size);
			free(new_arr);
		}
		void pop_back(){erase(size-1);}
		T &at(size_t index){
			if(index >= size){return NULL;}
			return &arr[index];	
		}
		bool in(T input){
			if(detect_charArray::detect<T> == true){
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
			if(detect_charArray::detect<T> == true){
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
};

#endif
