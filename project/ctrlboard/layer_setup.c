#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"
#include "global.h"

static ITUText* setupModelText;
static ITUIcon* setupModelIcon;
static ITUIcon* setupModelIcon1;
static ITUIcon* setupModelIcon2;
static ITUIcon* setupIcon;
static ITUAnimation* streamStdAnimation1;
static ITUAnimation* setupAnimation;
static ITUAnimation* setupModelAnimation;
static ITUBackground *setupBackground1, *setupBackground2, *setupBackground3;
static ITUSprite* setupSprite;
static ITUIcon *roastIcon10, *roastIcon11;
static ITUIcon *roastIcon20, *roastIcon21;
static ITUIcon *roastIcon30, *roastIcon31;

static int setupAnimationKey;

bool setupOnEnter(ITUWidget* widget, char* param)
{
    if (!setupModelText) {
        setupModelText = ituSceneFindWidget(&theScene, "setupModelText"); assert(setupModelText);
        setupModelIcon = ituSceneFindWidget(&theScene, "setupModelIcon"); assert(setupModelIcon);
        setupModelIcon1 = ituSceneFindWidget(&theScene, "setupModelIcon1"); assert(setupModelIcon1);
        setupModelIcon2 = ituSceneFindWidget(&theScene, "setupModelIcon2"); assert(setupModelIcon2);
        setupIcon = ituSceneFindWidget(&theScene, "setupIcon"); assert(setupIcon);
        streamStdAnimation1 = ituSceneFindWidget(&theScene, "streamStdAnimation1"); assert(streamStdAnimation1);
        setupAnimation = ituSceneFindWidget(&theScene, "setupAnimation"); assert(setupAnimation);
        setupModelAnimation = ituSceneFindWidget(&theScene, "setupModelAnimation"); assert(setupModelAnimation);
        setupBackground1 = ituSceneFindWidget(&theScene, "setupBackground1"); assert(setupBackground1);
        setupBackground2 = ituSceneFindWidget(&theScene, "setupBackground2"); assert(setupBackground2);
        setupBackground3 = ituSceneFindWidget(&theScene, "setupBackground3"); assert(setupBackground3);
        setupSprite = ituSceneFindWidget(&theScene, "setupSprite"); assert(setupSprite);
        roastIcon10 = ituSceneFindWidget(&theScene, "roastIcon10"); assert(roastIcon10);
        roastIcon11 = ituSceneFindWidget(&theScene, "roastIcon11"); assert(roastIcon11);
        roastIcon20 = ituSceneFindWidget(&theScene, "roastIcon20"); assert(roastIcon20);
        roastIcon21 = ituSceneFindWidget(&theScene, "roastIcon21"); assert(roastIcon21);
        roastIcon30 = ituSceneFindWidget(&theScene, "roastIcon30"); assert(roastIcon30);
        roastIcon31 = ituSceneFindWidget(&theScene, "roastIcon31"); assert(roastIcon31);
    }

    setupAnimationKey = setupAnimation->keyframe;
    return true;
}

static void load(FILE* f, ITUIcon* icon1)
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
            free(data);
        }
    }
    fclose(f);
}

static void load_1(FILE* f, ITUIcon* icon1, ITUIcon* icon2)
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

bool setup_timer(ITUWidget* widget, char* param)
{
    static int loop = 0;
    static uint8_t pre = ~0, pre1 = ~0;
    static char *mode_text[2] = {"标准蒸", "经典烘焙"};

    loop++;
    loop %= 100;

    if (pre1 != level1) {
        FILE *f1, *f2, *f3;
        pre1 = level1;
        ituSpriteGoto(setupSprite, pre1 % 2);

        switch (pre1 % 2) {
        case 0:
            f1 = fopen(CFG_PUBLIC_DRIVE ":/setup/steam_std.png", "rb");
            f2 = fopen(CFG_PUBLIC_DRIVE ":/setup/steam_setup.png", "rb");
            f3 = fopen(CFG_PUBLIC_DRIVE ":/setup/steam_cursor.png", "rb");
            ituTextSetString(setupModelText, mode_text[0]);
            break;
        case 1:
            f1 = fopen(CFG_PUBLIC_DRIVE ":/setup/roast.png", "rb");
            f2 = fopen(CFG_PUBLIC_DRIVE ":/setup/roast_setup.png", "rb");
            f3 = fopen(CFG_PUBLIC_DRIVE ":/setup/roast_cursor.png", "rb");
            ituTextSetString(setupModelText, mode_text[1]);
        default:
            break;
        }
        load(f1, setupModelIcon);
        load(f3, setupIcon);
        load_1(f2, setupModelIcon1, setupModelIcon2);
    }

    if      (loop ==  0) ituAnimationPlay(streamStdAnimation1, 0);
    else if (loop == 60) ituAnimationStop(streamStdAnimation1);

#ifdef WIN32
    if (loop == 0) level0--;
#endif

    if (setupAnimationKey == setupAnimation->keyframe && setupAnimation->frame == 0)
        ituAnimationStop(setupAnimation);

    if (pre != level0) {
        pre = level0;

        setupAnimationKey = pre % 3;
        if (setupAnimationKey > setupAnimation->keyframe)
            ituAnimationPlay(setupAnimation, setupAnimation->keyframe);
        else if (setupAnimationKey < setupAnimation->keyframe)
            ituAnimationReversePlay(setupAnimation, setupAnimation->keyframe);

        ituAnimationGotoFrame(setupModelAnimation, pre % 120);
    }

    ituWidgetSetVisible(roastIcon10, loop > 10); ituWidgetSetVisible(roastIcon11, loop > 10);
    ituWidgetSetVisible(roastIcon20, loop > 40); ituWidgetSetVisible(roastIcon21, loop > 40);
    ituWidgetSetVisible(roastIcon30, loop > 70); ituWidgetSetVisible(roastIcon31, loop > 70);

	return true;
}

void setupReset(void)
{
    setupModelText = NULL;
}
