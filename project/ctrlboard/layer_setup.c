﻿#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"
#include "global.h"

#define CYCLE_R   190  //圆半径
#define CYCLE_X   400  //圆心X坐标
#define CYCLE_Y   200  //圆心Y坐标

static const short sinTable[360] = {
     0,    571,   1143,   1714,   2285,   2855,   3425,   3993,   4560,   5126,
  5690,   6252,   6812,   7371,   7927,   8480,   9032,   9580,  10125,  10668,
 11207,  11742,  12275,  12803,  13327,  13848,  14364,  14876,  15383,  15886,
 16383,  16876,  17364,  17846,  18323,  18794,  19260,  19720,  20173,  20621,
 21062,  21497,  21926,  22347,  22762,  23170,  23571,  23964,  24351,  24730,
 25101,  25465,  25821,  26169,  26509,  26841,  27165,  27481,  27788,  28087,
 28377,  28659,  28932,  29196,  29451,  29697,  29935,  30163,  30381,  30591,
 30791,  30982,  31164,  31336,  31498,  31651,  31794,  31928,  32051,  32165,
 32270,  32364,  32449,  32523,  32588,  32643,  32688,  32723,  32748,  32763,
 32767,  32763,  32748,  32723,  32688,  32643,  32588,  32523,  32449,  32364,
 32270,  32165,  32051,  31928,  31794,  31651,  31498,  31336,  31164,  30982,
 30791,  30591,  30381,  30163,  29935,  29697,  29451,  29196,  28932,  28659,
 28377,  28087,  27788,  27481,  27165,  26842,  26509,  26169,  25821,  25465,
 25101,  24730,  24351,  23965,  23571,  23170,  22762,  22347,  21926,  21497,
 21062,  20621,  20174,  19720,  19260,  18795,  18323,  17846,  17364,  16876,
 16384,  15886,  15383,  14876,  14364,  13848,  13328,  12803,  12275,  11743,
 11207,  10668,  10125,   9580,   9032,   8481,   7927,   7371,   6812,   6252,
  5690,   5126,   4560,   3993,   3425,   2856,   2285,   1715,   1143,    571,
     0,   -571,  -1143,  -1714,  -2285,  -2855,  -3425,  -3993,  -4560,  -5125,
 -5690,  -6252,  -6812,  -7371,  -7927,  -8480,  -9031,  -9580, -10125, -10668,
-11207, -11742, -12275, -12803, -13327, -13848, -14364, -14876, -15383, -15886,
-16383, -16876, -17364, -17846, -18323, -18794, -19260, -19720, -20173, -20621,
-21062, -21497, -21925, -22347, -22762, -23170, -23571, -23964, -24351, -24730,
-25101, -25465, -25821, -26169, -26509, -26841, -27165, -27481, -27788, -28087,
-28377, -28659, -28932, -29196, -29451, -29697, -29935, -30163, -30381, -30591,
-30791, -30982, -31164, -31336, -31498, -31651, -31794, -31928, -32051, -32165,
-32270, -32364, -32449, -32523, -32588, -32643, -32688, -32723, -32748, -32763,
-32767, -32763, -32748, -32723, -32688, -32643, -32588, -32523, -32449, -32364,
-32270, -32165, -32051, -31928, -31794, -31651, -31498, -31336, -31164, -30982,
-30791, -30591, -30382, -30163, -29935, -29697, -29451, -29196, -28932, -28659,
-28377, -28087, -27788, -27481, -27165, -26842, -26509, -26169, -25821, -25465,
-25101, -24730, -24351, -23965, -23571, -23170, -22762, -22347, -21926, -21497,
-21062, -20621, -20174, -19720, -19260, -18795, -18323, -17846, -17364, -16876,
-16384, -15886, -15383, -14876, -14364, -13848, -13328, -12803, -12275, -11743,
-11207, -10668, -10126,  -9580,  -9032,  -8481,  -7927,  -7371,  -6813,  -6252,
 -5690,  -5126,  -4560,  -3993,  -3425,  -2856,  -2285,  -1715,  -1143,   -572,
};
static const short cosTable[360] = {
 32767,  32763,  32748,  32723,  32688,  32643,  32588,  32523,  32449,  32364,
 32270,  32165,  32051,  31928,  31794,  31651,  31498,  31336,  31164,  30982,
 30791,  30591,  30381,  30163,  29935,  29697,  29451,  29196,  28932,  28659,
 28377,  28087,  27788,  27481,  27165,  26841,  26509,  26169,  25821,  25465,
 25101,  24730,  24351,  23965,  23571,  23170,  22762,  22347,  21926,  21497,
 21062,  20621,  20174,  19720,  19260,  18794,  18323,  17846,  17364,  16876,
 16384,  15886,  15383,  14876,  14364,  13848,  13327,  12803,  12275,  11743,
 11207,  10668,  10125,   9580,   9032,   8481,   7927,   7371,   6812,   6252,
  5690,   5126,   4560,   3993,   3425,   2855,   2285,   1714,   1143,    571,
     0,   -571,  -1143,  -1714,  -2285,  -2855,  -3425,  -3993,  -4560,  -5125,
 -5690,  -6252,  -6812,  -7371,  -7927,  -8480,  -9032,  -9580, -10125, -10668,
-11207, -11742, -12275, -12803, -13327, -13848, -14364, -14876, -15383, -15886,
-16383, -16876, -17364, -17846, -18323, -18794, -19260, -19720, -20173, -20621,
-21062, -21497, -21926, -22347, -22762, -23170, -23571, -23964, -24351, -24730,
-25101, -25465, -25821, -26169, -26509, -26841, -27165, -27481, -27788, -28087,
-28377, -28659, -28932, -29196, -29451, -29697, -29935, -30163, -30381, -30591,
-30791, -30982, -31164, -31336, -31498, -31651, -31794, -31928, -32051, -32165,
-32270, -32364, -32449, -32523, -32588, -32643, -32688, -32723, -32748, -32763,
-32767, -32763, -32748, -32723, -32688, -32643, -32588, -32523, -32449, -32364,
-32270, -32165, -32051, -31928, -31794, -31651, -31498, -31336, -31164, -30982,
-30791, -30591, -30381, -30163, -29935, -29697, -29451, -29196, -28932, -28659,
-28377, -28087, -27788, -27481, -27165, -26842, -26509, -26169, -25821, -25465,
-25101, -24730, -24351, -23965, -23571, -23170, -22762, -22347, -21926, -21497,
-21062, -20621, -20174, -19720, -19260, -18795, -18323, -17846, -17364, -16876,
-16384, -15886, -15383, -14876, -14364, -13848, -13328, -12803, -12275, -11743,
-11207, -10668, -10125,  -9580,  -9032,  -8481,  -7927,  -7371,  -6812,  -6252,
 -5690,  -5126,  -4560,  -3993,  -3425,  -2856,  -2285,  -1715,  -1143,   -572,
     0,    571,   1143,   1714,   2285,   2855,   3425,   3993,   4560,   5125,
  5689,   6252,   6812,   7371,   7927,   8480,   9031,   9580,  10125,  10668,
 11207,  11742,  12274,  12803,  13327,  13848,  14364,  14876,  15383,  15886,
 16383,  16876,  17364,  17846,  18323,  18794,  19260,  19720,  20173,  20621,
 21062,  21497,  21925,  22347,  22762,  23170,  23571,  23964,  24351,  24730,
 25101,  25465,  25821,  26169,  26509,  26841,  27165,  27481,  27788,  28087,
 28377,  28659,  28932,  29196,  29451,  29697,  29934,  30163,  30381,  30591,
 30791,  30982,  31164,  31336,  31498,  31651,  31794,  31928,  32051,  32165,
 32270,  32364,  32449,  32523,  32588,  32643,  32688,  32723,  32748,  32763
};

