#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main(){

    int sock;
    struct sockaddr_in sv_addr;
    int port = 5000;
    char buffer[1024];
    socklen_t addr_len = sizeof(sv_addr);

    sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0){
        perror("[+] error socket.\n");
        exit(EXIT_FAILURE);
    }

    printf("[+] socket created.\n");

    memset(&sv_addr,'\0',sizeof(sv_addr));

    sv_addr.sin_family=AF_INET;
    sv_addr.sin_port=port;
    sv_addr.sin_addr.s_addr=INADDR_ANY;

    // send message

    while(1){
        fgets(buffer,1024,stdin);
        sendto(sock,buffer,strlen(buffer),0,(struct sockaddr*)&sv_addr,addr_len);
        recvfrom(sock, buffer, sizeof(buffer),0, (struct sockaddr*)&sv_addr,&addr_len);
        printf("Server: %s\n",buffer);
        if(strncmp(buffer, "chau", 4)==0){
            break;
        }

    }

    close(sock);
}