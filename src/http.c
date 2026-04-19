#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "http.h"
#include "utils.h"
#include <arpa/inet.h>

void handle_request(int client_socket){
  char buffer[2048];  
  char post_data[2048];
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
    char response[16384];
    int status_code;
    char* conn_header = keep_alive ? "keep-alive" : "close";

    if(method!=NULL && strcmp(method,"GET")==0){
      if(path!=NULL && strcmp(path,"/")==0){
        //Open html file and serve it
        FILE *file = fopen("public/index.html","r");

        if(file==NULL){
          status_code=404;
          sprintf(response,
          "HTTP/1.1 %d Not Found\r\n"
          "Content-Type: text/plain\r\n"
          "Content-Length: %zu\r\n"
          "Connection: %s\r\n"
          "\r\n"
          "File not found"
          ,status_code,strlen("Page not found"),conn_header);
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
          "Content-Length: %zu\r\n"
          "Connection: %s\r\n"
          "\r\n"
          "%s"
          ,status_code,strlen(file_buffer),conn_header,file_buffer);
        }
      }
      else{
        if(path==NULL){
          break;
        }
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
          "Content-Length: %zu\r\n"
          "Connection: %s\r\n"
          "\r\n"
          "Page not found"
          ,status_code,strlen("Page not found"),conn_header);
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
          "Content-Length: %zu\r\n"
          "Connection: %s\r\n"
          "\r\n"
          "%s"
          ,status_code,content_type,strlen(file_buffer),conn_header,file_buffer);
        }
      }
    }
    //POST
    else if(method!=NULL && strcmp(method,"POST")==0){
      if(path!=NULL && strcmp(path,"/submit")==0){
              printf("I work %s\n",post_data);
        status_code=200;
        sprintf(response,
        "HTTP/1.1 %d OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %zu\r\n"
        "Connection: %s\r\n"
        "\r\n"
        "Data received: %s",status_code,strlen("Data received: ")+strlen(post_data),conn_header,post_data);
      }
      else{
        status_code=404;
        sprintf(response,
          "HTTP/1.1 %d Not Found\r\n"
          "Content-Type: text/plain\r\n"
          "Content-Length: %zu\r\n"
          "Connection: %s\r\n"
          "\r\n"
          "Invalid Post Route"
          ,status_code,strlen("Invalid Post Route"),conn_header);
      }
    }
    else{
      sprintf(response,
        "HTTP/1.1 405 Method Not Allowed\r\n"
        "Connection: %s\r\n\r\n", conn_header);
    }
    

    send(client_socket,response,strlen(response),0);// 0 => flags , no special behaviour

    //Log request
    log_request(method,path,status_code);
    if(!keep_alive){
      break;
    }
  }
  
}