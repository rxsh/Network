/* Server code in C */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <iostream>

void readSocketThread(int cliSocket){
  char buffer[300];    
  do{

      int n = read(cliSocket,buffer,100);
      buffer[n] = '\0';
      printf("\n%s\n", buffer);
      printf("enter a message: ");
  } while(strncmp(buffer,"exit",4) != 0);
  
}


int main(void) {
  struct sockaddr_in stSockAddr;
  struct sockaddr_in cli_addr;
  socklen_t cli_size;
  int client;
  int SocketFD;
  char buffer[256];
  char buffer2[256];
  int n;

  if ((SocketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket");
    exit(1);
  }

  if (setsockopt(SocketFD, SOL_SOCKET, SO_REUSEADDR, "1", sizeof(int)) == -1) {
    perror("Setsockopt");
    exit(1);
  }

  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(45000);
  stSockAddr.sin_addr.s_addr = INADDR_ANY;

  if (bind(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(struct sockaddr)) ==
      -1) {
    perror("Unable to bind");
    exit(1);
  }

  if (listen(SocketFD, 5) == -1) {
    perror("Listen");
    exit(1);
  }

  client = sizeof(struct sockaddr_in);

  int clientFD = accept(SocketFD, (struct sockaddr *)&cli_addr,
                        &cli_size);
  // for (;;) {

  // if (cli)

  // bzero(buffer,256);
  // bzero(buffer2,256);

  do {
    printf("enter your message: ");
    fgets(buffer, sizeof(buffer), stdin);
    n = strlen(buffer);
    buffer[n] = '\0';
    n = write(SocketFD, buffer, 100);
    n = read(SocketFD, buffer, 100);
    buffer[n] = '\0';
    printf("%s\n", buffer);

  } while (strcmp(buffer, "exit") != 0);

  close(clientFD);
  close(SocketFD);
  return 0;
}
