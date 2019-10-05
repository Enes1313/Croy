#ifndef SOCKET_INC_PROCESS_H_
#define SOCKET_INC_PROCESS_H_

#include <windows.h>

void processCMD(SOCKET sckt);
void processFileUpload(SOCKET sckt, char * path);
void processFileDownload(SOCKET sckt, char * path);
void processIpPort(char * ipPort);
void processRecvSendText(SOCKET sckt);

#endif /* SOCKET_INC_PROCESS_H_ */
