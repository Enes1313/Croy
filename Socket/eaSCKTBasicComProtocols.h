#ifndef SCKT_BASIC_COM_PROTOCOLS
#define SCKT_BASIC_COM_PROTOCOLS

#include <stdbool.h>

#include "esSCKTBaseDef.h"

bool recverText(EAScktType s, char *text, unsigned int len);

bool senderText(EAScktType s, const char *text);

bool recverFile(EAScktType s);

bool senderFile(EAScktType s, const char *path);

#endif /* SCKT_BASIC_COM_PROTOCOLS  */
