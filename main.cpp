#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include <errno.h>

#include "main.h"
#include "NQTVDlg.h"
#include "NQTV.h"

#define HOST_NAME NULL //default = localhost
#define USER_NAME "amro"//NULL
#define PASSWORD "amro"//NULL
#define PORT_NUM 0 //use default port
#define SOCKET_NAME NULL //use default socket name
#define DB_NAME "QuanticksSchema"//NULL


// We're including defines here...need to convert to my.cnf + *groups[]
MYSQL* conn; /* pointer to connection handler. ima leave this here for now. where should we place this declaration?
			  * should we keep it in this scope? */

int main(int argc, char **argv)
{
  
  if (argc < 3)
  {
      std::cout << "Usage: "<< std::endl;
      std::cout << "QuantServer  <Valid_ITCH50_FileName>  <Buffer Size in Bytes>" << std::endl;
      std::cout << std::endl;
      std::cout << "Example:" << std::endl; 
      std::cout << "QuantServer /user/local/InputFileName  1000000" << std::endl; 
      return 1;
  }
  
  std::cout << "Hit 'S' or 's' to stop" << std::endl; 
  
  theApp.strFeedFileName = argv[1];  	//  ::TODO add more checks here
  int  iHandle = -1;
  
  if ((iHandle = open(theApp.strFeedFileName.c_str(), O_RDWR )==  -1))  
  {
      std::cout << "Invalid File Name Entered" << std::endl; 
      return 1;
  }

  theApp.dwBufferSize = atol(argv[2]);	//  ::TODO add more checks here
  if (theApp.dwBufferSize < 1000000)
    theApp.dwBufferSize = 1000000;
  
  const char *ThreadMessage = "NQTV Thread";
  pthread_t	NQTVThread;
  int iRet = pthread_create(&NQTVThread, NULL, NQTVFunction, (void*) ThreadMessage);
   if(iRet)
   {
      fprintf(stderr,"Error - pthread_create() return code: %d\n", iRet);
      exit(EXIT_FAILURE);
   }
   
  char cResp;
  while ( cResp != 'q')
  {
    std::cin >> cResp;
    if ((cResp == 's') || (cResp == 'S'))
    {
	theApp.g_bReceiving = false;
	break;
    }	
     
    if ((cResp == 'p') || (cResp == 'P'))
    {
	theApp.g_bReceiving = PAUSSED;
	continue;
    }	
   
  }

  return 0;
}
///////////////////////////////////////////////////////////////
void *NQTVFunction(void* ptr)
{

  CNQTVDlg* pCNQTVDlg  = NULL;
  pCNQTVDlg =  new CNQTVDlg(); // All the data needed by this object is Global in "theApp" Object....sorry...Inheritted from the wondows code
  
  //int iRet = pCNQTVDlg->ConnectMySql();  // check for return erro code before continuing
  conn = pCNQTVDlg->ConnectMySql(HOST_NAME, USER_NAME, PASSWORD, 
								 DB_NAME, PORT_NUM, SOCKET_NAME, 0);
  if (conn == NULL)
  {
	  printf("Connection Failed\n");
	  theApp.g_bConnected = false;
	  if (pCNQTVDlg)
	  {
		  delete pCNQTVDlg;
	  }
	  return NULL;
  } 
  else
  {
	  printf("Connection: Success!\n");  
	  theApp.g_bConnected = true;
  }  
  pCNQTVDlg->m_bTestFeed = true;
  
  pCNQTVDlg->FeedStart();   // Entry point to processing
  
  pCNQTVDlg->Disconnect1MySql();
  
    if (pCNQTVDlg)
      delete (pCNQTVDlg);
    
    return NULL;
 
}
///////////////////////////////////////////////////////////////


