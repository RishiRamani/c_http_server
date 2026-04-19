#include <stdio.h>
#include <string.h>
#include <time.h>
#include "utils.h"

//Logs request with timestamp
void log_request(char *method, char *path, int status_code){
  time_t request_time = time(NULL);
  struct tm *tm_info = localtime(&request_time);

  char time_str[100];
  strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

  printf("[%s] %s %s %d\n",time_str,method,path,status_code);
}

//Returns correct MIME type based on file extension
char* get_content_type(char *filename){
  char* ext = strrchr(filename,'.');

  if(ext!=NULL){
    if(strcmp(ext,".html")==0){
      return "text/html";
    }else if(strcmp(ext,".css")==0){
      return "text/css";
    }else if(strcmp(ext,".js")==0){
      return "application/javascript";
    }
  }

  return "text/plain";
}