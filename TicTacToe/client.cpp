#include <cstdlib>
#include <exception>
#include <netinet/in.h>
#include <sstream>
#include <stdlib.h>
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <map>
#include <thread>
#include <cstring>
#include <fstream>
#include <iomanip>

using namespace std;

void fileTransferProtocol(const string& fileData){

	istringstream iss(fileData);
	string line;
	string filename, hash_pre, hash_post;
	string fileContent;

	bool readingContent = false;
	while(getline(iss,line)){
		if(line.rfind("FILE ",0) == 0){
			filename = line.substr(5);
		} else if(line.rfind("HASH_PRE: ",0) == 0){
			hash_pre = line.substr(10);
		} else if(line.rfind("CONTENT:",0) == 0){
			readingContent = true;
		} else if(line.rfind("HASH_POST: ",0) == 0){
			hash_post = line.substr(11);
			readingContent = false;
		} else if(readingContent){

			fileContent += line + "\n";

		}
	}

	if(!fileContent.empty() && fileContent.back() == '\n'){
		fileContent.pop_back();
	}

	// Calcular el hash local
    //unsigned int hashCalc = 0;
    //for (char c : fileContent) {
    //    hashCalc = (hashCalc * 101 + static_cast<unsigned int>(c)) % 1000000007;
    //}
    //stringstream ss;
    //ss << hex << hashCalc;
    //string computedHash = ss.str();
    
    cout << "\n---- Transfer files ----\n";
    cout << "File: " << filename << "\n";
    cout << "(HASH_PRE): " << hash_pre << "\n";
	//cout << "Content: " << fileContent;
    cout << "(HASH_POST): " << hash_post << "\n";
    //cout << "Locally computed hash: " << computedHash << "\n";
    if(!hash_post.empty() && hash_post == hash_pre){
        ofstream outFile("download_" + filename, ios::binary);
        if(outFile){
            outFile << fileContent;
            outFile.close();
            cout << "File saved as: download_" << filename << "\n";
        } else {
            cout << "Error: The file could not be saved locally.\n";
        }
    } else {
        cout << "Error: The hashes dont match. The file may have been corrupted.\n";
    }
    cout << "----------------------------------\n> ";
    cout.flush();
}


void readThreadSocket(int clientFD){

	char buffer[1024];
	int n;
	bool fileTransferActive = false;
	string readText;
	bool expectingSpectators = false;

	while(true){
		memset(buffer,0,sizeof(buffer));
		n = read(clientFD,buffer,sizeof(buffer)-1);
		if(n<0) break;

		buffer[n] = '\0';
		string inp(buffer,n);

		if(fileTransferActive){
			readText += inp;
			if(readText.find("HASH_POST:") != string::npos){
				fileTransferProtocol(readText);
				fileTransferActive = false;
				readText = "";
			}
		} else{
			if(inp.find("FILE ") == 0){
				fileTransferActive = true;
				readText += inp;
				
				if(readText.find("HASH_POST:") != string::npos){
					fileTransferProtocol(readText);
					fileTransferActive = false;
					readText = "";
				}
			} else if(expectingSpectators){
				// no hacemos nada con la respuesta, ya que el servidor maneja esto
				expectingSpectators = false;

			} else if(expectingSpectators){

				expectingSpectators = true;
				cout << inp;
				cout.flush();

			} else{
				// normal message
				cout << inp;
				cout << "\n> ";
				cout.flush();	
			}
		}

	}

	close(clientFD);
}

int main(){

	int sock;
	struct sockaddr_in sv_addr;
	char buffer[300];

	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0){
		cerr << "error socket.\n";
		return EXIT_FAILURE;
	}

	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(45000);
	inet_pton(AF_INET,"127.0.0.1",&sv_addr.sin_addr);

	if(connect(sock,(const struct sockaddr*)&sv_addr,sizeof(sv_addr)) < 0){
		cerr << "error connect to server.\n";
		return EXIT_FAILURE;
	}

	cout << "Coonect to the server. Enter the next commands" << endl;
	cout << "- N<name> to register (ej.NCesar)" << endl;
	cout << "- L to display connected users" << endl;
	cout << "- M<name> <message> to send private message" << endl;
	cout << "- B to send public message" << endl;
	cout << "- F <destinary> <filename> to receive file" << endl;
	cout << "- J to join TicTacToe game" << endl;
	cout << "- Q to quit" << endl;

	thread(readThreadSocket,sock).detach();
	while(true){
		string msg;
		getline(cin, msg);

		write(sock, msg.c_str(), msg.size());
		if(msg[0] == 'Q') break; 
	}

	close(sock);
	cout << "left from the server.\n";
	return 0;
}
