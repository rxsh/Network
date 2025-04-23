#include <cstdlib>
#include <iostream>
#include <map>
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <list>
#include <string.h>
#include <arpa/inet.h>

using namespace std;

list<int> listClients;
map<int, int> clientSockets;  // Mapea ID de cliente con su socket
int clientIDCounter = 1;
mutex mtx;

void sendToClient(int client, const string &message) {
    write(client, message.c_str(), message.size());
}

void readSocket(int cli, int clientID) {
    char buffer[300];
    int n;

    string idMessage = "your ID is: " + to_string(clientID) + "\n";
    sendToClient(cli, idMessage);

    do {
        n = read(cli, buffer, sizeof(buffer) - 1);
        if (n <= 0) break;

        buffer[n] = '\0';
        string msg(buffer);

        if (msg.rfind("*", 0) == 0) {  // private message
            size_t position = msg.find(":");
            if (position != string::npos) {
                int targetID = stoi(msg.substr(1, position - 1));
                string actualMsg = msg.substr(position + 1);

                mtx.lock();
                if (clientSockets.count(targetID)) {
                    sendToClient(clientSockets[targetID], "[private] client " + to_string(clientID) + ": " + actualMsg);
                } else {
                    sendToClient(cli, "Client not found.\n");
                }
                mtx.unlock();
            }
        } else {  // public message
            mtx.lock();
            for (int client : listClients) {
                if (client != cli) {  // No reenviar al mismo cliente
                    sendToClient(client, "Client " + to_string(clientID) + ": " + msg);
                }
            }
            mtx.unlock();
        }
    } while (strncmp(buffer, "exit", 4) != 0);

    cout << "client " << clientID << " disconnected.\n";

    mtx.lock();
    listClients.remove(cli);
    clientSockets.erase(clientID);
    shutdown(cli,SHUT_RDWR);
    close(cli);
    mtx.unlock();
}

int main() {
    int sv, cl;
    struct sockaddr_in stSock;
    int n;

    sv = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sv < 0) {
        cout << "error socket.\n";
        return EXIT_FAILURE;
    }

    memset(&stSock, 0, sizeof(stSock));
    stSock.sin_family = AF_INET;
    stSock.sin_port = htons(45000);
    stSock.sin_addr.s_addr = INADDR_ANY;

    if (bind(sv, (const struct sockaddr*)&stSock, sizeof(stSock)) < 0) {
        cout << "error bind.\n";
        return EXIT_FAILURE;
    }

    if (listen(sv, 5) < 0) {
        cout << "error listen.\n";
        return EXIT_FAILURE;
    }

    for (;;) {
        cl = accept(sv, NULL, NULL);
        if (cl < 0) {
            cout << "error connection.\n";
            continue;
        }

        mtx.lock();
        int clientID = clientIDCounter++;
        listClients.push_back(cl);
        clientSockets[clientID] = cl;
        mtx.unlock();

        cout << "new client with ID: " << clientID << "\n";
        thread(readSocket, cl, clientID).detach();
    }

    close(sv);
    return 0;
}
