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
static ITUBackground* mainTopBackgroundBig;
static ITUBackground* mainTopBackgroundMid;
static ITUBackground* mainBotBackgroundBig;
static ITUBackground* mainBotBackgroundMid;
static ITUText* mainBotTextBig;
static ITUText* mainTopTextBig;
static ITUText* mainBotTextMid;
static ITUText* mainTopTextMid;
static ITUText* botBackText;
static ITUText* topBackText;

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
        mainTopBackgroundBig = ituSceneFindWidget(&theScene, "mainTopBackgroundBig"); assert(mainTopBackgroundBig);
        mainTopBackgroundMid = ituSceneFindWidget(&theScene, "mainTopBackgroundMid"); assert(mainTopBackgroundMid);
        mainBotBackgroundBig = ituSceneFindWidget(&theScene, "mainBotBackgroundBig"); assert(mainBotBackgroundBig);
        mainBotBackgroundMid = ituSceneFindWidget(&theScene, "mainBotBackgroundMid"); assert(mainBotBackgroundMid);
        mainBotTextBig = ituSceneFindWidget(&theScene, "mainBotTextBig"); assert(mainBotTextBig);
        mainTopTextBig = ituSceneFindWidget(&theScene, "mainTopTextBig"); assert(mainTopTextBig);
        mainBotTextMid = ituSceneFindWidget(&theScene, "mainBotTextMid"); assert(mainBotTextMid);
        mainTopTextMid = ituSceneFindWidget(&theScene, "mainTopTextMid"); assert(mainTopTextMid);
        botBackText = ituSceneFindWidget(&theScene, "botBackText"); assert(botBackText);
        topBackText = ituSceneFindWidget(&theScene, "topBackText"); assert(topBackText);
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
    char *mode_text[3] = {"专业模式", "智能菜单", "厨房助手"};

#ifdef WIN32
    static int cnt;
    if (++cnt < 15) return false;
    cnt = 0;
    level0--;
#else
    if (level1)
#endif
        ituLayerGoto(setupLayer);

    if (pre == level0) return false;

    switch (level0 % 3) {
    case 2:
        ituWidgetSetX(unselectedIcon1, 744); ituWidgetSetY(unselectedIcon1, 195);
        ituWidgetSetX(selectedIcon,    742); ituWidgetSetY(selectedIcon,    220);
        ituWidgetSetX(unselectedIcon2, 744); ituWidgetSetY(unselectedIcon2, 248);
        ituTextSetString(mainBotTextBig, mode_text[2]); ituTextSetString(mainTopTextBig, mode_text[0]);
        ituTextSetString(mainBotTextMid, mode_text[2]); ituTextSetString(mainTopTextMid, mode_text[0]);
        ituTextSetString(botBackText, mode_text[0]); ituTextSetString(topBackText, mode_text[1]);
        fu = fopen(CFG_PUBLIC_DRIVE ":/main/kitchen_helper.png", "rb"); fd = fopen(CFG_PUBLIC_DRIVE ":/main/pro_mode.png", "rb");
        break;
    case 1:
        ituWidgetSetX(unselectedIcon1, 744); ituWidgetSetY(unselectedIcon1, 195);
        ituWidgetSetX(unselectedIcon2, 744); ituWidgetSetY(unselectedIcon2, 222);
        ituWidgetSetX(selectedIcon,    742); ituWidgetSetY(selectedIcon,    246);
        ituTextSetString(mainBotTextBig, mode_text[0]); ituTextSetString(mainTopTextBig, mode_text[1]);
        ituTextSetString(mainBotTextMid, mode_text[0]); ituTextSetString(mainTopTextMid, mode_text[1]);
        ituTextSetString(botBackText, mode_text[1]); ituTextSetString(topBackText, mode_text[2]);
        fu = fopen(CFG_PUBLIC_DRIVE ":/main/pro_mode.png", "rb"); fd = fopen(CFG_PUBLIC_DRIVE ":/main/smart_mode.png", "rb");
        break;
    case 0:
        ituWidgetSetX(selectedIcon,    742); ituWidgetSetY(selectedIcon,    193);
        ituWidgetSetX(unselectedIcon1, 744); ituWidgetSetY(unselectedIcon1, 222);
        ituWidgetSetX(unselectedIcon2, 744); ituWidgetSetY(unselectedIcon2, 248);
        ituTextSetString(mainBotTextBig, mode_text[1]); ituTextSetString(mainTopTextBig, mode_text[2]);
        ituTextSetString(mainBotTextMid, mode_text[1]); ituTextSetString(mainTopTextMid, mode_text[2]);
        ituTextSetString(botBackText, mode_text[2]); ituTextSetString(topBackText, mode_text[0]);
        fu = fopen(CFG_PUBLIC_DRIVE ":/main/smart_mode.png", "rb"); fd = fopen(CFG_PUBLIC_DRIVE ":/main/kitchen_helper.png", "rb");
    default:
        break;
    }
    load(fu, &mainTopBackgroundBig->icon, &mainTopBackgroundMid->icon);
    load(fd, &mainBotBackgroundBig->icon, &mainBotBackgroundMid->icon);

    ituAnimationGoto(mainTopAnimation, 0); ituAnimationGoto(mainBotAnimation, 0);
    ituAnimationGotoFrame(mainTopAnimation, 0); ituAnimationGotoFrame(mainBotAnimation, 0);
    /* ituAnimationGoto(topBackAnimation, 0); ituAnimationGoto(botBackAnimation, 0); */
    /* ituAnimationGotoFrame(topBackAnimation, 0); ituAnimationGotoFrame(botBackAnimation, 0); */

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
