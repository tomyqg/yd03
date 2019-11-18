#include "ite/itu.h"

extern bool LogoOnEnter(ITUWidget* widget, char* param);

ITUActionFunction actionFunctions[] =
{
    "LogoOnEnter", LogoOnEnter,
    NULL, NULL
};
