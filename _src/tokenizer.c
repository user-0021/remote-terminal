#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DELIME_STR " \t\r\n\a"

int CommandLineTokenize(char* line,command_block* block)
{
  size_t command_count = 0;

  command_block* commandBlockItr = block;

  command_list  commandList = {NULL,NULL};
  command_list* commandListItr = &commandList;

  token_list token = {NULL,NULL}; 
  token_list * tokenItr = &token;

  char * left,* right;
  
  left = strtok(line , DELIME_STR);
  right = strtok(NULL, DELIME_STR);

  while (left != NULL) 
  {
    if(strcmp(left,"&&") == 0)
    {

      if(token.next == NULL)
      {
        fprintf(stderr,"予期しないトークン'&&'の周変に構文エラーがあります\n");
        break;
      }

      //コマンドブロクに要素を追加
      commandListItr = (command_list*)malloc(sizeof(command_list));
      commandListItr->token = token.next;
      commandListItr->next = commandList.next;
      commandList.next = commandListItr;

      //コマンドブロックに要素を追加して他を初期化
      commandBlockItr->next = (command_block*)malloc(sizeof(command_block));
      commandBlockItr = commandBlockItr->next;
      commandBlockItr->command = commandList.next;
      commandBlockItr->next = NULL;

      token.next = NULL;
      tokenItr = &token;

      commandList.next = NULL;
      commandListItr = & commandList;


      //トークンを一つ読み進める
      left = right;
      right = strtok(NULL, DELIME_STR);      

      continue;
    }
    else if(strcmp(left,"|") == 0)
    {
      if(token.next == NULL)
      {
        fprintf(stderr,"予期しないトークン'|'の周変に構文エラーがあります\n");
        break;
      }

      //blockListに要素を追加して他を初期化
      commandListItr = (command_list*)malloc(sizeof(command_list));
      commandListItr->token = token.next;
      commandListItr->next = commandList.next;
      commandList.next = commandListItr;
      
      token.next = NULL;
      tokenItr = &token;

      //トークンを一つ読み進める
      left = right;
      right = strtok(NULL, DELIME_STR);
      continue;      
    }

    //トークンリストに要素を追加
    tokenItr->next = (token_list*)malloc(sizeof(token_list));
    tokenItr = memset(tokenItr->next,0,sizeof(token_list));

    tokenItr->s = (char*)malloc(strlen(left) + 1);
    strcpy(tokenItr->s,left);

    //トークンを一つ読み進める
    left = right;
    right = strtok(NULL, DELIME_STR);
  }

  if(token.next == NULL)
  {
    return -1;
  }
  
  commandListItr = (command_list*)malloc(sizeof(command_list));
  commandListItr->token = token.next;
  commandListItr->next = commandList.next;
  commandList.next = commandListItr;

  commandBlockItr->next = (command_block*)malloc(sizeof(command_block));
  commandBlockItr = commandBlockItr->next;
  commandBlockItr->command = commandList.next;
  commandBlockItr->next = NULL;
  
  return 0;
}


void CommandListFree(command_block* block)
{
  command_block* blockItr = block->next;

  //ブロックの線形探査
  while (blockItr != NULL)
  {
    command_list* listItr = blockItr->command;

    //リストの線形探査
    while(listItr != NULL)
    {
      token_list * tokenItr = listItr->token;

      //トークンの線形探査
      while(tokenItr != NULL)
      {
        token_list * tokenTmp = tokenItr;
        tokenItr = tokenItr->next;

        free(tokenTmp->s);
        free(tokenTmp);
      }
      
      command_list* listTmp = listItr;
      listItr = listItr->next;      
      free(listTmp);
    }

    command_block* blockTmp = blockItr;
    blockItr = blockItr->next;
    free(blockTmp);
  }
}

char** TokenToArray(token_list* token)
{
  size_t tokenNum = 0;
  token_list* itr = token;

  while (itr != NULL)
  {
    itr = itr->next;
    tokenNum++;
  }

  char** args =  (char**)malloc((tokenNum+1)*sizeof(char*));
  memset(args,0,(tokenNum+1)*sizeof(char*));

  itr = token;
  int i;
  for(i = 0;i < tokenNum;i++)
  {
    args[i] = itr->s;
    itr = itr->next;
  }  

  return args;
}
