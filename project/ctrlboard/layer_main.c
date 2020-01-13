#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"
#include "global.h"

static ITUAnimation* mainTopAnimation;
static ITUAnimation* mainBotAnimation;
static ITUAnimation* topBackAnimation;
static ITUAnimation* botBackAnimation;
static ITUIcon* selectedIcon;
static ITUIcon* unselectedIcon1;
static ITUIcon* unselectedIcon2;
static ITUIcon* mainTopIconBig;
static ITUIcon* mainTopIconMid;
static ITUIcon* mainBotIconBig;
static ITUIcon* mainBotIconMid;

static ITULayer* setupLayer;

static int pre;

bool mainOnEnter(ITUWidget* widget, char* param)
{
    if (!mainTopAnimation) {
        mainTopAnimation = ituSceneFindWidget(&theScene, "mainTopAnimation"); assert(mainTopAnimation);
        mainBotAnimation = ituSceneFindWidget(&theScene, "mainBotAnimation"); assert(mainBotAnimation);
        topBackAnimation = ituSceneFindWidget(&theScene, "topBackAnimation"); assert(topBackAnimation);
        botBackAnimation = ituSceneFindWidget(&theScene, "botBackAnimation"); assert(botBackAnimation);
        selectedIcon = ituSceneFindWidget(&theScene, "selectedIcon"); assert(selectedIcon);
        unselectedIcon1 = ituSceneFindWidget(&theScene, "unselectedIcon1"); assert(unselectedIcon1);
        unselectedIcon2 = ituSceneFindWidget(&theScene, "unselectedIcon2"); assert(unselectedIcon2);
        mainTopIconBig = ituSceneFindWidget(&theScene, "mainTopIconBig"); assert(mainTopIconBig);
        mainTopIconMid = ituSceneFindWidget(&theScene, "mainTopIconMid"); assert(mainTopIconMid);
        mainBotIconBig = ituSceneFindWidget(&theScene, "mainBotIconBig"); assert(mainBotIconBig);
        mainBotIconMid = ituSceneFindWidget(&theScene, "mainBotIconMid"); assert(mainBotIconMid);
        setupLayer = ituSceneFindWidget(&theScene, "setupLayer"); assert(setupLayer);
    }
    ituAnimationPlay(topBackAnimation, 0); ituAnimationPlay(botBackAnimation, 0);
    pre = level0;
    return true;
}

static void load(FILE* f, ITUIcon* icon1, ITUIcon* icon2)
{
    uint8_t* data;
    int size;
    struct stat sb;

    if (f == NULL) return;

    if (fstat(fileno(f), &sb) != -1) {
        size = sb.st_size;
        data = malloc(size);
        if (data) {
            size = fread(data, 1, size, f);
            ituIconLoadPngData(icon1, data, size);
            ituIconLoadPngData(icon2, data, size);
            free(data);
        }
    }
    fclose(f);
}

bool main_timer(ITUWidget* widget, char* param)
{
    FILE *fu, *fd;

#ifdef WIN32
    static int cnt;
    if (++cnt < 50) return false;
    cnt = 0;
    level0--;
#endif

#ifndef WIN32
    if (level1)
#endif
        ituLayerGoto(setupLayer);

    if (pre == level0) return false;

    switch (level0 % 3) {
    case 2:
        ituWidgetSetX(unselectedIcon1, 744); ituWidgetSetY(unselectedIcon1, 195);
        ituWidgetSetX(selectedIcon,    742); ituWidgetSetY(selectedIcon,    220);
        ituWidgetSetX(unselectedIcon2, 744); ituWidgetSetY(unselectedIcon2, 248);
        fu = fopen(CFG_PUBLIC_DRIVE ":/main/kitchen_helper.png", "rb"); fd = fopen(CFG_PUBLIC_DRIVE ":/main/pro_mode.png", "rb");
        break;
    case 1:
        ituWidgetSetX(unselectedIcon1, 744); ituWidgetSetY(unselectedIcon1, 195);
        ituWidgetSetX(unselectedIcon2, 744); ituWidgetSetY(unselectedIcon2, 222);
        ituWidgetSetX(selectedIcon,    742); ituWidgetSetY(selectedIcon,    246);
        fu = fopen(CFG_PUBLIC_DRIVE ":/main/pro_mode.png", "rb"); fd = fopen(CFG_PUBLIC_DRIVE ":/main/smart_mode.png", "rb");
        break;
    case 0:
        ituWidgetSetX(selectedIcon,    742); ituWidgetSetY(selectedIcon,    193);
        ituWidgetSetX(unselectedIcon1, 744); ituWidgetSetY(unselectedIcon1, 222);
        ituWidgetSetX(unselectedIcon2, 744); ituWidgetSetY(unselectedIcon2, 248);
        fu = fopen(CFG_PUBLIC_DRIVE ":/main/smart_mode.png", "rb"); fd = fopen(CFG_PUBLIC_DRIVE ":/main/kitchen_helper.png", "rb");
    default:
        break;
    }
    load(fu, mainTopIconBig, mainTopIconMid);
    load(fd, mainBotIconBig, mainBotIconMid);

    ituAnimationStop(mainTopAnimation); ituAnimationStop(mainBotAnimation);
    ituAnimationStop(topBackAnimation); ituAnimationStop(botBackAnimation);
    if (pre < level0) {
        ituAnimationPlay(mainTopAnimation, 0); ituAnimationPlay(mainBotAnimation, 0);
    } else {
        ituAnimationReversePlay(mainTopAnimation, 2); ituAnimationReversePlay(mainBotAnimation, 2);
    }
    ituAnimationPlay(topBackAnimation, 0); ituAnimationPlay(botBackAnimation, 0);

    pre = level0;
    return true;
}

void mainReset(void)
{
    mainTopAnimation = NULL;
}
