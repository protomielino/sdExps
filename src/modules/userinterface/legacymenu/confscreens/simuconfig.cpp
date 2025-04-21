/***************************************************************************

    file        : simuconfig.cpp
    created     : Wed Nov  3 21:48:26 CET 2004
    copyright   : (C) 2004 by Eric Espie
    email       : eric.espie@free.fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** @file
            Simutation option menu
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <raceman.h>
#include <portability.h>
#include <tgfclient.h>

#include "simuconfig.h"

#include "gui.h"

/*#ifndef OFFICIAL_ONLY
#define OFFICIAL_ONLY 1
#endif*/


/* list of available simulation engine */
static const int DefaultSimuVersion = 0;
static const char *SimuVersionList[] =
    { RM_VAL_MOD_SIMU_V5, RM_VAL_MOD_SIMU_REPLAY };
static const char *SimuVersionDispNameList[] = 	{ "V5.0", "Replay" };
static const int NbSimuVersions = sizeof(SimuVersionList) / sizeof(SimuVersionList[0]);
static int CurSimuVersion = DefaultSimuVersion;

/* list of available multi-threading schemes */
static const char *MultiThreadSchemeList[] = {RM_VAL_AUTO, RM_VAL_ON, RM_VAL_OFF};
static const int NbMultiThreadSchemes = sizeof(MultiThreadSchemeList) / sizeof(MultiThreadSchemeList[0]);

/* list of available thread affinity schemes */
static const char *ThreadAffinitySchemeList[] = {RM_VAL_ON, RM_VAL_OFF};
static const int NbThreadAffinitySchemes = sizeof(ThreadAffinitySchemeList) / sizeof(ThreadAffinitySchemeList[0]);

static int CurMultiThreadScheme = 0;    // Auto
static int CurThreadAffinityScheme = 0; // On

/* list of available replay record schemes */
static const char *ReplaySchemeList[] = {RM_VAL_REPLAY_OFF, RM_VAL_REPLAY_LOW, RM_VAL_REPLAY_NORMAL, RM_VAL_REPLAY_HIGH, RM_VAL_REPLAY_PERFECT};
static const char *ReplaySchemeDispNameList[] = 	{"off", "Low", "Normal", "High", "Perfect"};
static const int NbReplaySchemes = sizeof(ReplaySchemeList) / sizeof(ReplaySchemeList[0]);
static int CurReplayScheme = 0;

/* list of available start paused schemes */
static const char *StartPausedSchemeList[] = {RM_VAL_ON, RM_VAL_OFF};
static const int NbStartPausedSchemes = sizeof(StartPausedSchemeList) / sizeof(StartPausedSchemeList[0]);
static int CurStartPausedScheme = 0; // On

/* list of available cooldown schemes */
static const char *CooldownSchemeList[] = {RM_VAL_ON, RM_VAL_OFF};
static const int NbCooldownSchemes = sizeof(CooldownSchemeList) / sizeof(CooldownSchemeList[0]);
static int CurCooldownScheme = 0; // On

/* gui label ids */
static int SimuVersionId;
static int MultiThreadSchemeId;
static int ThreadAffinitySchemeId;
static int ReplayRateSchemeId;

static int StartPausedSchemeId;

static int CooldownSchemeId;

/* gui screen handles */
static void *ScrHandle = NULL;
static void *PrevScrHandle = NULL;


