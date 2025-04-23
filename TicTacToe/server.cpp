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
#include <queue>

using namespace std;

mutex clients_mutex;
map<string, int> clients;


struct TicTacToe{

    vector<vector<char>> board;
    string playerX; string playerO;
    int sockX, sockO;
    bool gameStart;
    queue<string> spectators;
    char turn;

    TicTacToe() : board(3,vector<char>(3,' ')), gameStart(false), turn('X') {}
};

TicTacToe game;

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

void broadcastSpects(const string& msg){
    queue<string> tmp = game.spectators;
    while(!tmp.empty()){
        string spect = tmp.front();
        tmp.pop();
        if(clients.find(spect) != clients.end()){
            sendClient(clients[spect], msg);
        }
    }
}

void updatePlayers(){

    string board = "\nCurrent Board: \n";
    for(int i=0; i<3; i++){
        for(int j=0; j<3; j++){
            board += game.board[i][j];
            if(j < 2) board += "|";
        }
        board += '\n';
        if(i<2) board += "-------\n";
    }
    board += '\n';

    if(game.gameStart){
        if(!game.playerX.empty() && clients.find(game.playerX) != clients.end()){
            string msgX = board;
            if(game.turn == 'X') msgX += "Is your turn (X). Enter your movement (row column. ej 1 2)";
            else msgX += "waiting player O...\n";
            sendClient(game.sockX,msgX);
        }
        if(!game.playerO.empty() && clients.find(game.playerO) != clients.end()){
            string msgO = board;
            if(game.turn == 'O') msgO += "Is your turn (O). Enter your movement (row column. ej 1 2)";
            else msgO += "waiting player X...\n";
            sendClient(game.sockO,msgO);
        }
    }

    broadcastSpects(board);
}

bool checkWinner(char player){
    // check rows 
    for(int i=0; i<3; i++){
        if(game.board[i][0] == player && game.board[i][1] == player && game.board[i][2] == player){
            return true;
        }
    }

    // check columns
    for(int j=0; j<3; j++){
        if(game.board[0][j] == player && game.board[1][j] == player && game.board[2][j] == player){
            return true;
        }
    }

    // check diagonals
    if(game.board[0][0] ==player && game.board[1][1] == player && game.board[2][2] == player){
        return true;
    } else if(game.board[2][0] == player && game.board[1][1] == player && game.board[0][2] == player){
        return true;
    }

    return false;

}

bool checkDraw(){
    for(int i=0; i<3; i++){
        for(int j=0; j<3; j++){
            if(game.board[i][j] == ' '){
                return false;
            }
        }
    }
    return true;
}

void handleGame(const string& nickname, int clientFD, const string& nextMove){

    char player = (nickname != game.playerO) ? 'X' : 'O';
    if(player != game.turn){
        sendClient(clientFD,"no es tu turno.\n");
        return;
    }

    // parse the movement
    istringstream ss(nextMove);
    int row, col;
    if(!(ss >> row >> col) || row < 1 || row > 3 || col < 1 || col > 3){
        sendClient(clientFD,"Invalid move. Please try again (row column).\n");
        return;
    }

    // verify if casilla is empty
    if(game.board[row-1][col-1] != ' '){
        sendClient(clientFD, "casilla ocupada. Elige otra\n");
        return;
    }

    game.board[row-1][col-1] = player;

    if(checkWinner(player)){
        string msgWin = "¡Player " + nickname + " (" + player + ") " + "win.\n";
        sendClient(clientFD, msgWin);
        if(player == 'X' && clients.find(game.playerO) != clients.end()){
            sendClient(game.sockO, msgWin);
        } else if(player == 'O' && clients.find(game.playerX) != clients.end()){
            sendClient(game.sockX,msgWin);
        }
        broadcastSpects(msgWin);

        // reset game 
        game = TicTacToe();
        return;

    }

    if(checkDraw()){
        string msgDraw = "The game has finished in draw.\n";
        sendClient(clientFD,msgDraw);
        if(clients.find(game.playerX) != clients.end()){
            sendClient(game.sockX,msgDraw);
        } 
        if(clients.find(game.playerO) != clients.end()){
            sendClient(game.sockO,msgDraw);
        }

        broadcastSpects(msgDraw);

        // reset game
        game = TicTacToe();
        return;
    }

    // change turn
    game.turn = (player == 'X') ? 'O' : 'X';

    // update spects
    updatePlayers();
    
}

void handleGameJoin(const string& nickname, int clientFD){

    if(game.gameStart){
        // game in progress
        sendClient(clientFD,"Ya hay dos jugadores. Deseas ver el juego (S/N): ");

        // read response
        char rd[10];
        int n = read(clientFD,rd, sizeof(rd)-1);
        if(n>0){
            rd[n] = '\0';
            if(toupper(rd[0]) == 'S'){
                game.spectators.push(nickname);
                sendClient(clientFD,"Now you are like spectator.\n");
                updatePlayers();
            } else{
                sendClient(clientFD,"Ok. See you soon.\n");
            }
        }
    } else{
        if(game.playerX.empty()){
            game.playerX = nickname;
            game.sockX = clientFD;
            sendClient(clientFD,"You are player X. Waiting other player.\n");
        } else if(game.playerO.empty()){
            game.playerO = nickname;
            game.sockO = clientFD;
            game.gameStart = true;

            // notify to both players
            sendClient(game.sockX,"The game start. You are player X");
            sendClient(game.sockO,"The game start. You are player O");

            updatePlayers();
        }
    }

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
        cout << "Size of del msg: " << msg.size() << endl;
        
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
