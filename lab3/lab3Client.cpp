#include <iostream>
#include <cstring>
#include <ostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>

using namespace std;

void readThreadSocket(int clienFD){

    char buffer[300];
    int n;
    while(true){

        memset(buffer,0,sizeof(buffer));
        n = read(clienFD,buffer,sizeof(buffer)-1);
        if(n <0) break;

        buffer[n] = '\0';

        cout <<buffer << endl;
        cout << "> ";
        cout.flush();

    }

    close(clienFD);

}

int main() {
    int sock;
    struct sockaddr_in serverAddr;
    char buffer[300];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "Error al crear el socket\n";
        return EXIT_FAILURE;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(45000);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error al conectar con el servidor\n";
        return EXIT_FAILURE;
    }

    cout << "Connect to the server. Enter the next commands" << endl;
    cout << "- N<name> to register (ej. Ncesar)" << endl;
    cout << "- L to display connected users" << endl;
    cout << "- M<name> to send private message" << endl;
    cout << "- B to send a public message" << endl;
    cout << "- Q to quit" << endl;
    
    thread(readThreadSocket,sock).detach();
    while (true) {
        string message;
        getline(cin, message);

        if (message == "exit") {
            write(sock,message.c_str(),message.size());
            break;
        }

        write(sock, message.c_str(), message.size());
    }

    close(sock);
    cout << "left from the server.\n";
    return 0;
}

