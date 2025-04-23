#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main(){

    char *myIP = "127.0.0.1";
    int PORT = 4300;
    int sock = 0;
    struct sockaddr_in addr;
    socklen_t addr_size;
    char buffer[1024];

    sock = socket(AF_INET,SOCK_STREAM, 0);
    if(sock < 0){
        perror("[X] Socket failed.\n");
        exit(EXIT_FAILURE);
    }

    printf("[+] TCP Socket created.\n");

    memset(&addr,'\0',sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = PORT;
    addr.sin_addr.s_addr = inet_addr(myIP);
    

    if(connect(sock, (const struct sockaddr*)&addr, sizeof(addr)) < 0){
        perror("connection failed.\n");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server. Type messages (type 'chau' to disconnect).\n\n");

    while(1){

        printf("You: "); fgets(buffer, 1024, stdin);

        send(sock, buffer, strlen(buffer), 0);

        if(strncmp(buffer, "chau", 4) == 0){
            printf("Disconnecting from server.\n");
            break;
        }

        // receive server response
        memset(buffer, '\0', 1024);
        int bytesRead = read(sock, buffer, 1024);
        if(bytesRead > 0){
            printf("Server: %s", buffer);
        }
    }

    close(sock);
    return 0;
}

/*
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>

int main(){

    char *ip = "127.0.0.1";
    int port = 4300;

    int sock;
    struct sockaddr_in addr;
    socklen_t addr_size;
    char buffer[1024];
    int n;

    sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0){
        perror("[-] SOCKET ERROR");
        exit(EXIT_FAILURE);
    }

    printf("[+] TCP server socket created.\n");

    memset(&addr,'\0',sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr.s_addr = inet_addr(ip);

    connect(sock,(struct sockaddr*)&addr, sizeof(addr));
    printf("CONNECTED TO THE SERVER.\n");

    bzero(buffer, 1024);
    strcpy(buffer, "HELLO, THIS IS CLIENT.");
    printf("Client: %s\n", buffer);
    send(sock, buffer, strlen(buffer),0);

    bzero(buffer, 1024);
    recv(sock, buffer,sizeof(buffer), 0);
    printf("Server: %s\n", buffer);

    close(sock);
    printf("DISCONNECTED FROM THE SERVER.\n");
}
*/