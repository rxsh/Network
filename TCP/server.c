#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(){

    int port=5000;
    int sockServer, sockClient;
    struct sockaddr_in sv_addr, cl_addr;
    socklen_t address_size;
    char buffer[1024];
    int n;

    sockServer = socket(AF_INET,SOCK_STREAM,0);

    if(sockServer < 0){
        perror("[X] sock error.\n");
        exit(EXIT_FAILURE);
    }

    printf("[+] sock created.\n");

    memset(&sv_addr,'\0',sizeof(sv_addr));

    sv_addr.sin_family=AF_INET;
    sv_addr.sin_port=port;
    sv_addr.sin_addr.s_addr= INADDR_ANY;

    n = bind(sockServer,(const struct sockaddr*)&sv_addr, sizeof(sv_addr));

    if(n < 0){
        perror("[X] bind error.\n");
        exit(EXIT_FAILURE);
    }

    printf("[+] bind to the port number: %d\n",port);

    listen(sockServer,3);
    printf("[!] Listening...\n");

    sockClient = accept(sockServer,(struct sockaddr*)&cl_addr,&address_size);

    if(sockClient < 0){
        perror("[X] client error\n");
        exit(EXIT_FAILURE);
    }

    printf("[+] connection established.\n");

    while(1){

        memset(buffer,'\0',sizeof(buffer));

        int bytesText = read(sockClient,buffer,sizeof(buffer));

        if(bytesText < 0){
            printf("[X] client disconnected.\n");
            break;
        }

        printf("Client: %s",buffer);

        if(strncmp(buffer,"chau",4) == 0){
            break;
        }

        send(sockClient,buffer,sizeof(buffer),0);
    }

    close(sockClient);
    close(sockServer);
}