#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main(){

    int sock;
    char buffer[1024];
    int port = 5000;
    struct sockaddr_in sv_addr, cl_addr;
    socklen_t addr_len = sizeof(cl_addr);
    int n;

    sock = socket(AF_INET,SOCK_DGRAM,0);

    if(sock < 0){
        perror("[X] socket erorr.\n");
        exit(EXIT_FAILURE);
    }

    printf("[+] socket created.\n");

    memset(&sv_addr,'\0',sizeof(sv_addr));
    sv_addr.sin_family=AF_INET;
    sv_addr.sin_port=port;
    sv_addr.sin_addr.s_addr=INADDR_ANY;

    if(bind(sock,(const struct sockaddr*)&sv_addr,sizeof(sv_addr))<0){
        perror("[X] bind error.\n");
        exit(EXIT_FAILURE);
    }

    printf("[+] bind to the port number: %d\n", port);

    // receive data

    while(1){
        recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&cl_addr, &addr_len);
        printf("Client: %s\n",buffer);
        if(strncmp(buffer,"chau",4) == 0){
            break;
        }

        fgets(buffer,1024,stdin);
        sendto(sock,buffer,strlen(buffer), 0,(struct sockaddr*)&cl_addr,addr_len);    
    }
    
    close(sock);

}