#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <thread>

using namespace std;

void ReadThreadSocket(int cli) {
    char buffer[300];
    int m;
    do {
        m = read(cli, buffer, sizeof(buffer) - 1);
        if (m <= 0) break;

        buffer[m] = '\0';
        printf("\n%s\n", buffer);

    } while (strncmp(buffer, "exit", 4) != 0);
}

int main() {
    int sock;
    struct sockaddr_in stSock;
    int n;
    char buffer[300];

    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        cout << "Error al crear socket.\n";
        return EXIT_FAILURE;
    }

    memset(&stSock, 0, sizeof(stSock));
    stSock.sin_family = AF_INET;
    stSock.sin_port = htons(45000);

    if (inet_pton(AF_INET, "127.0.0.1", &stSock.sin_addr) <= 0) {
        cout << "Error en la dirección del servidor.\n";
        return EXIT_FAILURE;
    }

    if (connect(sock, (struct sockaddr*)&stSock, sizeof(struct sockaddr)) < 0) {
        cout << "Error en la conexión.\n";
        return EXIT_FAILURE;
    }

    thread(ReadThreadSocket, sock).detach();

    do {
        printf("\ntype message - *ID private message: ");
        fgets(buffer, sizeof(buffer), stdin);
        n = strlen(buffer);
        buffer[n] = '\0';
        write(sock, buffer, n);

    } while (strncmp(buffer, "exit", 4) != 0);

    shutdown(sock, SHUT_RDWR);
    close(sock);
    return 0;
}
