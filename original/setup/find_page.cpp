#include <string.h>
#include <iostream>
#include <ctime>
//#define CURL_STATICLIB
#include <curl/curl.h>
//clang++ find_page.cpp -o find_page -lcurl -std=c++11

using namespace std;
bool test(string base, string endpoint){
	CURL *curl;
	string url = base+endpoint+"/";
	curl = curl_easy_init();
	CURLcode response;
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2);
	response = curl_easy_perform(curl);
	if(response != CURLE_OK){
		cout<<"request failed"<<endl;
		return false;
	}
	curl_global_cleanup();
	return true;
}
void find(int lower, int upper){
	string baseURL = "http://192.168.1.";
	for(int i = lower; i <= upper; i++){
		cout<<"trying "<<i<<endl;
		bool result = test(baseURL, to_string(i));
		if(result == true){
			string url = baseURL + to_string(i)+"/";
			cout<<"\nFound! "<< url<<endl;
			break;
		}
	}
}




int main(){
	find(100, 170);	
}
