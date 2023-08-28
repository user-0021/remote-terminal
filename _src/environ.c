#include "environ.h"

#include <string.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <sys/mman.h>
#include <linux/limits.h>

typedef struct 
{
  char * element;
  int equalPos;
} EnvironElement;


EnvironElement** user_environ = NULL;

void EnvironSet();
size_t EnvironGetKeyPos(const char* const key);

int EnvironUpdate(const char* const key,const char* const value)
{
  if(user_environ == NULL)
    EnvironSet();

  size_t keyPos = EnvironGetKeyPos(key);
  if(user_environ[keyPos]->element == NULL)
  {
    return -1;
  }
  
  user_environ[keyPos]->element = (char*)realloc(user_environ[keyPos]->element,strlen(key) + user_environ[keyPos]->equalPos + 2);
  strcpy(user_environ[keyPos]->element,key);
  strcat(user_environ[keyPos]->element,"=");
  strcat(user_environ[keyPos]->element,value);

  return 0;
}


const char* EnvironGet(const char* const key)
{
  if(user_environ == NULL)
    EnvironSet();

  size_t keyPos = EnvironGetKeyPos(key);
  if(user_environ[keyPos]->element == NULL)
  {
    return NULL;
  }
  
  return user_environ[keyPos]->element + user_environ[keyPos]->equalPos + 1;
}


void EnvironSet()
{ 
  size_t environCount;//enviromentの個数を取得
  for(environCount = 0 ; __environ[environCount] != NULL ; environCount++);

  user_environ = (EnvironElement**)malloc((environCount + 1) * sizeof(EnvironElement*));
  memset(user_environ,0,(environCount + 1) * sizeof(EnvironElement*));
  
  size_t i;
  for(i = 0;i < environCount;i++)
  {//コピー
    user_environ[i] = (EnvironElement*)malloc(sizeof(EnvironElement));
    user_environ[i]->element = (char*)malloc(strlen(__environ[i])+1);
    strcpy(user_environ[i]->element,__environ[i]);

    user_environ[i]->equalPos = strchr(__environ[i],'=') - __environ[i];
  }
}


size_t EnvironGetKeyPos(const char* const key)
{
  size_t i;
  for(i = 0 ; user_environ[i]->element != NULL ; i++)
  {
    if(strncmp(key,user_environ[i]->element,user_environ[i]->equalPos) == 0)
    {
      break;
    }
  }

  return i;
}