static void loadSimuCfg(void)
{
    const char *simuVersionName;
    const char *multiThreadSchemeName;
    const char *threadAffinitySchemeName;
#ifdef THIRD_PARTY_SQLITE3
    const char *replayRateSchemeName;
#endif
    const char *startPausedSchemeName;
    const char *cooldownSchemeName;

    int i;

    char buf[1024];

    void *paramHandle = GfParmReadFileLocal(RACE_ENG_CFG, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);

    // Simulation engine name.
    simuVersionName = GfParmGetStr(paramHandle, RM_SECT_MODULES, RM_ATTR_MOD_SIMU, SimuVersionList[DefaultSimuVersion]);
    for (i = 0; i < NbSimuVersions; i++)
    {
        if (strcmp(simuVersionName, SimuVersionList[i]) == 0)
        {
            CurSimuVersion = i;
            break;
        }
    }

    // Check if the selected simulation module is there, and fall back to the default one if not.
    snprintf(buf, sizeof(buf), "%smodules/simu/%s%s", GfLibDir(), SimuVersionList[CurSimuVersion], DLLEXT);
    if (!GfFileExists(buf))
    {
        GfLogWarning("User settings %s physics engine module not found ; falling back to %s\n",
                     SimuVersionList[CurSimuVersion], SimuVersionList[DefaultSimuVersion]);
        CurSimuVersion = DefaultSimuVersion;
    }

    // Multi-threading.
    multiThreadSchemeName = GfParmGetStr(paramHandle, RM_SECT_RACE_ENGINE, RM_ATTR_MULTI_THREADING, MultiThreadSchemeList[0]);
    for (i = 0; i < NbMultiThreadSchemes; i++)
    {
        if (strcmp(multiThreadSchemeName, MultiThreadSchemeList[i]) == 0)
        {
            CurMultiThreadScheme = i;
            break;
        }
    }

    // Thread affinity.
    threadAffinitySchemeName = GfParmGetStr(paramHandle, RM_SECT_RACE_ENGINE, RM_ATTR_THREAD_AFFINITY, ThreadAffinitySchemeList[0]);
    for (i = 0; i < NbThreadAffinitySchemes; i++)
    {
        if (strcmp(threadAffinitySchemeName, ThreadAffinitySchemeList[i]) == 0)
        {
            CurThreadAffinityScheme = i;
            break;
        }
    }

    // Replay Rate
#ifdef THIRD_PARTY_SQLITE3
    replayRateSchemeName = GfParmGetStr(paramHandle, RM_SECT_RACE_ENGINE, RM_ATTR_REPLAY_RATE, ReplaySchemeList[0]);
    for (i = 0; i < NbReplaySchemes; i++)
    {
        if (strcmp(replayRateSchemeName, ReplaySchemeList[i]) == 0)
        {
            CurReplayScheme = i;
            break;
        }
    }
#else
    CurReplayScheme = 0;
#endif

    // startpaused
    startPausedSchemeName = GfParmGetStr(paramHandle,RM_SECT_RACE_ENGINE, RM_ATTR_STARTPAUSED, StartPausedSchemeList[1]);
    for (i = 0; i < NbStartPausedSchemes; i++)
    {
        if (strcmp(startPausedSchemeName, StartPausedSchemeList[i]) == 0)
        {
            CurStartPausedScheme = i;
            break;
        }
    }

    // cooldown
    cooldownSchemeName = GfParmGetStr(paramHandle,RM_SECT_RACE_ENGINE, RM_ATTR_COOLDOWN, CooldownSchemeList[1]);
    for (i = 0; i < NbCooldownSchemes; i++)
    {
        if (strcmp(cooldownSchemeName, CooldownSchemeList[i]) == 0)
        {
            CurCooldownScheme = i;
            break;
        }
    }

    GfParmReleaseHandle(paramHandle);

    GfuiLabelSetText(ScrHandle, SimuVersionId, SimuVersionDispNameList[CurSimuVersion]);
    GfuiLabelSetText(ScrHandle, MultiThreadSchemeId, MultiThreadSchemeList[CurMultiThreadScheme]);
    GfuiLabelSetText(ScrHandle, ThreadAffinitySchemeId, ThreadAffinitySchemeList[CurThreadAffinityScheme]);
    GfuiLabelSetText(ScrHandle, ReplayRateSchemeId, ReplaySchemeDispNameList[CurReplayScheme]);

#ifndef THIRD_PARTY_SQLITE3
    GfuiEnable(ScrHandle, ReplayRateSchemeId, GFUI_DISABLE);
#endif
    GfuiLabelSetText(ScrHandle, StartPausedSchemeId, StartPausedSchemeList[CurStartPausedScheme]);
    GfuiLabelSetText(ScrHandle, CooldownSchemeId, CooldownSchemeList[CurCooldownScheme]);
}


