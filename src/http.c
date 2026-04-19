#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "http.h"
#include "utils.h"
#include <arpa/inet.h>

void handle_request(int client_socket){

  char buffer[2048];

  //Read client's message
  int bytes_read = read(client_socket,buffer,sizeof(buffer));

  if(bytes_read<0){
    perror("Read");
    return;
  }
  else if(bytes_read==0){ //incase client closes connection immediately
    return;
  }

  buffer[bytes_read] = '\0';

  char* http = strtok(buffer,"\r\n");// get the METHOD /PATH HTTPVERSION line
  char* method=NULL; //intialization safety
  char* path=NULL;

  if(http!=NULL){
    method = strtok(http," ");//split at space
    path = strtok(NULL," ");//split at space again , NULL = CONTINUE
  }
  
  //Respond with HTTP [STATUS] [HEADER] [BODY]
  char response[16384];
  int status_code;

  if(path!=NULL && strcmp(path,"/")==0){
    //Open html file and serve it
    FILE *file = fopen("public/index.html","r");

    if(file==NULL){
      status_code=404;
      sprintf(response,
      "HTTP/1.1 %d Not Found\r\n"
      "Content-Type: text/plain\r\n"
      "\r\n"
      "File not found"
      ,status_code);
    }
    else{
      char file_buffer[8192];
      int bytes = fread(file_buffer,1,sizeof(file_buffer),file);
      file_buffer[bytes] = '\0';
      fclose(file);

      status_code=200;

      //return HTML page
      sprintf(response,
      "HTTP/1.1 %d OK\r\n"
      "Content-Type: text/html\r\n"
      "\r\n"
      "%s"
      ,status_code,file_buffer);
    }
  }
  else{
    char* filename = path+1; //skip the / and get the filename

    char full_path[256];
    sprintf(full_path,"public/%s",filename);

    char * content_type = get_content_type(filename);

    FILE *file = fopen(full_path,"r");

    if(file==NULL){
      status_code=404;
      //filename not in server so return page not found
      sprintf(response,
      "HTTP/1.1 %d Not Found\r\n"
      "Content-Type: text/plain\r\n"
      "\r\n"
      "Page not found"
      ,status_code);
    }
    else{
      status_code=200;

      char file_buffer[8192];
      int bytes = fread(file_buffer,1,sizeof(file_buffer),file);
      file_buffer[bytes] = '\0';
      fclose(file);

      sprintf(response,
      "HTTP/1.1 %d OK\r\n"
      "Content-Type: %s\r\n"
      "\r\n"
      "%s"
      ,status_code,content_type,file_buffer);
    }
  }

  send(client_socket,response,strlen(response),0);// 0 => flags , no special behaviour

  //Log request
  log_request(method,path,status_code);
}