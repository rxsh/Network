#include <algorithm>
#include <asm-generic/socket.h>
#include <unistd.h>
#include <mutex>
#include <cstdio>
#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

//vector<int> clients; // stores connected clients
vector<int> clients;
mutex clients_mutex;



void handle_client(int client_socket){
	vector<char> buffer(1024);
	while(true){
		memset(buffer.data(),0,buffer.size());
		int recv = read(client_socket,buffer.data(),buffer.size()-1);

		if(recv <= 0){
			cout << "Client disconnected.\n";
			break;
		}

		buffer[recv] = '\0';
		//cout << "Client: " << buffer.data() << endl;
	
		if(strncmp(buffer.data(),"chau",4) == 0){
			cout << "client said 'chau' disconnecting...\n";
			break;

		}
	
		{
		// broadcast the message to all other clients
			lock_guard<mutex> lock(clients_mutex);
			for(int sock : clients){
				if(sock != client_socket){
					send(sock,buffer.data(),recv,0);
				}
			}
		}
	}

	{
		// remove client from the list
		lock_guard<mutex> lock(clients_mutex);
		clients.erase(remove(clients.begin(),clients.end(),client_socket), clients.end());
	}

	close(client_socket);

}

int main(){

	int server, client;
	struct sockaddr_in sv_addr, cl_addr;
	socklen_t address_size = sizeof(cl_addr);
	vector<char> buffer(1024);
	int n;

	server = socket(AF_INET,SOCK_STREAM,0);

	if(server < 0){
		cerr << "Server socket error.\n";
		return EXIT_FAILURE;
	}

	cout << "socket created.\n";

	int optval = 1;
	if(setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0){
		cerr << "setsockopt error.\n";
		return EXIT_FAILURE;
	}

	memset(&sv_addr,0,sizeof(sv_addr));

	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(45000);
	sv_addr.sin_addr.s_addr = INADDR_ANY;

	n = bind(server,(const struct sockaddr*)&sv_addr, sizeof(sv_addr));

	if (n<0){
		cerr << "bind error.\n";
		return EXIT_FAILURE;
	}

	if(listen(server,5) < 0){
		cerr << "listen error.\n";
		return EXIT_FAILURE;
	}

	cout << "Listening port 45000...\n";

	while(true){
		client = accept(server, (struct sockaddr*)&cl_addr, &address_size);
		if(client < 0){
			cerr << "connection error.\n";
			continue;
		}

		cout << "new client connected.\n";

		{
			lock_guard<mutex> lock(clients_mutex);
			clients.push_back(client);
		}

		thread(handle_client,client).detach();
	}

/*

	int m;

	do {
		
		m = read(client, buffer.data(), buffer.size());
		if( m<=0){
			cout << "Client Disconnected.\n";
			break;
		}
		
		buffer[m] = '\0';
		cout << "Client: " << buffer.data() << endl;

		cout << "enter your message: "; cin.getline(buffer.data(),buffer.size());

		
		send(client,buffer.data(),buffer.size(),0);


	} while(strncmp(buffer.data(), "chau", 4) != 0);

*/
}
