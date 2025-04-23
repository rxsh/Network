#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <thread>
#include <arpa/inet.h>
#include <cstring>
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

using namespace std;

void receiveMessage(int sock){

	vector<char> buffer(1024);
	while (true){
		int m;
		m = read(sock,buffer.data(),buffer.size());

		if(m < 0){
			cerr << "Disconnected from server.\n";
			break;
		}

		buffer[m] = '\0';
		cout << "Message: " << buffer.data() << endl;
	}

	close(sock);
	exit(0);
}

int main(){

	int sock;
	struct sockaddr_in addr;
	vector<char> buffer(1024);
	int n;
	int Res;

	sock = socket(AF_INET,SOCK_STREAM,0);

	if(sock < 0){
		cerr << "socket error.\n";
		return EXIT_FAILURE;
	}

	cout << "socket created.\n";

	memset(&addr,0,sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(45000);
	Res = inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr);

	if (Res < 0){
		cerr << "error first parameter.\n";
		return EXIT_FAILURE;
	}
	else if(Res==0){
		cerr << "error second parameter.\n";
		return EXIT_FAILURE;
	}

	if(connect(sock,(const struct sockaddr*)&addr,sizeof(addr)) < 0){
		cerr << "error connect.\n";
		return EXIT_FAILURE;
	}

	cout << "connect to the server.\n";

	thread(receiveMessage,sock).detach();

	string message;
	while(true){

		getline(cin,message);
		if(message == "chau"){
			break;
		}

		send(sock,message.c_str(),message.length(),0);
	}

	close(sock);

	/*
	int m;

	do {

		cout << "enter your message: "; cin.getline(buffer.data(),buffer.size());
		m = send(sock,buffer.data(),buffer.size(),0);
		if (strncmp(buffer.data(),"chau",4) == 0){
			cerr << "client disconnected.\n";
			break;
		}

		buffer[m] = '\0';
		m = read(sock,buffer.data(),buffer.size());
		cout << "Server: " << buffer.data() << endl;

	} while(strncmp(buffer.data(),"chau",4) != 0);

	*/

}
