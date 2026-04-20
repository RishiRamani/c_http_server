#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "server.h"
#include "http.h"
#include <pthread.h>

// since threads share memory we need a unique copy of client socket 
void* thread_handler(void* arg){
  int client_socket = *(int*)arg; //make arg pointer to int then dereference it
  free(arg); //free arg as no longer needed

  handle_request(client_socket);

  //Close socket after handling request
  close(client_socket);
  return NULL;
}

void start_server(int port){
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
  address.sin_port = htons(port); //Assigned PORT formatted properly such that it can be read(big endian format)

  //To avoid os wait time regarding server port usage
  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  //Bind socket to port
  if(bind(server_fd,(struct sockaddr *) &address,addrlen)<0){ 
    perror("Bind Failed");
    exit(EXIT_FAILURE);
  }

  if(listen(server_fd,3)<0){ //3 = max no of connections os queues
    perror("Listen");
    exit(EXIT_FAILURE);
  }

  printf("Server listening on PORT %d.\n",port);

  while(1){
    //Accept connection from client
    client_socket = accept(server_fd,(struct sockaddr*) &address, (socklen_t *)&addrlen);

    if(client_socket<0){
        perror("Accept");
        exit(EXIT_FAILURE);
    }

    //Concurrency via thread
    int* arg = malloc(sizeof(int));
    *arg = client_socket;

    pthread_t tid;
    pthread_create(&tid,NULL,thread_handler,(void*)arg);

    //ensure the worker thread runs seperately from main thread
    pthread_detach(tid);

  }

  close(server_fd);
}