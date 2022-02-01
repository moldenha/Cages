#include <string.h>
#include <iostream>
//#define CURL_STATICLIB
#include <ctime>
#include <stdlib.h>
//g++ free_test.cpp -o free_test

using namespace std;

void clear(char** input, int size){
	for(int i = 0; i < size; i++){
		free(input[i]);
	}
	input = (char**)realloc(input, 1);
}

int main(){
	char **inputs;
	inputs = (char**)malloc(sizeof(char*)*10);
	cout<<"set"<<endl;
	for(int i = 0; i < 10; i++){
		inputs[i] = (char*)malloc(sizeof(char)*6);
		cout<<"set2"<<endl;
		for(int q = 0; q < 5; q++){
			cout<<"inputting"<<endl;
			inputs[i][q] = 'h';
		}
	}
	cout<<"clearing"<<endl;
	clear(inputs, 1);
	cout<<"cleared"<<endl;
	inputs = (char**)realloc(inputs, sizeof(char*)*10);
	cout<<"realloced"<<endl;
	for(int i = 0; i < 1; i++){
		inputs[i] = (char*)malloc(sizeof(char)*6);
		for(int q = 0; q < 5; q++){
			inputs[i][q] = 'h';
		}
	}
	cout<<"set"<<endl;
	return 0;
}