static ITUCoverFlow* setupCoverFlow;
static ITUBackground* setupBackgroundChild[10] = { 0 };
static ITUIcon* setupText[10] = { 0 };
static int pre;
static int pre1;
static ITUAnimation* Animation1;

static void arrangePosition()
{
	int i;
	for (i = 0; i < 10; i++) {
		if (ituWidgetGetY(setupBackgroundChild[i]) < 0 || ituWidgetGetY(setupBackgroundChild[i]) > 480){
			ituWidgetSetVisible(setupText[i], false);
			continue;
		}
		ituWidgetSetVisible(setupText[i], true);
		ituWidgetSetPosition(setupText[i],
			CYCLE_X - (CYCLE_R * (int)sinTable[ituWidgetGetY(setupBackgroundChild[i]) * 90 / CYCLE_R] >> 15),
			CYCLE_Y - (CYCLE_R * (int)cosTable[ituWidgetGetY(setupBackgroundChild[i]) * 90 / CYCLE_R] >> 15));
	}
}

bool setupOnEnter(ITUWidget* widget, char* param)
{
    static char name[30];
	int i;

    if (!setupCoverFlow) {
        setupCoverFlow = ituSceneFindWidget(&theScene, "setupCoverFlow"); assert(setupCoverFlow);

		for (i = 0; i < 10; i++) {
			sprintf(name, "setupText%d", i);
            setupText[i] = ituSceneFindWidget(&theScene, name); assert(setupText[i]);
			setupBackgroundChild[i] = (ITUBackground*)itcTreeGetChildAt(setupCoverFlow, i); assert(setupBackgroundChild[i]);
			//ituWidgetSetHeight(setupBackgroundChild[i],96);
		}

        Animation1 = ituSceneFindWidget(&theScene, "Animation1"); assert(Animation1);
    }
	arrangePosition();
    pre = level0;
    pre1 = level1;
    return true;
}

bool setup_timer(ITUWidget* widget, char* param)
{
	int i, x;

#ifdef WIN32
    static int cnt;
    if (++cnt > 50) {
        cnt = 0;
        level0--;
        level1++;
    }
#endif

    if (pre != level0) {
        if (pre < level0) {
            ituCoverFlowNext(setupCoverFlow);
        } else {
            ituCoverFlowPrev(setupCoverFlow);
        }
        pre = level0;
    }
    if (pre1 != level1) {
        pre1 = level1;
        ituAnimationStop(Animation1);
        ituAnimationPlay(Animation1, 0);
    }

	ituWidgetUpdate(setupBackgroundChild[setupCoverFlow->focusIndex], ITU_EVENT_LAYOUT, 0, 0, 0);
    /* printf("focusIndex%d\n", setupCoverFlow->focusIndex); */
    arrangePosition();
	return true;
}

void setupReset(void)
{
    setupCoverFlow = NULL;
}