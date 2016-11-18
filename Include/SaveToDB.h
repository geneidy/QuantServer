#pragma once
#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>

#include <pthread.h>
#include <fcntl.h>
#include <errno.h>

#include "ITCHMessages.h"


class CSaveToDB
{
public:
  CSaveToDB();
  
  ~CSaveToDB();
  
  int m_iError;
  int GetError();
   
};