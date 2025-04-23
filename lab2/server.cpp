/* Client code in C */

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

list<int> listClients;


void writeSocket(int cli){

  char buffer[300];
  int m;
  do{

    printf("enter message: "); fgets(buffer,100,stdin);
    m = strlen(buffer);
    buffer[m] = '\0';
    m = write(cli,buffer,m);
    if(m < 0){
      cout << "error write";
      break;
    }
  } while(strncmp(buffer,"exit",4) != 0);
  shutdown(cli,SHUT_RDWR);
  close(cli);
}

void readSocket(int cli){

  char buffer[300];
  int n;
  do{
    n = read(cli,buffer,sizeof(buffer));
    buffer[n] = '\0';
    for(list<int>::iterator it=listClients.begin(); it != listClients.end(); it++){
      write(*it,buffer,n);
    }
    //printf("message: %s",buffer);
  }while(strncmp(buffer,"exit",4) != 0);
  shutdown(cli,SHUT_RDWR);
  close(cli);
  listClients.remove(cli);

}

int main(){

  int sv,cl;
  struct sockaddr_in stSock;
  char buffer[300];
  int n;

  sv = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
  if(sv < 0){
    cout << "error socket.\n";
  }

  memset(&stSock,0,sizeof(stSock));

  stSock.sin_family = AF_INET;
  stSock.sin_port = htons(45000);
  stSock.sin_addr.s_addr = INADDR_ANY;

  n = bind(sv, (const struct sockaddr*)&stSock,sizeof(stSock));
  if(n < 0){
    cout << "bind error.\n";
  }

  if(listen(sv,5) <0){
    cout << "error listen.\n";
  }

  for(;;){

    cl = accept(sv,NULL,NULL);
    
    if(cl < 0){
      cout << "error connected.\n";
    }

    listClients.push_back(cl);
    cout << "new client connected.\n";

    //thread(writeSocket,cl).detach();
    thread(readSocket,cl).detach();

  }

  return 0;
}
