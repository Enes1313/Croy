#ifndef SCKT_BASIC_COM_PROTOCOLS
#define SCKT_BASIC_COM_PROTOCOLS

#include "esSCKTBaseDef.h"

/**
 * @brief 
 * 
 * @param s 
 * @param text 
 * @param len 
 * @return true 
 * @return false 
 */
bool recverText(EASCKT s, char *text, unsigned int len);

/**
 * @brief 
 * 
 * @param s 
 * @param text 
 * @return true 
 * @return false 
 */
bool senderText(EASCKT s, const char *text);

/**
 * @brief 
 * 
 * @param s 
 * @return true 
 * @return false 
 */
bool recverFile(EASCKT s);

/**
 * @brief 
 * 
 * @param s 
 * @param path 
 * @return true 
 * @return false 
 */
bool senderFile(EASCKT s, const char *path);

#endif /* SCKT_BASIC_COM_PROTOCOLS  */
