#include <string.h>
#include <iostream>
#include <ctime>
#define CURL_STATICLIB
#include <curl/curl.h>
//clang++ correct_time.cpp -o correct_time -lcurl -std=c++11
//x86_64-w64-mingw32-g++ correct_time.cpp -o correct_t.exe -lcurl
//i686-w64-mingw32-g++ correct_time.cpp -o correct_t.exe -L/Downloads/Cage_Server.v5.1_Server2-CH_edit-TM/curl/curl/lib/curl-7.81.0-win32-mingw/lib/ -I/Downloads/Cage_Server.v5.1_Server2-CH_edit-TM/curl/curl/lib/curl-7.81.0-win32-mingw/include -lcurl

using namespace std;

int main(){
	//string baseURL = "http://192.168.1.141/";
	string baseURL;
	cout<<"base url: ";
	cin >> baseURL;
	
	time_t ttime = time(0);
	tm *local_time = localtime(&ttime);
	if(baseURL.at(baseURL.size()-1) != '/'){baseURL+= '/';}
	baseURL += "correctTime?correctionh="+to_string(local_time->tm_hour)+"&correctionm="+to_string(local_time->tm_min)+"&corrections="+to_string(local_time->tm_sec);

	cout<<"Enacting url: "<<baseURL<<endl;
	CURL *curl;

	curl = curl_easy_init();
	CURLcode response;
	curl_easy_setopt(curl, CURLOPT_URL, baseURL.c_str());
	response = curl_easy_perform(curl);
	if(response != CURLE_OK){
		cout<<"request failed"<<endl;
		return -1;
	}
	curl_global_cleanup();
	cout<<"time changed"<<endl;
}
