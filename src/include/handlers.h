#ifndef HANDLERS_H
#define HANDLERS_H

extern int status_code;
extern int file;
extern long file_size;
extern char* file_buffer;

void handle_get(char* path, char* response, char* conn_header);
void handle_post(char* path, char* response, char* conn_header, char* post_data);
void handle_delete(char* path, char* response, char* conn_header);
void handle_put(char* path, char* response, char* conn_header, char* post_data);

#endif