////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Logger.cpp
#include <stdexcept>
#include "Logger.h"
#include "Util.h"
#include "time.h"
#include "string.h"

using namespace std;

const string Logger::Debug = "[D]";
const string Logger::Info  = "[I]";
const string Logger::Error = "[E]";

const char* const  Logger::kLogFileName = {""};

Logger* Logger::pInstance = nullptr;
mutex 	Logger::sMutex;
////////////////////////////////////////////////////////////////////
Logger& Logger::instance()
{
    static Cleanup cleanup;

    lock_guard<mutex> guard(sMutex);
    if (pInstance == nullptr)
        pInstance = new Logger();
    return *pInstance;
}
////////////////////////////////////////////////////////////////////
Logger::Cleanup::~Cleanup()
{
    lock_guard<mutex> guard(Logger::sMutex);
    delete Logger::pInstance;
    Logger::pInstance = nullptr;
}
////////////////////////////////////////////////////////////////////
Logger::~Logger()
{
    mOutputStream.close();
}
////////////////////////////////////////////////////////////////////
Logger::Logger()
{
    CUtil Util;
    std::string strkLogFileName;
    strkLogFileName.empty();
    
//     char *pPath;
//      
//      pPath = getenv("PATH");
//      if (pPath != NULL){
// 
//     }

    struct stat st = {0};
    if (stat("../Logs/", &st) == -1) {
        mkdir("../Logs/", 0700);
    }

    strkLogFileName = "../Logs/"; //TODO fix relative path...
    strkLogFileName += Util.GetFormatedDate();
    strkLogFileName += "Logs.txt";

    mOutputStream.open(strkLogFileName, std::fstream::in | std::fstream::out |std::fstream::app);
    if (!mOutputStream.good()) {
        throw runtime_error("Unable to initialize the Logger!");
    }
}
////////////////////////////////////////////////////////////////////
void Logger::log(const string& inMessage, const string& inLogLevel)
{
    lock_guard<mutex> guard(sMutex);
    logHelper(inMessage, inLogLevel);
}
////////////////////////////////////////////////////////////////////
void Logger::log(const vector<string>& inMessages, const string& inLogLevel)
{
    lock_guard<mutex> guard(sMutex);
    for (size_t i = 0; i < inMessages.size(); i++) {
        logHelper(inMessages[i], inLogLevel);
    }
}
/////////////////////////////////////////////////////////////////////////////////////
void Logger::logHelper(const std::string& inMessage, const std::string& inLogLevel)
{
    // Add logger time in the first column !!!
    time_t ltime = 0;
    struct tm stToday;

    char	szLogTime[26];
    time( &ltime );
    localtime_r( &ltime ,  &stToday);

    memset(szLogTime, 0 , 26);
    strftime(szLogTime, 26, "%Y-%m-%d- %H:%M:%S" , &stToday);
    mOutputStream << szLogTime << ": " << inLogLevel << " : " << inMessage << endl;
}
////////////////////////////////////////////////////////////////////////////////////////
