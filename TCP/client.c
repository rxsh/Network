#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

int main(){

    int port = 45000;
    char *ip = "172.16.19.18";
    int sock;
    struct sockaddr_in addr;
    socklen_t address_size;
    char buffer[1024];
    int n;

    sock = socket(AF_INET,SOCK_STREAM,0);

    if(sock < 0){
        perror("[X] socket error.\n");
        exit(EXIT_FAILURE);
    }

    printf("[+] sock created.\n");

    memset(&addr,'\0',sizeof(addr));

    addr.sin_family=AF_INET;
    addr.sin_port=port;
    addr.sin_addr.s_addr = inet_addr(ip);

    n = connect(sock,(const struct sockaddr*)&addr,sizeof(addr));

    if(n<0){
        perror("[X] connection failed.\n");
        exit(EXIT_FAILURE);
    }

    printf("[+] connect to the server.\n");

    while(1){

        printf("You: ");
        fgets(buffer,1024,stdin);

        send(sock, buffer,sizeof(buffer),0);

        if(strncmp(buffer, "chau", 4) == 0){
            break;
        }

        memset(buffer,'\0',sizeof(buffer));
        int bytesText = read(sock,buffer,sizeof(buffer));
        if(bytesText > 0){
            printf("Server: %s",buffer);
        }

    }

    close(sock);

}