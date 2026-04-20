#ifndef STORAGE_H
#define STORAGE_H

#include <pthread.h>
#include "config.h"

struct User {
  int id;
  char data[USER_DATA_SIZE];
};

extern struct User storage[STORAGE_SIZE];
extern int total_data;
extern int next_id;
extern pthread_mutex_t lock;

int add_data(char* data);
int get_all(char* buffer);
int get_by_id(int id, char* buffer);
int delete_by_id(int id);
void delete_all();
int update_by_id(int id, char* data);

#endif