#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "include/http.h"
#include "include/utils.h"
#include "include/config.h"
#include <arpa/inet.h>
#include <pthread.h>
#include "include/handlers.h"
#include "include/storage.h"

extern int status_code;

void handle_request(int client_socket){
  char buffer[HTTP_BUFFER_SIZE];  
  char post_data[POST_DATA_BUFFER_SIZE];

  while(1){
    //Connection alive or dead
    int keep_alive = 1;
    int total_read=0;

    int client_disconnected=0;
    //Read headers
    while(1){
      memset(buffer, 0, sizeof(buffer));
      total_read = 0;
      memset(post_data, 0, sizeof(post_data));
      int no = read(client_socket,buffer+total_read,sizeof(buffer)-total_read);
      if(no<=0){
        client_disconnected=1;
        break; //client closed connection or header content over
      }
      total_read+=no;
      if(strstr(buffer,"\r\n\r\n")!=NULL){ //upon end of header stop reading (also contains some part of body potentially)
        break;
      }
    }
    if(client_disconnected) break;

    buffer[total_read] = '\0';
    char* header_end = strstr(buffer,"\r\n\r\n");
    char* body = NULL;
    int prev_read=0;
    if(header_end!=NULL){
      body=header_end+4;
      //as TCP can deliver content with delay we must read until content length amount of data is read
      prev_read = total_read - (body-buffer); 
      if(prev_read>=0){
        memcpy(post_data,body,prev_read);
      }
    }
    char* http = strtok(buffer,"\r\n");// get the METHOD /PATH HTTPVERSION line
    char* line;
    char * connection = NULL;
    int content_length = 0;

    //Process Headers
    while((line=strtok(NULL,"\r\n")) !=NULL){
      if(strlen(line)==0){
        break; //empty line = end of header
      }
      //Connection Type
      if(strncasecmp(line,"Connection:",11)==0){ //if first 11 char of line = Connection
        connection = line+strlen("Connection:"); // read type of connection
        while (*connection == ' ')
        {
          connection++;
        }
      }
      //Content Length (in case of body)
      if(strncasecmp(line,"Content-Length:",15)==0){
        char* contentlen = line+strlen("Content-Length:");
        while (*contentlen == ' ')
        {
          contentlen++;
        }
        content_length = atoi(contentlen);
      }
    }

    //Read content if any
    if(content_length>0 && body!=NULL){
      //read remaining
      while(prev_read<content_length){
        int n = read(client_socket,post_data+prev_read,content_length-prev_read);

        if(n<=0){
          break;  
        }

        prev_read+=n;
      }
      post_data[content_length] = '\0';
    } 


    if(connection && strcasecmp(connection,"close")==0){ //strcasecmp compared CLOSE close Close etc
      keep_alive=0;
    }

    char* method=NULL; //intialization safety
    char* path=NULL;

    if(http!=NULL){
      method = strtok(http," ");//split at space
      path = strtok(NULL," ");//split at space again , NULL = CONTINUE
    }
    
    //Respond with HTTP [STATUS] [HEADER] [BODY]
    char response[HTTP_RESPONSE_SIZE];
    char* conn_header = keep_alive ? "keep-alive" : "close";
    file = 0;
    file_size = 0;
    file_buffer = NULL;
    status_code = 200;
    if(method!=NULL && strcmp(method,"GET")==0){
      handle_get(path,response,conn_header);
    }
    //POST check
    else if(method!=NULL && strcmp(method,"POST")==0){
      handle_post(path,response,conn_header,post_data);
    }
    //DELETE check
    else if(method!=NULL && strcmp(method,"DELETE")==0){
      handle_delete(path,response,conn_header);
    }
    //PUT check
    else if(method!=NULL && strcmp(method,"PUT")==0){
      handle_put(path,response,conn_header,post_data);
    }
    else{
      status_code = 405;
      sprintf(response,
        "HTTP/1.1 405 Method Not Allowed\r\n"
        "Connection: %s\r\n\r\n", conn_header);
    }
    

    send(client_socket,response,strlen(response),0);// 0 => flags , no special behaviour
    if(file){ 
      send(client_socket, file_buffer, file_size, 0);
      free(file_buffer);
    }

    //Log request
    log_request(method,path,status_code);
    if(!keep_alive){
      break;
    }
  }
  
}