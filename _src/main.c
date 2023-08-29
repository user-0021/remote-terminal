#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <fcntl.h>

#include "udp.h"
#include "exce.h"
#include "environ.h"
#include "tokenizer.h"
#include "remote-terminal.h"

//clientList
client_list head = {};

//server
UDP_SERVER_HANDLE server;


void signal_handler_2(int signum);
void Callback(UDP_SERVER_HANDLE this,void* buff,size_t size,struct sockaddr_in from,void* userData);

void PrintLineHead();

int main(int argc,char** argv)
{
  if(argc < 2)
  {
    fprintf(stderr,"to few args\n");
    exit(EXIT_FAILURE); 
  }
  //sigintのバインド
  signal(SIGINT, signal_handler_2);

  struct in_addr serverIp;  

  if(inet_aton(argv[1],&serverIp) == 0)
  {//引数をホストネー厶として再試行する
    struct addrinfo hints, *info;
    
    memset( &hints, 0, sizeof(hints) );

    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(argv[1], NULL, &hints, &info ) != 0)
    {
      fprintf(stderr,"Invaled ip '%s'\n",argv[1]);
      exit(EXIT_FAILURE); 
    }

    serverIp = ((struct sockaddr_in *)(info->ai_addr))->sin_addr;
  }

  //bind server
  if(UdpServer_Create(&server,serverIp.s_addr,11302,Callback,NULL) != UDP_HANDLE_SUCCESS)
  {
    exit(EXIT_FAILURE);
  }


  //read buff
  char readStr[4096];
  ssize_t size;
  //loop work
  while (1)
  {
    //check udp port
    UdpServer_Loop(server);


    //check process output
    client_list * itr = head.next;
    while (itr != NULL)
    {
      if((size = read(itr->stdOut[0],readStr,sizeof(readStr))) > 0)
      {
        UdpServer_Send(server,itr->client,readStr,size);
      }


      if((size = read(itr->stdErr[0],readStr,sizeof(readStr))) > 0)
      {
        UdpServer_Send(server,itr->client,readStr,size);
      }

      itr = itr->next;
    }
    

  }

  return 0;
}


void signal_handler_2(int signum)
{
  CloseUdpServer(server);
  
  client_list * itr = head.next;
  while (itr != NULL)
  {
    close(itr->stdErr[0]);
    close(itr->stdOut[0]);
    close(itr->stdIn[1]);
    kill(itr->pid,SIGINT);
    usleep(100);
    kill(itr->pid,SIGKILL);

    itr = itr->next;
  }

  sleep(1);
  _exit(EXIT_SUCCESS);
}

void PrintLineHead()
{
  //頻繁にアクセスするポインタに参照用
  const char* pwd = EnvironGet("PWD");
  const char* home = EnvironGet("HOME");

  //ホストネーム取得
  char hostName[256];
  gethostname(hostName,sizeof(hostName));

  //PWDのコピー
  char* currentDir = (char*)malloc(strlen(pwd) + 1);
  strcpy(currentDir,pwd);

  //ホームパスの置換
  if(strncmp(currentDir,home,strlen(home)) == 0)
  {
    free(currentDir);
    currentDir = (char*)malloc(strlen(pwd) - strlen(home) + 2);
    strcpy(currentDir,"~");
    strcat(currentDir,pwd + strlen(home));
  }

  //入力受付のあれ
  printf("%s@%s:%s$ ",EnvironGet("USERNAME"),hostName,currentDir);
  fflush(stdout);

  free(currentDir);
}




void terminalProcess();



