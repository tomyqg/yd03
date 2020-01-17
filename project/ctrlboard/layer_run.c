#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"
#include "global.h"

static void piont_set(ITUAnimation* animation)
{
    animation->totalframe = rand() % 150 + 100; \
    animation->frame = animation->totalframe / 2; \
}

static void piont_show(ITUAnimation* animation, ITUIcon* icon)
{
    int x, y;
    if (animation->frame) return;
    x = rand() % 740 + 30; y = rand() % 290 + 150;
    /* if (x > 200 && x < 600) x = x > 400 ? 770 : 30; */
    /* if (y >  40 && y < 440) y = y > 240 ? 460 : 200; */
    ituWidgetSetX(icon, x); ituWidgetSetY(icon, y);
    animation->totalframe = rand() % 150 + 100;
}

static ITUText* runText;
static ITUAnimation* randAnimation1;
static ITUAnimation* randAnimation2;
static ITUAnimation* randAnimation3;
static ITUAnimation* randAnimation4;
static ITUAnimation* randAnimation5;
static ITUAnimation* randAnimation6;
static ITUAnimation* randAnimation7;
static ITUAnimation* randAnimation8;
static ITUAnimation* randAnimation9;
static ITUAnimation* randAnimation0;
static ITUAnimation* randAnimation10;
static ITUAnimation* randAnimation11;
static ITUAnimation* randAnimation12;
static ITUAnimation* randAnimation13;
static ITUAnimation* randAnimation14;
static ITUAnimation* randAnimation15;
static ITUAnimation* randAnimation16;
static ITUAnimation* randAnimation17;
static ITUAnimation* randAnimation18;
static ITUAnimation* randAnimation19;
static ITUIcon* randIcon1;
static ITUIcon* randIcon2;
static ITUIcon* randIcon3;
static ITUIcon* randIcon4;
static ITUIcon* randIcon5;
static ITUIcon* randIcon6;
static ITUIcon* randIcon7;
static ITUIcon* randIcon8;
static ITUIcon* randIcon9;
static ITUIcon* randIcon0;
static ITUIcon* randIcon10;
static ITUIcon* randIcon11;
static ITUIcon* randIcon12;
static ITUIcon* randIcon13;
static ITUIcon* randIcon14;
static ITUIcon* randIcon15;
static ITUIcon* randIcon16;
static ITUIcon* randIcon17;
static ITUIcon* randIcon18;
static ITUIcon* randIcon19;

static void rand_show(void)
{
    piont_show(randAnimation1, randIcon1);
    piont_show(randAnimation2, randIcon2);
    piont_show(randAnimation3, randIcon3);
    piont_show(randAnimation4, randIcon4);
    piont_show(randAnimation5, randIcon5);
    piont_show(randAnimation6, randIcon6);
    piont_show(randAnimation7, randIcon7);
    piont_show(randAnimation8, randIcon8);
    piont_show(randAnimation9, randIcon9);
    piont_show(randAnimation0, randIcon0);
    piont_show(randAnimation10, randIcon10);
    piont_show(randAnimation11, randIcon11);
    piont_show(randAnimation12, randIcon12);
    piont_show(randAnimation13, randIcon13);
    piont_show(randAnimation14, randIcon14);
    piont_show(randAnimation15, randIcon15);
    piont_show(randAnimation16, randIcon16);
    piont_show(randAnimation17, randIcon17);
    piont_show(randAnimation18, randIcon18);
    piont_show(randAnimation19, randIcon19);
}

