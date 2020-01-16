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
static ITUAnimation* streamStdAnimation1;
static ITUAnimation* setupAnimation;
static ITUAnimation* setupModelAnimation;
static ITUBackground *setupBackground1, *setupBackground2, *setupBackground3;
static ITUSprite* setupSprite;
static ITUIcon *roastIcon10, *roastIcon11;
static ITUIcon *roastIcon20, *roastIcon21;
static ITUIcon *roastIcon30, *roastIcon31;

static int setupAnimationKey;
static int setupModelAnimationKey;

bool setupOnEnter(ITUWidget* widget, char* param)
{
    if (!setupModelText) {
        setupModelText = ituSceneFindWidget(&theScene, "setupModelText"); assert(setupModelText);
        setupModelIcon = ituSceneFindWidget(&theScene, "setupModelIcon"); assert(setupModelIcon);
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
    setupModelAnimationKey = setupModelAnimation->keyframe;
    return true;
}

bool setup_timer(ITUWidget* widget, char* param)
{
    static int cnt = 0;
    static uint8_t pre;

    cnt++;
    cnt %= 100;

    ituSpriteGoto(setupSprite, 1);

    if      (cnt ==  0) ituAnimationPlay(streamStdAnimation1, 0);
    else if (cnt == 60) ituAnimationStop(streamStdAnimation1);

#ifdef WIN32
    if (cnt == 0) level1--;
#endif

    if (setupAnimationKey == setupAnimation->keyframe && setupAnimation->frame == 0)
        ituAnimationStop(setupAnimation);
    if (setupModelAnimationKey == setupModelAnimation->keyframe && setupModelAnimation->frame == 0)
        ituAnimationStop(setupModelAnimation);
    if (pre != level1) {
        pre = level1;

        setupAnimationKey = pre % 3;
        if (setupAnimationKey > setupAnimation->keyframe)
            ituAnimationPlay(setupAnimation, setupAnimation->keyframe);
        else if (setupAnimationKey < setupAnimation->keyframe)
            ituAnimationReversePlay(setupAnimation, setupAnimation->keyframe);

        setupModelAnimationKey = pre % 4;
        if (setupModelAnimationKey > setupModelAnimation->keyframe)
            ituAnimationPlay(setupModelAnimation, setupModelAnimation->keyframe);
        else if (setupModelAnimationKey < setupModelAnimation->keyframe)
            ituAnimationReversePlay(setupModelAnimation, setupModelAnimation->keyframe);
    }

    ituWidgetSetVisible(roastIcon10, cnt > 10); ituWidgetSetVisible(roastIcon11, cnt > 10);
    ituWidgetSetVisible(roastIcon20, cnt > 40); ituWidgetSetVisible(roastIcon21, cnt > 40);
    ituWidgetSetVisible(roastIcon30, cnt > 70); ituWidgetSetVisible(roastIcon31, cnt > 70);

	return true;
}

void setupReset(void)
{
    setupModelText = NULL;
}
