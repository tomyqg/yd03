#include "ite/itu.h"

extern void LogoReset(void);
extern void mainReset(void);

void resetScene(void)
{
    LogoReset();
    mainReset();
}