bool runOnEnter(ITUWidget* widget, char* param)
{
    if (!runText) {
        runText = ituSceneFindWidget(&theScene, "runText"); assert(runText);
        randAnimation1 = ituSceneFindWidget(&theScene, "randAnimation1"); assert(randAnimation1);
        randAnimation2 = ituSceneFindWidget(&theScene, "randAnimation2"); assert(randAnimation2);
        randAnimation3 = ituSceneFindWidget(&theScene, "randAnimation3"); assert(randAnimation3);
        randAnimation4 = ituSceneFindWidget(&theScene, "randAnimation4"); assert(randAnimation4);
        randAnimation5 = ituSceneFindWidget(&theScene, "randAnimation5"); assert(randAnimation5);
        randAnimation6 = ituSceneFindWidget(&theScene, "randAnimation6"); assert(randAnimation6);
        randAnimation7 = ituSceneFindWidget(&theScene, "randAnimation7"); assert(randAnimation7);
        randAnimation8 = ituSceneFindWidget(&theScene, "randAnimation8"); assert(randAnimation8);
        randAnimation9 = ituSceneFindWidget(&theScene, "randAnimation9"); assert(randAnimation9);
        randAnimation0 = ituSceneFindWidget(&theScene, "randAnimation0"); assert(randAnimation0);
        randAnimation10 = ituSceneFindWidget(&theScene, "randAnimation10"); assert(randAnimation10);
        randAnimation11 = ituSceneFindWidget(&theScene, "randAnimation11"); assert(randAnimation11);
        randAnimation12 = ituSceneFindWidget(&theScene, "randAnimation12"); assert(randAnimation12);
        randAnimation13 = ituSceneFindWidget(&theScene, "randAnimation13"); assert(randAnimation13);
        randAnimation14 = ituSceneFindWidget(&theScene, "randAnimation14"); assert(randAnimation14);
        randAnimation15 = ituSceneFindWidget(&theScene, "randAnimation15"); assert(randAnimation15);
        randAnimation16 = ituSceneFindWidget(&theScene, "randAnimation16"); assert(randAnimation16);
        randAnimation17 = ituSceneFindWidget(&theScene, "randAnimation17"); assert(randAnimation17);
        randAnimation18 = ituSceneFindWidget(&theScene, "randAnimation18"); assert(randAnimation18);
        randAnimation19 = ituSceneFindWidget(&theScene, "randAnimation19"); assert(randAnimation19);
        randIcon1 = ituSceneFindWidget(&theScene, "randIcon1"); assert(randIcon1);
        randIcon2 = ituSceneFindWidget(&theScene, "randIcon2"); assert(randIcon2);
        randIcon3 = ituSceneFindWidget(&theScene, "randIcon3"); assert(randIcon3);
        randIcon4 = ituSceneFindWidget(&theScene, "randIcon4"); assert(randIcon4);
        randIcon5 = ituSceneFindWidget(&theScene, "randIcon5"); assert(randIcon5);
        randIcon6 = ituSceneFindWidget(&theScene, "randIcon6"); assert(randIcon6);
        randIcon7 = ituSceneFindWidget(&theScene, "randIcon7"); assert(randIcon7);
        randIcon8 = ituSceneFindWidget(&theScene, "randIcon8"); assert(randIcon8);
        randIcon9 = ituSceneFindWidget(&theScene, "randIcon9"); assert(randIcon9);
        randIcon0 = ituSceneFindWidget(&theScene, "randIcon0"); assert(randIcon0);
        randIcon10 = ituSceneFindWidget(&theScene, "randIcon10"); assert(randIcon10);
        randIcon11 = ituSceneFindWidget(&theScene, "randIcon11"); assert(randIcon11);
        randIcon12 = ituSceneFindWidget(&theScene, "randIcon12"); assert(randIcon12);
        randIcon13 = ituSceneFindWidget(&theScene, "randIcon13"); assert(randIcon13);
        randIcon14 = ituSceneFindWidget(&theScene, "randIcon14"); assert(randIcon14);
        randIcon15 = ituSceneFindWidget(&theScene, "randIcon15"); assert(randIcon15);
        randIcon16 = ituSceneFindWidget(&theScene, "randIcon16"); assert(randIcon16);
        randIcon17 = ituSceneFindWidget(&theScene, "randIcon17"); assert(randIcon17);
        randIcon18 = ituSceneFindWidget(&theScene, "randIcon18"); assert(randIcon18);
        randIcon19 = ituSceneFindWidget(&theScene, "randIcon19"); assert(randIcon19);
    }
    rand_show();
    piont_set(randAnimation1);
    piont_set(randAnimation2);
    piont_set(randAnimation3);
    piont_set(randAnimation4);
    piont_set(randAnimation5);
    piont_set(randAnimation6);
    piont_set(randAnimation7);
    piont_set(randAnimation8);
    piont_set(randAnimation9);
    piont_set(randAnimation0);
    piont_set(randAnimation10);
    piont_set(randAnimation11);
    piont_set(randAnimation12);
    piont_set(randAnimation13);
    piont_set(randAnimation14);
    piont_set(randAnimation15);
    piont_set(randAnimation16);
    piont_set(randAnimation17);
    piont_set(randAnimation18);
    piont_set(randAnimation19);
    return true;
}

bool run_timer(ITUWidget* widget, char* param)
{
    static char name[30];
    static int index[2];

#ifdef WIN32
    static int cnt;
    if (++cnt < 2) return false;
    cnt = 0;
    level0--;
#endif

    rand_show();
    if      (level0 <  10) ituWidgetSetX(runText, 368);
    else if (level0 < 100) ituWidgetSetX(runText, 300);
    else                   ituWidgetSetX(runText, 232);

    sprintf(name, "%d", level0);
    ituTextSetString(runText, name);

    return false;
}

void runReset(void)
{
    runText = NULL;
}