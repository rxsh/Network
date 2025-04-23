/* Server code in C */

#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream> // std::cout
#include <thread>   // std::thread, std::this_thread::sleep_for

#include <list>

using namespace std;

void ReadThreadSocket(int cli){

  char buffer[300];
  int m;
  do{
    m = read(cli,buffer,m);
    buffer[m] = '\0';

    if(m < 0){
      cerr << "error read";
      close(cli);
      exit(EXIT_FAILURE);
    }
    
    printf("\nx:%s\n",buffer);
    printf("new message: ");

  } while(strncmp(buffer,"exit",4) != 0);
}


int main(){

  int sock;
  struct sockaddr_in stSock;
  int n;
  int r;
  char buffer[300];

  sock = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);

  if(sock < 0){
    cout << "error socket.\n";
    exit(EXIT_FAILURE);
  }

  memset(&stSock,0,sizeof(stSock));

  stSock.sin_family = AF_INET;
  stSock.sin_port = htons(45000);
  r = inet_pton(AF_INET,"127.0.0.1",&stSock.sin_addr);

  if(r < 0){
    cout << "error first parameter";
    exit(EXIT_FAILURE);
  }

  else if(r == 0){
    cout << "second parameter.\n";
    exit(EXIT_FAILURE);
  }

  if(connect(sock,(struct sockaddr*)&stSock,sizeof(struct sockaddr)) < 0){
    cout << "error connection.\n";
    exit(EXIT_FAILURE);
  }


  do{
    thread(ReadThreadSocket,sock).detach();
    printf("\nenter message: "); fgets(buffer,100,stdin);
    n = strlen(buffer);
    buffer[n] = '\0';
    n = write(sock,buffer,100);

  } while(strncmp(buffer,"exit",4) != 0);
  
  shutdown(sock,SHUT_RDWR);
  close(sock);

  return 0;
}