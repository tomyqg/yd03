#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"
#include "global.h"

static ITUSprite* Sprite;

bool LogoOnEnter(ITUWidget* widget, char* param)
{
    Sprite = ituSceneFindWidget(&theScene, "Sprite"); assert(Sprite);
    return true;
}

bool logo_timer(ITUWidget* widget, char* param)
{
    int value = level0;
    if (value < 10) ituSpriteGoto(Sprite, value);
    return true;
}

void LogoReset(void)
{
}
