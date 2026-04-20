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
      char body[BODY_LINE_SIZE];
      if(get_by_id(ID, body) == -1){
        status_code=404;
        build_json_response(response, status_code, "Item Not Found", conn_header);
      }
      else{
        status_code=200;
        char json_data[BODY_LINE_SIZE];
        sprintf(json_data, "{\"id\":%d,\"data\":\"%s\"}", ID, body + strlen("1 : "));
        build_json_response_with_data(response, status_code, json_data, conn_header);
      }
    }else{
      status_code=200;
      char body[RESPONSE_BODY_SIZE] ="";
      int count = get_all(body);
      if(count==0){
        strcpy(body, "[]");
      }
      else{
        char json_array[RESPONSE_BODY_SIZE];
        strcpy(json_array, "[");
        char* line = strtok(body, "\n");
        int first = 1;
        while(line != NULL && strlen(line) > 0){
          if(!first) strcat(json_array, ",");
          char* colon = strstr(line, " : ");
          if(colon != NULL){
            int item_id = atoi(line);
            char* data = colon + 3;
            char json_obj[BODY_LINE_SIZE];
            sprintf(json_obj, "{\"id\":%d,\"data\":\"%s\"}", item_id, data);
            strcat(json_array, json_obj);
            first = 0;
          }
          line = strtok(NULL, "\n");
        }
        strcat(json_array, "]");
        strcpy(body, json_array);
      }
      build_json_response_with_data(response, status_code, body, conn_header);
    }
  }else{
    if(path!=NULL && strcmp(path,"/")==0){
    //Open html file and serve it
    FILE *file = fopen("public/index.html","r");

    if(file==NULL){
      status_code=404;
      build_json_response(response, status_code, "File not found", conn_header);
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
      build_json_response(response, status_code, "File not found", conn_header);
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
    if(total_data<STORAGE_SIZE){
      int new_id = add_data(post_data);
      status_code=201;
      char json_data[BODY_LINE_SIZE];
      sprintf(json_data, "{\"id\":%d,\"message\":\"Data stored successfully\"}", new_id);
      build_json_response_with_data(response, status_code, json_data, conn_header);
    }
    else{
      status_code=507;
      build_json_response(response, status_code, "Storage Full", conn_header);
    }
  }
  else{
    status_code=404;
    build_json_response(response, status_code, "Invalid POST route", conn_header);
  }
}


void handle_delete(char* path, char* response, char* conn_header){
  if(path!=NULL && strncmp(path,"/data",5)==0){
    char* id = strstr(path,"?id=");
    if(id!=NULL){
      id+=4;
      int ID = atoi(id);
      if(delete_by_id(ID) == -1){
        status_code=404;
        build_json_response(response, status_code, "Item Not Found", conn_header);
      }
      else{
        status_code=200;
        build_json_response(response, status_code, "Item deleted successfully", conn_header);
      }
    }
    else{
      delete_all();
      status_code=200;
      build_json_response(response, status_code, "All data deleted successfully", conn_header);
    }
  }
  else{
    status_code=404;
    build_json_response(response, status_code, "Invalid DELETE route", conn_header);
  }
}


void handle_put(char* path, char* response, char* conn_header, char* post_data){
  if(path!=NULL && strncmp(path,"/data",5)==0){
    char* id = strstr(path,"?id=");
    if(id!=NULL){
      id+=4;
      int ID = atoi(id);
      if(update_by_id(ID, post_data) == -1){
        status_code=404;
        build_json_response(response, status_code, "Item Not Found", conn_header);
      }
      else{
        status_code=200;
        char json_data[BODY_LINE_SIZE];
        sprintf(json_data, "{\"id\":%d,\"message\":\"Item updated successfully\"}", ID);
        build_json_response_with_data(response, status_code, json_data, conn_header);
      }
    }else{
      status_code=400;
      build_json_response(response, status_code, "The ID field is required", conn_header);
    }
  }else{
    status_code=404;
    build_json_response(response, status_code, "Invalid PUT route", conn_header);
  }
}