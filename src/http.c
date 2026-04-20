#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "http.h"
#include "utils.h"
#include <arpa/inet.h>
#include <pthread.h>

#define STORAGE_SIZE 100

struct User{
  int id;
  char data[256];
};

int total_data = 0;
int next_id = 1;

struct User storage[STORAGE_SIZE];

//Mutex
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


//To handle binary files , toggling btw sending header seperately and not
int file=0;

void handle_request(int client_socket){
  char buffer[2048];  
  char post_data[2048];
  long file_size = 0;
  char *file_buffer = NULL;

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
    file = 0;
    if(method!=NULL && strcmp(method,"GET")==0){
      //check for API
      if(path!=NULL && strncmp(path,"/data",5)==0){
        char* id = strstr(path,"?id=");
        if(id!=NULL){
          id+=4;
          int ID = atoi(id);
          int foundid = -1;
          pthread_mutex_lock(&lock);
          //Simple search due to small storage
          for(int i=0;i<total_data;i++){
            if(storage[i].id==ID){
              foundid = i;
              break;
            }
          }
          pthread_mutex_unlock(&lock);
          if(foundid==-1){
            status_code=404;
            sprintf(response,
            "HTTP/1.1 %d Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %zu\r\n"
            "Connection: %s\r\n"
            "\r\n"
            "Item Not Found"
            ,status_code,strlen("Item Not Found"),conn_header);
          }
          else{
            status_code=200;
            char body[512];
            pthread_mutex_lock(&lock);
            sprintf(body,"%d : %s",storage[foundid].id,storage[foundid].data);
            pthread_mutex_unlock(&lock);
            sprintf(response,
            "HTTP/1.1 %d OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %zu\r\n"
            "Connection: %s\r\n"
            "\r\n"
            "%s"
            ,status_code,strlen(body),conn_header,body);
          }
        }else{
          status_code=200;
          char body[8192] ="";
          pthread_mutex_lock(&lock);
          if(total_data==0){
            sprintf(body,"No data available");
          }
          else{
            for(int i=0;i<total_data;i++){
              char line[512];
              sprintf(line,"%d : %s\n",storage[i].id,storage[i].data);
              strcat(body,line);
            }
          }
          pthread_mutex_unlock(&lock);
          sprintf(response,
          "HTTP/1.1 %d OK\r\n"
          "Content-Type: text/plain\r\n"
          "Content-Length: %zu\r\n"
          "Connection: %s\r\n"
          "\r\n"
          "%s"
          ,status_code,strlen(body),conn_header,body);
        }
      }else{
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
          "Content-Length: %d\r\n"
          "Connection: %s\r\n"
          "\r\n"
          "%s"
          ,status_code,bytes,conn_header,file_buffer);
        }
      }
      else{
        if(path==NULL){
          break;
        }
        char* filename = path+1; //skip the / and get the filename

        char full_path[256];
        sprintf(full_path,"public/%s",filename);

        FILE *fp = fopen(full_path,"rb");

        if(fp==NULL){
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
          char * content_type = get_content_type(filename);
          //read in binary to support all files
          fseek(fp, 0, SEEK_END);
          file_size = ftell(fp);
          rewind(fp);
          file_buffer = malloc(file_size);
          fread(file_buffer, 1, file_size, fp);

          file = 1;
          fclose(fp);

          sprintf(response,
          "HTTP/1.1 %d OK\r\n"
          "Content-Type: %s\r\n"
          "Content-Length: %ld\r\n"
          "Connection: %s\r\n"
          "\r\n"
          ,status_code,content_type,file_size,conn_header);
        }
      }
      }
    }
    //POST check
    else if(method!=NULL && strcmp(method,"POST")==0){
      //Endpoint check
      if(path!=NULL && strcmp(path,"/submit")==0){
        pthread_mutex_lock(&lock);
        //Critical Section
        //Storage Check
        if(total_data<STORAGE_SIZE){
          storage[total_data].id = next_id;
          strcpy(storage[total_data].data,post_data);
          char body[512];
          sprintf(body,"Stored with ID: %d",next_id);
          status_code=200;
          sprintf(response,
          "HTTP/1.1 %d OK\r\n"
          "Content-Type: text/plain\r\n"
          "Content-Length: %zu\r\n"
          "Connection: %s\r\n"
          "\r\n"
          "%s",status_code,strlen(body),conn_header,body);
          total_data++;
          next_id++;
        }
        else{
          status_code=507;
          sprintf(response,
            "HTTP/1.1 %d Insufficient Storage\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %zu\r\n"
            "Connection: %s\r\n"
            "\r\n"
            "Storage Full"
            ,status_code,strlen("Storage Full"),conn_header);
        }
        pthread_mutex_unlock(&lock);
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