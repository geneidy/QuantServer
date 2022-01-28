/* 
 * This file is part of the QuantServer (https://github.com/geneidy/QuantServer).
 * Copyright (c) 2017 geneidy.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "Distributor.h"
#include "NQTV.h"

uint64_t CDistributor::m_ui64WriteSequence = 0; // Last Message number written

CDistributor::CDistributor()
{
      m_iError = 0;
      m_iHandle = open("QuanticksDistributor.Qtx", O_CREAT | O_APPEND, S_IRWXG); // -1 is returned on error

      if (m_iHandle == -1)
      {
            m_iError = m_iHandle;
            // check for error number and log
            /*
  switch (m_iHandle){
   
#define	EPERM		 1	 Operation not permitted 
#define	ENOENT		 2	 No such file or directory 
#define	ESRCH		 3	 No such process 
#define	EINTR		 4	 Interrupted system call 
#define	EIO		 5	 I/O error 
#define	ENXIO		 6	 No such device or address 
#define	E2BIG		 7	 Argument list too long 
#define	ENOEXEC		 8	 Exec format error 
#define	EBADF		 9	 Bad file number 
#define	ECHILD		10	 No child processes 
#define	EAGAIN		11	 Try again 
#define	ENOMEM		12	 Out of memory 
#define	EACCES		13	 Permission denied 
#define	EFAULT		14	 Bad address 
#define	ENOTBLK		15	 Block device required 
#define	EBUSY		16	 Device or resource busy 
#define	EEXIST		17	 File exists 
#define	EXDEV		18	 Cross-device link 
#define	ENODEV		19	 No such device 
#define	ENOTDIR		20	 Not a directory 
#define	EISDIR		21	 Is a directory 
#define	EINVAL		22	 Invalid argument 
#define	ENFILE		23	 File table overflow 
#define	EMFILE		24	 Too many open files 
#define	ENOTTY		25	 Not a typewriter 
#define	ETXTBSY		26	 Text file busy 
#define	EFBIG		27	 File too large 
#define	ENOSPC		28	 No space left on device 
#define	ESPIPE		29	 Illegal seek 
#define	EROFS		30	 Read-only file system 
#define	EMLINK		31	 Too many links 
#define	EPIPE		32	 Broken pipe 
#define	EDOM		33	 Math argument out of domain of func 
#define	ERANGE		34	 Math result not representable 
*/
      }
      m_iSizeOfMessage = sizeof(QT_ITCH_MESSAGE);
}
/////////////////////////////////////////////////////////////////
CDistributor::~CDistributor()
{
}
//////////////////////////////////////////////////////////////////////////////////////////
uint64_t CDistributor::GetNumberOfMessagesInFile()
{
      // later on...

      return 1;
}
//////////////////////////////////////////////////////////////////////////////////////////
uint64_t CDistributor::GetLastSequence()
{

      return m_ui64WriteSequence;
}
//////////////////////////////////////////////////////////////////////////////////////////
int CDistributor::WriteFeedToFile(char cMessageType, ITCH_MESSAGES_UNION MessageToWrite)
{
      // File format : One struct containing the sequence, messagetype and then the Union

      memset(&m_DistMessage, '\0', m_iSizeOfMessage);

      m_DistMessage.cMessageType = cMessageType;
      m_DistMessage.uiMessageSequence = m_ui64WriteSequence++;
      m_DistMessage.uiTCH_MESSAGE = MessageToWrite;

      //  write(m_iHandle, (void*)(&m_DistMessage), sizeof(QT_ITCH_MESSAGE));
      ssize_t iRet = write(m_iHandle, (void *)(&m_DistMessage), m_iSizeOfMessage);
      if (iRet == -1)
      {
            m_iError = 1;
            switch (errno)
            {
            case EBADF:
                  m_strError = "fd is not an open file descriptor";
                  break;
            case EFAULT:
                  m_strError = "buf is outside your accessible address space.";
                  break;
            case EFBIG:
                  m_strError = "An attempt was made to write a file that exceeds the implementation-defined maximum file size or the process's file size limit, or to write at a position past the maximum allowed offset.";
                  break;
            case EINVAL:
                  m_strError = "fd is attached to an object which is unsuitable for writing; or the file was opened with the O_DIRECT flag, and either the address specified in buf, the value specified in count, or the current file offset is not suitably aligned.";
                  break;
            case EIO:
                  m_strError = "A low-level I/O error occurred while modifying the inode.";
                  break;
            default:
                  m_strError = "Unknown Error";
                  break;
            }
      }

      return 1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QT_ITCH_MESSAGE CDistributor::GetMessageBySequence(uint64_t uiMessageSequence)
{

      QT_ITCH_MESSAGE Return_ITCH_MESSAGE;
      ssize_t iRet = 0;
      off_t oRet = lseek64(m_iHandle, uiMessageSequence * m_iSizeOfMessage, SEEK_SET);

      if (oRet == -1)
      {
            switch (errno)
            {
                  m_iError = 1;
            case EBADF:
                  m_strError = "fd is not an open file descriptor";
                  break;
            case EINVAL:
                  m_strError = "whence is not valid. Or: the resulting file offset would be negative, or beyond the end of a seekable device.";
                  break;
            case EOVERFLOW:
                  m_strError = "The resulting file offset cannot be represented in an off_t.";
                  break;
            case ESPIPE:
                  m_strError = "fd is associated with a pipe, socket, or FIFO.";
                  break;
            case ENXIO:
                  m_strError = "Whence is SEEK_DATA or SEEK_HOLE, and the current file offset is beyond the end of the file.";
                  break;
            default:
                  break;
            }
      }

      iRet = read(m_iHandle, &Return_ITCH_MESSAGE, m_iSizeOfMessage);

      if (iRet == -1)
      {
            m_strError.empty();
            switch (errno)
            {
                  m_iError = 1;
            case EBADF:
                  m_strError = "fd is not a valid file descriptor or is not open for reading";
                  break;
            case EFAULT:
                  m_strError = "buf is outside your accessible address space";
                  break;
            case EINTR:
                  m_strError = "The call was interrupted by a signal before any data was read";
                  break;
            case EINVAL:
                  m_strError = "fd is attached to an object which is unsuitable for reading; or the file was opened with the O_DIRECT flag, and either the address specified in buf, the value specified in count, or the current file offset is not suitably aligned";
                  break;
            case EIO:
                  m_strError = "I/O error. This will happen for example when the process is in a background process group, tries to read from its controlling terminal, and either it is ignoring or blocking SIGTTIN or its process group is orphaned. It may also occur when there is a low-level I/O error while reading from a disk or tape.";
                  break;
            default:
                  break;
            }
      }
      return Return_ITCH_MESSAGE;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
string CDistributor::GetErrorString()
{

      m_iError = 0;
      return m_strError;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
