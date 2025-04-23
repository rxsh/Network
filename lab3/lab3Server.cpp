#include <cstdlib>
#include <iostream>
#include <map>
#include <sys/socket.h>
#include <thread>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <iomanip>
#include <arpa/inet.h>

using namespace std;

map<string, int> mapSockets;

void sendClient(int clientFD, const string& message){
    write(clientFD, message.c_str(),message.size());

}

bool notNicknameEmpty(string nickname, int clientFD){
    if(!nickname.empty()){
        sendClient(clientFD,"Error. You have a nickname " + nickname);
        return true;
    }
    return false;
}

bool nicknameEmpty(string nickname, int clientFD){
    if(nickname.empty()){
        sendClient(clientFD,"You need a nickname");
        return true;
    }
    return false;
}


void readThreadSocket(int clientFD){
    char buffer[300];
    int n;
    string nickname = "";

    while(true){
        memset(buffer,0,sizeof(buffer));
        n = read(clientFD,buffer,sizeof(buffer));
        if(n<=0) break;

        buffer[n] = '\0';
        string msg(buffer);

        if(msg == "exit\n" or msg == "exit"){
            if(!nickname.empty()){
                mapSockets.erase(nickname);
                cout << nickname << " disconnected.\n";
            }
            close(clientFD);
            break;
        }

        if(msg[0] == 'N'){ // register a new client.

            /*if(!nickname.empty()){
                string res = "Sorry. You have a nickname " + nickname + '\n';
                sendClient(clientFD, res);
            } 
            */
            if (notNicknameEmpty(nickname, clientFD)) {
                continue;
            }else if(msg[1]== ' ' or msg[1] == 0){
                string res = "Sorry. You type a blank space.\n";
                sendClient(clientFD,res);
            } 
            else{
                nickname = msg.substr(1);

                if(mapSockets.find(nickname) != mapSockets.end()){
                    string res = "Error. The nickname '" + nickname + "' is already taken.\n";
                    sendClient(clientFD, res);
                }else{
                    mapSockets[nickname] = clientFD;
                    string res = "Client " + nickname + " connected.\n";
                    cout << res;
                    sendClient(clientFD,res);
                }
            }

        } else if(msg[0] == 'L'){ // display list of clients
            
            /*if(nickname.empty()){
                string res = "Error. You need a nickname.";
                sendClient(clientFD,res);
            }*/
            if(nicknameEmpty(nickname, clientFD)){
                continue;
            } else{
                string listUsers = "Connected clients: ";
                for(auto it=mapSockets.begin(); it!= mapSockets.end();it++){
                    listUsers = listUsers + it->first + ", ";
                }
                
                listUsers = listUsers.substr(0,listUsers.length() - 2) + '\n';
                sendClient(clientFD, listUsers);
            }


        } else if(msg.rfind('M',0) == 0){// message private
            
            /*if(nickname.empty()){
                string nick = "You need a nickname. Please enter N<name>";
                sendClient(clientFD,nick);
            } */
            if(nicknameEmpty(nickname, clientFD)){
                continue;
            }else{
                // message private format "MName mensaje"
                size_t position = msg.find(' ');
                string targetName;
                string messageContent;
 
                if(position != string::npos){
                    targetName = msg.substr(1,position-1);
                    messageContent = msg.substr(position+1);

                    auto it = mapSockets.find(targetName);
                    if(it != mapSockets.end()){
                        sendClient(it->second, "[" + nickname + "]: " + messageContent);
                    } else{
                        string msgError = "Client " + nickname + " not found.\n";
                        sendClient(clientFD, msgError);
                    }
                }
                else{
                    string msgError = "error command.\n";
                    sendClient(clientFD, msgError);
                }
            }
           
        } else if(msg[0] == 'B'){
            if(nicknameEmpty(nickname, clientFD)){
                continue;
            } else{
                size_t position = msg.find(' ');
                string messageContent = msg.substr(position+1);
                for(auto it : mapSockets){
                    if(it.second != clientFD){
                        string res = "[" + nickname + "]: " + messageContent; 
                        sendClient(it.second,res);
                    } 
                }
            }  
        }
        else if(msg[0] == 'F'){

        } else if(msg[0] == 'Q'){
            if(!nickname.empty()){
                mapSockets.erase(nickname);
                cout << nickname << " disconnected.\n";
            }
            close(clientFD);
            break;

        }
    }
}

int main() {
    int sock, client;
    struct sockaddr_in serverAddr;
    char buffer[300];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "Error al crear el socket\n";
        return EXIT_FAILURE;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(45000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        return EXIT_FAILURE;
    }

    if (listen(sock, 10) < 0) {
        cerr << "Error en listen\n";
        return EXIT_FAILURE;
    }

    cout << "server open...\n";
    
    while (true) {
        client = accept(sock, NULL, NULL);
        if (client < 0) {
            cerr << "error accept\n";
            continue;
        }

        thread(readThreadSocket, client).detach();
    }

    close(sock);
    return 0;
}
