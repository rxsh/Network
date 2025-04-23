#include <cstdlib>
#include <mutex>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include <map>
#include <thread>
#include <iomanip>
#include <sstream>
#include <vector>

using namespace std;

mutex clients_mutex;
map<string, int> clients;

struct TicTacToe{

    char board[9] = {' ',' ',' ',' ',' ',' ',' ',' ',' '};
    string playerX;
    string playerO;
    vector<int> spectators;
    bool waiting = true;
    char currentTurn = 'X';

};

map<int, TicTacToe> activeGames;
mutex game_mutex;

string printBoard(const TicTacToe& game){

    stringstream ss;
    ss << "\n";
    ss << " " << game.board[0] << " | " << game.board[1] << " | " << game.board[2] << endl;
    ss << " " << game.board[3] << " | " << game.board[4] << " | " << game.board[5] << endl;
    ss << " " << game.board[6] << " | " << game.board[7] << " | " << game.board[8] << endl;

    if(!game.waiting){
        ss << "\nActual turn: " << game.currentTurn << "\n";
    }

    return ss.str();

}

char checkWinner(const TicTacToe& game){
    const int win[8][3] = {
        {0,1,2},{3,4,5},{6,7,8},
        {0,3,6},{1,4,7},{2,5,8},
        {0,4,8},{2,4,6}
    };

    for(auto& p : win){
        if(game.board[p[0]] != ' ' && game.board[p[0]] == game.board[p[1]] &&
        game.board[p[0]] == game.board[p[2]]){
            return game.board[p[0]];
        }
    }
}

void sendClient(int clientFD, const string& message){
    write(clientFD, message.c_str(), message.size());
}

bool existNickname(string nickname, int clientFD){
    if(!nickname.empty()){
        sendClient(clientFD, "Your nickname is: " + nickname + "\n");
        return true;
    }
    return false;
}

bool nicknameEmpty(string nickname, int clientFD){
    if(nickname.empty()){
        sendClient(clientFD, "You dont have nickname.\n");
        return true;
    }
    return false;
}

// Función simple para generar un hash a partir de una cadena (NO CRYPTOGRAPHY)
string simpleHash(const string& data){
    unsigned int hash = 0;
    for (char c : data) {
        // Multiplicamos y sumamos cada caracter; se usa un módulo para evitar números muy grandes
        hash = (hash * 101 + static_cast<unsigned int>(c)) % 1000000007;
    }
    stringstream ss;
    ss << hex << hash;
    return ss.str();
}

