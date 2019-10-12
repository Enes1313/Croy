#ifndef SOCKET_INC_PROCESS_H_
#define SOCKET_INC_PROCESS_H_

#include <windows.h>

void processCMD(int sckt);
void processFileUpload(int sckt, char * path);
void processFileDownload(int sckt, char * path);
void processIpPort(char * ipPort);
void processRecvSendText(int sckt);

#endif /* SOCKET_INC_PROCESS_H_ */
