#include "udp.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

typedef struct
{
  int socket;
  char buff[2048];

  UDP_CALLBACK callback;
  void* userData;
} UdpServerTable;

typedef struct
{
  struct sockaddr_in serverAddr;
  int socket;
  char buff[2048];

  UDP_CALLBACK callback;
  void* userData;
} UdpClientTable;


UDP_HANDLE_STATUS UdpServer_Create(UDP_SERVER_HANDLE* handle,in_addr_t ip,uint16_t port,UDP_CALLBACK callback,void* userData)
{
  UdpServerTable* table = (UdpServerTable*)malloc(sizeof(UdpServerTable));

  table->callback = callback;
  table->userData = userData;

  struct sockaddr_in addr;

  //open socket fd
  if((table->socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  {
    free(table);
    return UDP_HANDLE_FAILED_SOCKET;
  }

  //bind address
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = ip;

  if(bind(table->socket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
  {
    free(table);
    return UDP_HANDLE_FAILED_BIND;
  }

  //set non blocking
  unsigned long val = 1;
  ioctl(table->socket, FIONBIO, &val);

  //return handle
  (*handle) = table;
  

  return UDP_HANDLE_SUCCESS;
}

void UdpServer_Loop(UDP_SERVER_HANDLE handle)
{
  UdpServerTable* table = handle;
  struct sockaddr_in addr;
  int addrSize = sizeof(addr);
  int size;

  
  if((size = recvfrom(table->socket,table->buff,sizeof(table->buff),0,(struct sockaddr *)&addr, &addrSize)) > 0)
  {
    table->callback(table,table->buff,size,addr,table->userData);
  }
}

ssize_t UdpServer_Send(UDP_SERVER_HANDLE handle,struct sockaddr_in addr,void* buff,size_t size)
{
  UdpServerTable* table = handle;
  sendto(table->socket,buff,size, 0, (struct sockaddr *)&addr, sizeof(addr));
}

void CloseUdpServer(UDP_SERVER_HANDLE handle)
{
  UdpServerTable* table = handle;

  //close
  close(table->socket);

  //free
  free(table);
}



UDP_HANDLE_STATUS UdpClient_Create(UDP_CLIENT_HANDLE* handle,in_addr_t ip,uint16_t port,UDP_CALLBACK callback,void* userData)
{
  UdpClientTable* table = (UdpClientTable*)malloc(sizeof(UdpClientTable));

  table->callback = callback;
  table->userData = userData;

  //open socket fd
  if((table->socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  {
    free(table);
    return UDP_HANDLE_FAILED_SOCKET;
  }

  //bind address
  table->serverAddr.sin_family = AF_INET;
  table->serverAddr.sin_port = htons(port);
  table->serverAddr.sin_addr.s_addr = ip;


  //set non blocking
  unsigned long val = 1;
  ioctl(table->socket, FIONBIO, &val);

  //return handle
  (*handle) = table;
  

  return UDP_HANDLE_SUCCESS;
}


void UdpClient_Loop(UDP_CLIENT_HANDLE handle)
{  
  UdpClientTable* table = handle;
  struct sockaddr_in addr;
  int addrSize = sizeof(addr);
  int size;

  
  if((size = recvfrom(table->socket,table->buff,sizeof(table->buff),0,(struct sockaddr *)&addr, &addrSize)) > 0)
  {
    table->callback(table,table->buff,size,addr,table->userData);
  }
}

ssize_t UdpClient_Send(UDP_CLIENT_HANDLE handle,void* buff,size_t size)
{
  UdpClientTable* table = handle;

  sendto(table->socket,buff,size, 0, (struct sockaddr *)&table->serverAddr, sizeof(table->serverAddr));
}

void CloseUdpClient(UDP_CLIENT_HANDLE handle)
{
  UdpClientTable* table = handle;

  //close
  close(table->socket);

  //free
  free(table);
}