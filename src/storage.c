#include "include/storage.h"
#include "include/config.h"
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct User storage[STORAGE_SIZE];
int total_data = 0;
int next_id = 1;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int add_data(char* data) {
  if (total_data >= STORAGE_SIZE) {
    return -1;
  }
  
  pthread_mutex_lock(&lock);
  storage[total_data].id = next_id;
  strcpy(storage[total_data].data, data);
  int id = next_id;
  total_data++;
  next_id++;
  pthread_mutex_unlock(&lock);
  
  return id;
}

int get_all(char* buffer) {
  pthread_mutex_lock(&lock);
  
  if (total_data == 0) {
    strcpy(buffer, "No data available");
  } else {
    buffer[0] = '\0';
    for (int i = 0; i < total_data; i++) {
      char line[BODY_LINE_SIZE];
      sprintf(line, "%d : %s\n", storage[i].id, storage[i].data);
      strcat(buffer, line);
    }
  }
  
  pthread_mutex_unlock(&lock);
  return total_data;
}

int get_by_id(int id, char* buffer) {
  pthread_mutex_lock(&lock);
  
  int foundid = -1;
  for (int i = 0; i < total_data; i++) {
    if (storage[i].id == id) {
      foundid = i;
      break;
    }
  }
  
  if (foundid == -1) {
    pthread_mutex_unlock(&lock);
    return -1;
  }
  
  sprintf(buffer, "%d : %s", storage[foundid].id, storage[foundid].data);
  pthread_mutex_unlock(&lock);
  return foundid;
}

int delete_by_id(int id) {
  pthread_mutex_lock(&lock);
  
  int foundid = -1;
  for (int i = 0; i < total_data; i++) {
    if (storage[i].id == id) {
      foundid = i;
      break;
    }
  }
  
  if (foundid == -1) {
    pthread_mutex_unlock(&lock);
    return -1;
  }
  
  for (int i = foundid; i < total_data - 1; i++) {
    storage[i] = storage[i + 1];
  }
  total_data--;
  
  pthread_mutex_unlock(&lock);
  return 0;
}

void delete_all() {
  pthread_mutex_lock(&lock);
  total_data = 0;
  pthread_mutex_unlock(&lock);
}

int update_by_id(int id, char* data) {
  pthread_mutex_lock(&lock);
  
  int foundid = -1;
  for (int i = 0; i < total_data; i++) {
    if (storage[i].id == id) {
      foundid = i;
      break;
    }
  }
  
  if (foundid == -1) {
    pthread_mutex_unlock(&lock);
    return -1;
  }
  
  strcpy(storage[foundid].data, data);
  pthread_mutex_unlock(&lock);
  return 0;
}