/* Save the choosen values in the corresponding parameter file */
static void storeSimuCfg(void * /* dummy */)
{
    void *paramHandle = GfParmReadFileLocal(RACE_ENG_CFG, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
    GfParmSetStr(paramHandle, RM_SECT_MODULES, RM_ATTR_MOD_SIMU, SimuVersionList[CurSimuVersion]);
    GfParmSetStr(paramHandle, RM_SECT_RACE_ENGINE, RM_ATTR_MULTI_THREADING, MultiThreadSchemeList[CurMultiThreadScheme]);
    GfParmSetStr(paramHandle, RM_SECT_RACE_ENGINE, RM_ATTR_THREAD_AFFINITY, ThreadAffinitySchemeList[CurThreadAffinityScheme]);
    GfParmSetStr(paramHandle, RM_SECT_RACE_ENGINE, RM_ATTR_REPLAY_RATE, ReplaySchemeList[CurReplayScheme]);
    GfParmSetStr(paramHandle, RM_SECT_RACE_ENGINE, RM_ATTR_STARTPAUSED, StartPausedSchemeList[CurStartPausedScheme]);
    GfParmSetStr(paramHandle, RM_SECT_RACE_ENGINE, RM_ATTR_COOLDOWN, CooldownSchemeList[CurCooldownScheme]);
    GfParmWriteFile(NULL, paramHandle, "raceengine");
    GfParmReleaseHandle(paramHandle);

    /* return to previous screen */
    GfuiScreenActivate(PrevScrHandle);
    return;
}

/* Change the simulation version (but only show really available modules) */
static void
onChangeSimuVersion(int dir)
{
    char buf[1024];

    if (!dir)
        return;

    const int oldSimuVersion = CurSimuVersion;
    do
    {
        CurSimuVersion = (CurSimuVersion + NbSimuVersions + dir) % NbSimuVersions;

        snprintf(buf, sizeof(buf), "%smodules/simu/%s%s", GfLibDir(), SimuVersionList[CurSimuVersion], DLLEXT);
    }
    while (!GfFileExists(buf) && CurSimuVersion != oldSimuVersion);

    GfuiLabelSetText(ScrHandle, SimuVersionId, SimuVersionDispNameList[CurSimuVersion]);
}

static void
onChangeSimuLeft(void *)
{
    onChangeSimuVersion(-1);
}

static void
onChangeSimuRight(void *)
{
    onChangeSimuVersion(1);
}

/* Change the multi-threading scheme */
static void
onChangeMultiThreadScheme(int dir)
{
    CurMultiThreadScheme =
        (CurMultiThreadScheme + NbMultiThreadSchemes + dir) % NbMultiThreadSchemes;

    GfuiLabelSetText(ScrHandle, MultiThreadSchemeId, MultiThreadSchemeList[CurMultiThreadScheme]);
}

static void
onChangeMultiThreadLeft(void *)
{
    onChangeMultiThreadScheme(-1);
}

static void
onChangeMultiThreadRight(void *)
{
    onChangeMultiThreadScheme(1);
}

/* Change the thread affinity scheme */
static void
onChangeThreadAffinityScheme(int dir)
{
    CurThreadAffinityScheme =
        (CurThreadAffinityScheme + NbThreadAffinitySchemes + dir) % NbThreadAffinitySchemes;

    GfuiLabelSetText(ScrHandle, ThreadAffinitySchemeId, ThreadAffinitySchemeList[CurThreadAffinityScheme]);
}

static void
onChangeThreadAffinityLeft(void *)
{
    onChangeThreadAffinityScheme(-1);
}

static void
onChangeThreadAffinityRight(void *)
{
    onChangeThreadAffinityScheme(1);
}

#ifdef THIRD_PARTY_SQLITE3
/* Change the replay rate scheme */
static void
onChangeReplayRateScheme(int dir)
{
    CurReplayScheme =
        (CurReplayScheme + NbReplaySchemes + dir) % NbReplaySchemes;

    GfuiLabelSetText(ScrHandle, ReplayRateSchemeId, ReplaySchemeDispNameList[CurReplayScheme]);
}

static void
onChangeReplayRateLeft(void *)
{
    onChangeReplayRateScheme(-1);
}

static void
onChangeReplayRateRight(void *)
{
    onChangeReplayRateScheme(1);
}
#endif

/* Change the startpaused scheme */
static void
onChangeStartPausedScheme(int dir)
{
    CurStartPausedScheme =
        (CurStartPausedScheme + NbStartPausedSchemes + dir) % NbStartPausedSchemes;

    GfuiLabelSetText(ScrHandle, StartPausedSchemeId, StartPausedSchemeList[CurStartPausedScheme]);
}

static void
onChangeStartPausedLeft(void *)
{
    onChangeStartPausedScheme(-1);
}

static void
onChangeStartPausedRight(void *)
{
    onChangeStartPausedScheme(1);
}

/* Change the cooldown scheme */
static void
onChangeCooldownScheme(int dir)
{
    CurCooldownScheme =
        (CurCooldownScheme + NbCooldownSchemes + dir) % NbCooldownSchemes;

    GfuiLabelSetText(ScrHandle, CooldownSchemeId, CooldownSchemeList[CurCooldownScheme]);
}

static void
onChangeCooldownLeft(void *)
{
    onChangeCooldownScheme(-1);
}

static void
onChangeCooldownRight(void *)
{
    onChangeCooldownScheme(1);
}

static void onActivate(void * /* dummy */)
{
    loadSimuCfg();
}


/* Menu creation */
void *
SimuMenuInit(void *prevMenu)
{
    /* screen already created */
    if (ScrHandle) {
    return ScrHandle;
    }
    PrevScrHandle = prevMenu;

    ScrHandle = GfuiScreenCreate((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);

    void *menuDescHdle = GfuiMenuLoad("simuconfigmenu.xml");
    GfuiMenuCreateStaticControls(ScrHandle, menuDescHdle);

    SimuVersionId = GfuiMenuCreateLabelControl(ScrHandle,menuDescHdle,"simulabel");
    GfuiMenuCreateButtonControl(ScrHandle, menuDescHdle, "simuleftarrow", NULL, onChangeSimuLeft);
    GfuiMenuCreateButtonControl(ScrHandle, menuDescHdle, "simurightarrow", NULL, onChangeSimuRight);

    MultiThreadSchemeId = GfuiMenuCreateLabelControl(ScrHandle, menuDescHdle, "mthreadlabel");

    GfuiMenuCreateButtonControl(ScrHandle, menuDescHdle, "mthreadleftarrow", NULL, onChangeMultiThreadLeft);
    GfuiMenuCreateButtonControl(ScrHandle, menuDescHdle, "mthreadrightarrow", NULL, onChangeMultiThreadRight);

    ThreadAffinitySchemeId = GfuiMenuCreateLabelControl(ScrHandle, menuDescHdle, "threadafflabel");

    GfuiMenuCreateButtonControl(ScrHandle, menuDescHdle, "threadaffleftarrow", NULL, onChangeThreadAffinityLeft);
    GfuiMenuCreateButtonControl(ScrHandle, menuDescHdle, "threadaffrightarrow", NULL, onChangeThreadAffinityRight);

    ReplayRateSchemeId = GfuiMenuCreateLabelControl(ScrHandle, menuDescHdle, "replayratelabel");
#ifdef THIRD_PARTY_SQLITE3
    GfuiMenuCreateButtonControl(ScrHandle, menuDescHdle, "replayrateleftarrow", NULL, onChangeReplayRateLeft);
    GfuiMenuCreateButtonControl(ScrHandle, menuDescHdle, "replayraterightarrow", NULL, onChangeReplayRateRight);
#endif

    StartPausedSchemeId = GfuiMenuCreateLabelControl(ScrHandle, menuDescHdle, "startpausedlabel");

    GfuiMenuCreateButtonControl(ScrHandle, menuDescHdle, "startpausedleftarrow", NULL, onChangeStartPausedLeft);
    GfuiMenuCreateButtonControl(ScrHandle, menuDescHdle, "startpausedrightarrow", NULL, onChangeStartPausedRight);


    CooldownSchemeId = GfuiMenuCreateLabelControl(ScrHandle, menuDescHdle, "cooldownlabel");

    GfuiMenuCreateButtonControl(ScrHandle, menuDescHdle, "cooldownleftarrow", NULL, onChangeCooldownLeft);
    GfuiMenuCreateButtonControl(ScrHandle, menuDescHdle, "cooldownrightarrow", NULL, onChangeCooldownRight);

    GfuiMenuCreateButtonControl(ScrHandle, menuDescHdle, "ApplyButton", PrevScrHandle, storeSimuCfg);
    GfuiMenuCreateButtonControl(ScrHandle, menuDescHdle, "CancelButton", PrevScrHandle, GfuiScreenActivate);


    GfParmReleaseHandle(menuDescHdle);

    GfuiMenuDefaultKeysAdd(ScrHandle);
    GfuiAddKey(ScrHandle, GFUIK_RETURN, "Apply", NULL, storeSimuCfg, NULL);
    GfuiAddKey(ScrHandle, GFUIK_ESCAPE, "Cancel", PrevScrHandle, GfuiScreenActivate, NULL);
    GfuiAddKey(ScrHandle, GFUIK_LEFT, "Previous simu engine version", NULL, onChangeSimuLeft, NULL);
    GfuiAddKey(ScrHandle, GFUIK_RIGHT, "Next simu engine version", NULL, onChangeSimuRight, NULL);
    GfuiAddKey(ScrHandle, GFUIK_UP, "Previous multi-threading scheme", NULL, onChangeMultiThreadLeft, NULL);
    GfuiAddKey(ScrHandle, GFUIK_DOWN, "Next multi-threading scheme", NULL, onChangeMultiThreadRight, NULL);

    return ScrHandle;
}
