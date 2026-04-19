#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main(void){
  int server_fd;
  int client_socket;
  struct sockaddr_in address;

  int addrlen = sizeof(address);

  //Create socket
  server_fd = socket(AF_INET,SOCK_STREAM,0);

  if(server_fd<0){
    perror("Socket Failed");
    exit(EXIT_FAILURE);
  }

  //Server address setup
  address.sin_family = AF_INET; //Use IP4
  address.sin_addr.s_addr = INADDR_ANY; //0.0.0.0 (accept connections from any IP address)
  address.sin_port = htons(PORT); //Assigned PORT formatted properly such that it can be read(big endian format)



  //Bind socket to port
  if(bind(server_fd,(struct sockaddr *) &address,addrlen)<0){ 
    perror("Bind Failed");
    exit(EXIT_FAILURE);
  }


  if(listen(server_fd,3)<0){ //3 = max no of connections os queues
    perror("Listen");
    exit(EXIT_FAILURE);
  }

  printf("Server listening on PORT %d.\n",PORT);

  //Accept connection from client
  client_socket = accept(server_fd,(struct sockaddr*) &address, (socklen_t *)&addrlen); //pointer for addrlen as accept can modify it also struct sockadrr only as we want to define ip4 struct as generic
  
  if(client_socket<0){
    perror("Accept");
    exit(EXIT_FAILURE);
  }

  printf("Client Connected!!\n");


  //Respond
  char *message = "Welcome to the server\n";
  send(client_socket,message,strlen(message),0);// 0 => flags , no special behaviour

  //Close sockets
  close(client_socket);
  close(server_fd);

  return 0;
}