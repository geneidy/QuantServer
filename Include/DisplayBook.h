#pragma once
#include "BuildBook.h"



class DisplayBook{
  
  int m_iError;
  
  
  CBuildBook* pcBuildBook;
  
public:
  DisplayBook();
  ~DisplayBook();
  int GetError();
  char* GetErrorDescription();
  
};


