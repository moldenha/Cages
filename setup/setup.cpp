#include <string.h>
#include <iostream>
#include <ctime>
//#define CURL_STATICLIB
#include <curl/curl.h>
#include <ctime>
//clang++ setup.cpp -o setup -lcurl -std=c++11
//i686-w64-mingw32-g++ setup.cpp -o setup.exe -L/Downloads/Cage_Server.v5.1_Server2-CH_edit-TM/curl/curl/lib/curl-7.81.0-win32-mingw/lib/ -I/Downloads/Cage_Server.v5.1_Server2-CH_edit-TM/curl/curl/lib/curl-7.81.0-win32-mingw/include -lcurl


size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp){
	return size * nmemb;
}

using namespace std;
bool test(string base, string endpoint, int timeout){
	CURL *curl;
	string url = base+endpoint+"/";
	curl = curl_easy_init();
	CURLcode response;
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	if(timeout != 0){curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);}
	//curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	if(timeout != 0){curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);}
	response = curl_easy_perform(curl);
	if(response != CURLE_OK){
		cout<<"request failed"<<endl;
		return false;
	}
	curl_global_cleanup();
	return true;
}
string find(int lower, int upper){
	string baseURL = "http://192.168.1.";
	for(int i = lower; i <= upper; i++){
		cout<<"trying "<<i<<endl;
		bool result = test(baseURL, to_string(i), 2);
		if(result == true){
			string url = baseURL + to_string(i)+"/";
			url += to_string(i);
			url += "/";
			cout<<"\nFound! "<< url<<endl;
			return url;
		}
	}
	return baseURL;
}




int main(){
	string url = find(100, 170).c_str();
	url += "/";
	cout<<"found url: "<<url<<endl;
	const char* url_copy = url.c_str();
	time_t ttime = time(0);
	tm *local_time = localtime(&ttime);
	string endpoint = "correctTime?correctionh="+to_string(local_time->tm_hour)+"&correctionm="+to_string(local_time->tm_min)+"&corrections="+to_string(local_time->tm_sec);
	bool result = test(string(url_copy), endpoint, 2);
	cout<<"corrected time "<<endl;
	result = test(string(url_copy), "findServos", 0);
	cout<<"servos found"<<endl;
	return 0;
}
