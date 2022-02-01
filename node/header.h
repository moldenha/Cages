#include <string.h>


class client{
	private:
		char* ssid, password, host;
		char [50] timeBuffer;
		int port;
	public:
		client(const char* _ssid, const char* _password, const char* _host, int _port);
};

client::client(const char* _ssid, const char* _password, const char* _host, int _port)
	:port(_port)
{
	strcpy(ssid, _ssid);
	strcpy(password, _password);
	strcpy(host, _host);	
}
