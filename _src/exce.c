#include "exce.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include "environ.h"
#include "remote-terminal.h"

#define IS_LAST(list) (list->next == NULL)

int pid = 0;

void exceCommandList(command_list* list);
void exceCommandPipe(command_list* list);
bool exceBuildIn(command_list* list); 



void exceCommandBlock(command_block * block)
{
  if(!exceCommandIsDone())
    exceCommandWait();

  if(block->next == NULL)
    return;
  //リストを読み進める
  block = block->next;

  char* tmp = getcwd(NULL,0);
  if(strcmp(tmp,EnvironGet("PWD")) != 0)
  {
    chdir(EnvironGet("PWD"));
  }

  free(tmp);

  if(IS_LAST(block->command))
  {//コマンドが一つだけなら
    if(!exceBuildIn(block->command))
    {
      exceCommandList(block->command);
    }
  }
  else
  {
    exceCommandList(block->command);
  }
  
  //再起処理
  exceCommandBlock(block);
}



void exceCommandWait()
{
  if(!exceCommandIsDone())
  {
    int status;
    while(wait(&status) != pid);
  }
}

bool exceCommandIsDone()
{
  if(pid == 0 || kill(pid,0) != 0)
  {
    pid = 0;
    return true;
  }

  return false;
}


void exceCommandSendSigint()
{
  if(!exceCommandIsDone())
  {
    kill(pid,SIGINT);
  }
}





void exceCommandList(command_list* list)
{
  pid = fork();

  if(pid == -1)
  {
    fprintf(stderr,"Fork failed\n");
    return;
  }
  
  if(pid == 0)
  {
    if(list->next != NULL)
    {
      exceCommandPipe(list->next);
    }

    char ** args = TokenToArray(list->token);

    if(execvp(args[0],args) < 0)
    {
      printf("不明なコマンド '%s'\n",args[0]);
    }

    free(args);

    exit(EXIT_SUCCESS);
  }
}


void exceCommandPipe(command_list* list)
{
  int pipeFd[2];
  if(pipe(pipeFd) < 0)
  {
    fprintf(stderr,"Failed open pipe");
    return;
  }

  pid = fork();

  if(pid == -1)
  {
    fprintf(stderr,"Fork failed\n");
    return;
  }
  
  if(pid == 0)
  {
    //パイプ処理
    close(pipeFd[0]);
    dup2(pipeFd[1], STDOUT_FILENO);
    close(pipeFd[1]);
    pipeFd[1] = STDOUT_FILENO;
    

    if(list->next != NULL)
    {
      exceCommandPipe(list->next);
    }

    char ** args = TokenToArray(list->token);

    if(execvp(args[0],args) < 0)
    {
      printf("不明なコマンド '%s'\n",args[0]);
    }

    free(args);

    exit(EXIT_SUCCESS);
  }
  else
  {    
    //パイプ処理
    close(pipeFd[1]);
    dup2(pipeFd[0], STDIN_FILENO);
    close(pipeFd[0]);

    int status;
    while(wait(&status) != pid);
  }
}


bool exceBuildIn(command_list* list)
{
  char** arg = TokenToArray(list->token);

  if(strcmp(arg[0],"cd") == 0)
  {
    if(arg[1] == NULL)
    {
      chdir(EnvironGet("HOME"));
    }
    else
    {
      chdir(arg[1]);
    }

    char* tmp = getcwd(NULL,0);
    EnvironUpdate("PWD",tmp);
    free(tmp);

    free(arg);
    return true;
  }
  else if(strcmp(arg[0],"exit") == 0)
  {
    exit(EXIT_SUCCESS);
    return true;
  }

  free(arg);

  return false;
}