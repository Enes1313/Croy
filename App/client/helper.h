#ifndef CLIENT_HELPER_H
#define CLIENT_HELPER_H

#include <stdbool.h>
#include <wchar.h>

/**
 * @brief 
 * 
 * @param pathOfProgram 
 * @return true 
 * @return false 
 */
bool isThePlaceToBe(const char *pathOfProgram);


/**
 * @brief 
 * 
 * @return true 
 * @return false 
 */
bool isTheSystemInfected(void);

/**
 * @brief 
 * 
 * @return const wchar_t* 
 */
const wchar_t *infectTheSystem(void);

/**
 * @brief 
 * 
 * @param pathOfProgram 
 */
void startProgram(const wchar_t *pathOfProgram);

/**
 * @brief 
 * 
 */
void connectToBigBrother(void);

#endif /* CLIENT_HELPER_H */
