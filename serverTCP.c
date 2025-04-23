#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(){

    char *myIP = "127.0.0.1";
    int PORT = 4300;
    int TCPserver, TCPclient;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[1024];
    int n;

    TCPserver = socket(AF_INET,SOCK_STREAM,0);

    if(TCPserver < 0){
        perror("[X] TCP Socket error");
        exit(EXIT_FAILURE);
    }

    printf("[+] TCP Socket created.\n");

    memset(&server_addr,'\0',sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = PORT;
    server_addr.sin_addr.s_addr = inet_addr(myIP);

    n = bind(TCPserver, (const struct sockaddr*)&server_addr, sizeof(server_addr));

    if(n < 0){
        perror("[X] Bind error.\n");
        exit(EXIT_FAILURE);
    }

    printf("Bind to the port number: %d...\n", PORT);
    listen(TCPserver, 3);

    printf("Listening...\n\n");

    TCPclient = accept(TCPserver, (struct sockaddr*)&client_addr, &addr_size);

    if(TCPclient < 0){
        perror("[X] accept failed.\n");
        exit(EXIT_FAILURE);
    }

    printf("[+] Connection established...\n");

    while(1){

        memset(buffer,'\0', sizeof(buffer));
        int readBytes = read(TCPclient, buffer, sizeof(buffer));
        if(readBytes < 0){
            printf("client disconnected\n");
            break;
        }

        printf("Client: %s\n", buffer);


        if(strncmp(buffer, "chau", 4) == 0){
            printf("Client said 'chau'. Closing connections.\n");
            break;
        }

        send(TCPclient, buffer, readBytes, 0);
    }

    close(TCPclient);
    close(TCPserver);


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

    int server_socket, client_Socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    char buffer[1024];
    int n;

    server_socket = socket(AF_INET,SOCK_STREAM,0);
    if (server_socket < 0){
        perror("[!] SOCKET ERROR");
        exit(EXIT_FAILURE);
    }

    printf("[+] TCP SERVER SOCKET CREATED.\n");

    memset(&server_addr,'\0',sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    n = bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));

    if (n < 0){
        perror("[-] BIND ERROR");
        exit(EXIT_FAILURE);
    }

    printf("[+] BIND TO THE PORT NUMBER: %d\n", port);

    listen(server_socket, 5);
    printf("Listening...\n");

    while(1){
        addr_size = sizeof(client_addr);
        client_Socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);

        printf("[+] CLIENT CONNECTED.\n");
        bzero(buffer,1024);
        recv(client_Socket, buffer, sizeof(buffer),0);
        printf("Client: %s\n", buffer);

        bzero(buffer,1024);
        strcpy(buffer,"HI, THIS IS SERVER. HAVE A NICE DAY!!!");
        printf("SERVER: %s\n",buffer);
        send(client_Socket,buffer,strlen(buffer), 0);

        close(client_Socket);
        printf("[+] CLIENT DISCONNECTED!!!\n\n");
    }

}
*/