#include "include/handlers.h"
#include "include/storage.h"
#include "include/utils.h"
#include "include/config.h"
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int status_code = 200;
int file = 0;
long file_size = 0;
char* file_buffer = NULL;

void handle_get(char* path, char* response, char* conn_header){
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
        char body[BODY_LINE_SIZE];
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
      char body[RESPONSE_BODY_SIZE] ="";
      pthread_mutex_lock(&lock);
      if(total_data==0){
        sprintf(body,"No data available");
      }
      else{
        for(int i=0;i<total_data;i++){
          char line[BODY_LINE_SIZE];
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
      char file_buffer[RESPONSE_BODY_SIZE];
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
      return;
    }
    char* filename = path+1; //skip the / and get the filename

    char full_path[FILE_PATH_SIZE];
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


void handle_post(char* path, char* response, char* conn_header, char* post_data){
  //Endpoint check
  if(path!=NULL && strcmp(path,"/submit")==0){
    pthread_mutex_lock(&lock);
    //Critical Section
    //Storage Check
    if(total_data<STORAGE_SIZE){
      storage[total_data].id = next_id;
      strcpy(storage[total_data].data,post_data);
      char body[BODY_LINE_SIZE];
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


void handle_delete(char* path, char* response, char* conn_header){
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
        //removal of data and compaction
        for(int i=foundid;i<total_data-1;i++){
          storage[i]=storage[i+1];
        }
        total_data--;
        status_code=204;
        sprintf(response,
        "HTTP/1.1 %d No Content\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 0\r\n"
        "Connection: %s\r\n"
        "\r\n"
        ,status_code,conn_header);
      }
      pthread_mutex_unlock(&lock);
    }
    else{
      //deleteall
      pthread_mutex_lock(&lock);
      total_data=0;
      pthread_mutex_unlock(&lock);
      status_code=204;
      sprintf(response,
      "HTTP/1.1 %d No Content\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: 0\r\n"
      "Connection: %s\r\n"
      "\r\n"
      ,status_code,conn_header);
    }
  }
  
  else{
    status_code=404;
    sprintf(response,
    "HTTP/1.1 %d Not Found\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: %zu\r\n"
    "Connection: %s\r\n"
    "\r\n"
    "Page not found"
    ,status_code,strlen("Page not found"),conn_header);
  }
}


void handle_put(char* path, char* response, char* conn_header, char* post_data){
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
        //updation
        strcpy(storage[foundid].data,post_data);
        status_code=200;
        char body[BODY_LINE_SIZE];
        sprintf(body,"Item with ID : %d updated",ID);
        sprintf(response,
        "HTTP/1.1 %d OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %zu\r\n"
        "Connection: %s\r\n"
        "\r\n"
        "%s"
        ,status_code,strlen(body),conn_header,body);
      }
      pthread_mutex_unlock(&lock);
    }else{
      status_code=400;
      sprintf(response,
      "HTTP/1.1 %d Bad Request\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: %zu\r\n"
      "Connection: %s\r\n"
      "\r\n"
      "The ID Field is Required"
      ,status_code,strlen("The ID Field is Required"),conn_header);
    }
  }else{
    status_code=404;
    sprintf(response,
    "HTTP/1.1 %d Not Found\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: %zu\r\n"
    "Connection: %s\r\n"
    "\r\n"
    "Page not found"
    ,status_code,strlen("Page not found"),conn_header);
  }
}