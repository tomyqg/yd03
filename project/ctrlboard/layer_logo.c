#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"
#include "global.h"

/* static ITUSprite* Sprite; */

bool LogoOnEnter(ITUWidget* widget, char* param)
{
    /* if (!Sprite) { */
        /* Sprite = ituSceneFindWidget(&theScene, "Sprite"); assert(Sprite); */
    /* } */
    return true;
}

bool logo_timer(ITUWidget* widget, char* param)
{
    /* ituSpriteGoto(Sprite, level0 % 10); */
    return false;
}

void LogoReset(void)
{
    /* Sprite = NULL; */
}