void readThreadSocket(int clientFD){
    
    char buffer[300];
    string nickname = "";
    int n;

    while(true){
        n = read(clientFD, buffer, sizeof(buffer)-1);

        if(n <= 0) break;
        
        buffer[n] = '\0';
        string msg(buffer, n);
        
        if(msg[0] == 'N'){ // registrar usuario
            
            if(existNickname(nickname, clientFD)){
                continue;
            } else if(msg[1] == ' ' or msg[1] == 0){
                string res = "Error. You type a blank space\n";
                sendClient(clientFD, res);
            } else{
                nickname = msg.substr(1);
                if(clients.find(nickname) != clients.end()){
                    string res = "Error. Nickname '" + nickname + "' already in use.\n";
                    sendClient(clientFD, res);
                } else{
                    {
                        lock_guard<mutex> lock(clients_mutex);
                        clients[nickname] = clientFD;
                    }
                    cout << "Client " << nickname << " connected.\n";
                    string r = nickname + " join the server.\n";
                    sendClient(clientFD, r);
                }
            }

        } else if(msg[0] == 'L'){ // lista clientes

            if(nicknameEmpty(nickname, clientFD)){
                continue;
            } else{
                string r = "Clients connected:\n";
                for(auto it = clients.begin(); it != clients.end(); it++){
                    r += it->first + "\n";
                }
                sendClient(clientFD, r);
            }

        } else if(msg.rfind("M", 0) == 0){ // mensaje privado
            
            if(nicknameEmpty(nickname, clientFD)){
                continue;
            } else{
                size_t position = msg.find(' ');
                if(position != string::npos){
                    string targetName = msg.substr(1, position - 1);
                    string messageContent = msg.substr(position + 1);
                    auto it = clients.find(targetName);
                    if(it != clients.end()){
                        sendClient(it->second, "[" + nickname + "]: " + messageContent);
                    } else{
                        string msgError = "Client " + targetName + " not found.\n";
                        sendClient(clientFD, msgError);
                    }
                } else{
                    string msgError = "Error in the command.\n";
                    sendClient(clientFD, msgError);
                }
            
            }
        
        } else if(msg[0] == 'Q'){ // salir
        
            if(!nickname.empty()){
                lock_guard<mutex> lock(clients_mutex);
                clients.erase(nickname);
            }
            cout << nickname << " disconnected.\n";
            sendClient(clientFD, "Bye from the server.\n");

            string disconnectMsg = nickname + " left from the server.\n";
            {
                lock_guard<mutex> lock(clients_mutex);
                for(auto it : clients){
                    sendClient(it.second, disconnectMsg);
                }
            }
            shutdown(clientFD, SHUT_RDWR);
            close(clientFD);
            break;

        } else if(msg[0] == 'F'){ // transferencia de archivos entre clientes
            
            if(nicknameEmpty(nickname, clientFD)){
                continue;
            }
            
            size_t first_space = msg.find(' ', 2);
            if(first_space == string::npos){
                sendClient(clientFD, "Error: incorrect format. Use: F <destinatary> <filename>\n");
                continue;
            }
            
            string destiny = msg.substr(2, first_space - 2);
            string filename = msg.substr(first_space + 1);
            
            if(filename.empty()){
                sendClient(clientFD, "Error: no especify filename.\n");
                continue;
            }
            
            ifstream file(filename, ios::binary);
            if(!file){
                sendClient(clientFD, "Error: file not found.\n");
                continue;
            }

            string fileContent((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
            file.close();
            
            string hash = simpleHash(fileContent);
            
            cout << "Transfer file of " << nickname << " to " << destiny 
                 << ": " << filename << " (hash: " << hash << ")\n";
            
            bool recipientFound = false;

            for(auto& client : clients){
                if(client.first == destiny){  
                    stringstream ss;
                    ss << "FILE " << filename << "\n";
                    ss << "FROM: " << nickname << "\n";
                    ss << "HASH_PRE: " << hash << "\n";
                    ss << "CONTENT:\n";
                    ss << fileContent << "\n";
                    ss << "HASH_POST: " << hash << "\n";
                    
                    sendClient(client.second, ss.str()); 
                    recipientFound = true;
                    break;
                }
            }
            
            if(!recipientFound){
                sendClient(clientFD, "Error: Usuario destinatario no encontrado.\n");
            } else {
                sendClient(clientFD, "File sent to " + destiny + "\n");
            }
        }else if(msg[0] == 'B'){ // broadcast

            if(nicknameEmpty(nickname, clientFD)){
                continue;
            } else{
                size_t position = msg.find(' ');
                if(position == string::npos){
                    sendClient(clientFD, "Error: format broadcast error.\n");
                    continue;
                }
                string messageContent = msg.substr(position + 1);
                // Envía a todos, excepto al emisor
                lock_guard<mutex> lock(clients_mutex);
                for(auto it : clients){
                    if(it.second != clientFD){
                        string res = "[" + nickname + "]: " + messageContent;
                        sendClient(it.second, res);
                    }
                }
            }
        }
    }
}

int main(){
    int serverFD, clientFD;
    struct sockaddr_in server_address;
    
    serverFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(serverFD < 0){
        cerr << "Error to create socket.\n";
        return EXIT_FAILURE;
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(45000);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if(bind(serverFD, (const struct sockaddr*)&server_address, sizeof(server_address)) < 0){
        cerr << "Error bind.\n";
        return EXIT_FAILURE;
    }

    if(listen(serverFD, 10) < 0){
        cerr << "Error listen.\n";
        return EXIT_FAILURE;
    }

    for(;;){
        clientFD = accept(serverFD, NULL, NULL);
        if(clientFD < 0){
            cout << "Error connection.\n";
            continue;
        }
        thread(readThreadSocket, clientFD).detach();
    }
    
    close(serverFD);
    return 0;
}
