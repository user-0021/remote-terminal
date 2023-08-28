#pragma once

#include <arpa/inet.h>

typedef struct _client_list
{
  int stdIn[2];
  int stdOut[2];
  int stdErr[2];
  int pid;

  struct sockaddr_in client;
  struct _client_list *next;
}client_list;