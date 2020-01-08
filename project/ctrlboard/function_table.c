#include "ite/itu.h"

extern bool LogoOnEnter(ITUWidget* widget, char* param);
extern bool logo_timer(ITUWidget* widget, char* param);

extern bool mainOnEnter(ITUWidget* widget, char* param);
extern bool main_timer(ITUWidget* widget, char* param);

ITUActionFunction actionFunctions[] =
{
    "LogoOnEnter", LogoOnEnter,
    "logo_timer", logo_timer,

    "mainOnEnter", mainOnEnter,
    "main_timer", main_timer,

    NULL, NULL
};
