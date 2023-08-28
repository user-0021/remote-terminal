#pragma once
#include <stdbool.h>

typedef struct _token_list
{
  char* s;
  
  struct _token_list* next;
} token_list;

typedef struct _command_list
{
  token_list* token;

  struct _command_list* next;
} command_list;

typedef struct _command_block
{
  command_list* command;

  struct _command_block* next;
} command_block;

/**
 * @brief tokenize command list
 * 
 * @param [in] line command line 
 * @param [in,out] blockcommand block head
 * @return int err -1 , success 0
 */
int CommandLineTokenize(char * line,command_block* block);

/**
 * @brief free command list
 * 
 * @param [in] block head
 */
void CommandListFree(command_block* block);

/**
 * @brief 
 * 
 * @param token 
 * @return char** 
 */
char** TokenToArray(token_list* token);