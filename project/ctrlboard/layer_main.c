#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"
#include "global.h"

static ITUScaleCoverFlow* ScaleCoverFlow1;

bool mainOnEnter(ITUWidget* widget, char* param)
{
    if (!ScaleCoverFlow1) {
        ScaleCoverFlow1 = ituSceneFindWidget(&theScene, "ScaleCoverFlow1"); assert(ScaleCoverFlow1);
    }
    return true;
}

bool main_timer(ITUWidget* widget, char* param)
{
    static int cnt = 0;

    if (++cnt < 100) return false;
    cnt = 0;

    printf("ITUScaleCoverFlow\n");
    /* ituScaleCoverFlowOnAction(ScaleCoverFlow1, ITU_ACTION_NEXT, 0); */
    ituScaleCoverFlowNext(ScaleCoverFlow1);
    return true;
}

void mainReset(void)
{
    ScaleCoverFlow1 = NULL;
}
