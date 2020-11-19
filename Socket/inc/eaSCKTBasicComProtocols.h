#ifndef _INC_SCKT_BASICCP
#define _INC_SCKT_BASICCP

#include "esSCKTBaseDef.h"

int recver(EAScktType, char * bytes, unsigned char max_len);
int sender(EAScktType, const char * bytes, unsigned char max_len);
char * recverText(EAScktType text);
void senderText(EAScktType, const char * path);
char * recverFile(EAScktType);
void senderFile(EAScktType, const char * path);

#endif /* _INC_SCKT_BASICCP  */