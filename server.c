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

  printf("Server listening on PORT %d.\n",PORT);
  while(1){
    //Accept connection from client
    client_socket = accept(server_fd,(struct sockaddr*) &address, (socklen_t *)&addrlen); //pointer for addrlen as accept can modify it also struct sockadrr only as we want to define ip4 struct as generic
    //Concurrency via fork 
    int pid = fork();
    if(pid>0){
      close(client_socket);
      continue;
    }
    else if(pid==0){
      if(client_socket<0){
        perror("Accept");
        exit(EXIT_FAILURE);
      }

      printf("Client Connected!!\n");

      char buffer[2048];
      //Read client's message
      int bytes_read = read(client_socket,buffer,sizeof(buffer));

      if(bytes_read<0){
        perror("Read");
        exit(EXIT_FAILURE);
      }
      else if(bytes_read==0){ //incase client closes connection immediately
        close(client_socket);
        continue;
      }

      buffer[bytes_read] = '\0';
      printf("%s\n",buffer);

      char* http = strtok(buffer,"\r\n");// get the METHOD /PATH HTTPVERSION line
      char* method=NULL; //intialization safety
      char* path=NULL;

      if(http!=NULL){
        method = strtok(http," ");//split at space
        path = strtok(NULL," ");//split at space again , NULL = CONTINUE
        printf("Method : %s\n",method);
        printf("Path : %s\n",path);
      }
      
      //Respond with HTTP [STATUS] [HEADER] [BODY]
      char response[16384];

      if(path!=NULL && strcmp(path,"/")==0){
        //Open html file and serve it
        FILE *file = fopen("index.html","r");
        if(file==NULL){
          perror("File open failed");
          exit(EXIT_FAILURE);
        }

        char file_buffer[8192];
        int bytes = fread(file_buffer,1,sizeof(file_buffer),file);
        file_buffer[bytes] = '\0';
        fclose(file);
        //return HTML page
        sprintf(response,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"
        "%s"
        ,file_buffer);
      }
      else{
        char* filename = path+1; //skip the / and get the filename
        char* ext = strrchr(filename,'.'); //find extension by finding . last occurence
        char * content_type;
        if(ext!=NULL){
          if(strcmp(ext,".html")==0){
            content_type="text/html";
          }else if(strcmp(ext,".css")==0){
            content_type="text/css";
          }else if(strcmp(ext,".js")==0){
            content_type="application/javascript";
          }else{
            content_type= "text/plain";
          }
        }else{
          content_type = "text/plain";
        }
        FILE *file = fopen(filename,"r");
        if(file==NULL){
          //filename not in server so return page not found
          sprintf(response,
          "HTTP/1.1 404 Not Found\r\n"
          "Content-Type: text/plain\r\n"
          "\r\n"
          "Page not found");
        }
        else{
          char file_buffer[8192];
          int bytes = fread(file_buffer,1,sizeof(file_buffer),file);
          file_buffer[bytes] = '\0';
          fclose(file);
          sprintf(response,
          "HTTP/1.1 200 OK\r\n"
          "Content-Type: %s\r\n"
          "\r\n"
          "%s"
          ,content_type, file_buffer);
        }
      }

      send(client_socket,response,strlen(response),0);// 0 => flags , no special behaviour

      //Close sockets
      close(client_socket);
      exit(0);
    }
    
  }
  
  close(server_fd);

  return 0;
}