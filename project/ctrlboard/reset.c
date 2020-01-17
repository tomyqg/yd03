#include "ite/itu.h"

extern void LogoReset(void);
extern void mainReset(void);
extern void setupReset(void);
extern void runReset(void);

void resetScene(void)
{
    LogoReset();
    mainReset();
    setupReset();
    runReset();
}