void Callback(UDP_CLIENT_HANDLE this,void* buff,size_t size,struct sockaddr_in from,void* userData)
{
  char* recvStr = ((char*)buff)+1;

  switch(((char*)buff)[0])
  {

    case 'I':
    {      
      client_list * itr = head.next;

      while (1)
      {
        if(itr == NULL)
        {
          char forkErr[] = "Failed find your terminal\n";
          UdpServer_Send(this,from,forkErr,sizeof(forkErr));
          UdpServer_Send(server,from,"D",1);//あえての1
          return;
        }

        //addr とport とfamilyのチェック
        if(itr->client.sin_addr.s_addr == from.sin_addr.s_addr && itr->client.sin_port == from.sin_port && itr->client.sin_family == from.sin_family)
          break;

        itr = itr->next;
      }

      kill(itr->pid,SIGINT);

      break;
    }
    case 'S':
    {      
      client_list * itr = head.next;

      while (1)
      {
        if(itr == NULL)
        {
          char forkErr[] = "Failed find your terminal\n";
          UdpServer_Send(this,from,forkErr,sizeof(forkErr));
          UdpServer_Send(server,from,"D",1);//あえての1
          return;
        }

        //addr とport とfamilyのチェック
        if(itr->client.sin_addr.s_addr == from.sin_addr.s_addr && itr->client.sin_port == from.sin_port && itr->client.sin_family == from.sin_family)
          break;

        itr = itr->next;
      }

      write(itr->stdIn[1],recvStr,size -1);

      break;
    }
    case 'D':
    {      
      client_list * itr = head.next;
      client_list * befor= &head; 

      while (1)
      {
        if(itr == NULL)
        {
          char forkErr[] = "Failed find your terminal\n";
          UdpServer_Send(this,from,forkErr,sizeof(forkErr));
          UdpServer_Send(server,from,"D",1);//あえての1
          return;
        }

        //addr とport とfamilyのチェック
        if(itr->client.sin_addr.s_addr == from.sin_addr.s_addr && itr->client.sin_port == from.sin_port && itr->client.sin_family == from.sin_family)
          break;

        befor = itr;
        itr = itr->next;
      }

      write(itr->stdIn[1],"exit\n",6);
      close(itr->stdErr[0]);
      close(itr->stdOut[0]);
      close(itr->stdIn[1]);
      UdpServer_Send(server,itr->client,"Terminal closed\n",17);
      UdpServer_Send(server,itr->client,"D",1);//あえての1

      befor->next = itr->next;
      free(itr);

      break;
    }
    case 'C':
    {//Connect
      int i = 1;
      client_list * itr = &head;

      while (itr->next != NULL)
      {
        itr = itr->next;
        i++;
      }
      
      itr->next = (client_list*)malloc(sizeof(client_list));
      memset(itr->next,0,size);
      client_list *data = itr->next;
      data->client = from;

      //pipeを生成
      if(pipe(data->stdErr) < 0 | pipe(data->stdOut) < 0 | pipe(data->stdIn) < 0)
      {
        close(data->stdErr[0]);
        close(data->stdOut[0]);
        close(data->stdIn[0]);
        close(data->stdErr[1]);
        close(data->stdOut[1]);
        close(data->stdIn[1]);

        free(data);
        itr->next = NULL;

        char pipeErr[] = "Failed open pipe\n";
        UdpServer_Send(this,from,pipeErr,sizeof(pipeErr));
        return;
      }

      //fork
      data->pid = fork();

      if(data->pid > 0)
      {//parent
        close(data->stdErr[1]);
        close(data->stdOut[1]);
        close(data->stdIn[0]);

        //パイプをnon-Blockingにする
        int flags;
        flags = fcntl(data->stdErr[0], F_GETFL, 0);
        fcntl(data->stdErr[0], F_SETFL, flags | O_NONBLOCK);

        flags = fcntl(data->stdOut[0], F_GETFL, 0);
        fcntl(data->stdOut[0], F_SETFL, flags | O_NONBLOCK);

        char res[100];
        sprintf(res,"Connection success now connection clients %d\n",i); 
        UdpServer_Send(this,from,res,strlen(res)+1);
      }
      else if (data->pid == 0)
      {//child       
        //パイプをつなぐ
        if (data->stdOut[1] != STDOUT_FILENO) 
        {
          dup2(data->stdOut[1], STDOUT_FILENO);
          close(data->stdOut[1]);
          data->stdOut[1] = STDOUT_FILENO;
			  }

        if (data->stdOut[1] != STDERR_FILENO) 
        {
          dup2(data->stdErr[1],STDERR_FILENO);
          close(data->stdErr[1]);
          data->stdErr[1] = STDERR_FILENO;
        }

        dup2(data->stdIn[0],STDIN_FILENO);
        close(data->stdIn[0]);

        close(data->stdErr[0]);
        close(data->stdOut[0]);
        close(data->stdIn[1]);

        terminalProcess();
        _exit(EXIT_SUCCESS);
      }
      else
      {
        close(data->stdErr[0]);
        close(data->stdOut[0]);
        close(data->stdIn[0]);
        close(data->stdErr[1]);
        close(data->stdOut[1]);
        close(data->stdIn[1]);

        free(data);
        itr->next = NULL;

        char forkErr[] = "Failed fork process\n";
        UdpServer_Send(this,from,forkErr,sizeof(forkErr));
        return;
      }

      break;
    }
    default:
      break;
  }
}

void signal_handler(int signum);

void terminalProcess()
{
  // sigint の　ハンドル
  signal(SIGINT, signal_handler);

  char* lineStr = (char*)malloc(4096);
  ssize_t buffSize;

  while (1)
  {
    PrintLineHead();

    command_block block;
    getline(&lineStr, &buffSize, stdin);
    
    if(CommandLineTokenize(lineStr,&block) == 0)
    {
      exceCommandBlock(&block);
      CommandListFree(&block);
    }
    
    exceCommandWait();
  } 
}

void signal_handler(int signum) 
{
  exceCommandSendSigint();
  if(exceCommandIsDone())
  {
    putchar('\n');
    PrintLineHead();
    fflush(stdout);
  }
}

