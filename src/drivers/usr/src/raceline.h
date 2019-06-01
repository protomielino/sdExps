/***************************************************************************

    file                 : raceline.h
    created              : Sat Feb 07 19:53:00 CET 2015
    copyright            : (C) 2015 by Andrew Sumner
    email                : novocas7rian@gmail.com
    version              : $Id: raceline.h,v 1.3 2015/02/07 20:11:49 andrew Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _RACELINE_H_
#define _RACELINE_H_

#include <iostream>
#include <fstream>
#include <string>

#include "linalg.h"
#include "linemode.h"
#include "cardata.h"
#include "manual_override.h"

#define NUM_RACELINES 4
#define NUM_RACELINE_SPEEDS 4 //12
#define MAXSEGMENTS 6000
#define MAXDIVS 11000

typedef struct
{
    tSituation *s;
    v2d *target;
    int next_apex;
    int avoidmode;
    float speed;
    float left_speed;
    float right_speed;
    float left_speed_outsteer;
    float right_speed_outsteer;
    float leftlane_2left;
    float rightlane_2right;
    float raceoffset;
    float lookahead;
    float racesteer;
    float laststeer;
    float angle;
    float target_lane;
    float speedangle;
    double error;
    double vnerror;
    float skid;
    bool coll;
    LLineMode *linemode;
} RaceLineDriveData;

class Driver;
class LLearn;

class LRaceLine
{
  public:
    LRaceLine(Driver *pdriver);
    ~LRaceLine();

    void setCornerSpeeds( double nml, double mid, double slow ) { CornerSpeed = nml; CornerSpeedMid = mid; CornerSpeedSlow = slow; }
    void setBrakeDist( double wi ) { BrakeDist = wi; }
    void setCar( tCarElt *mycar, SingleCardata *mycardata ) { car = mycar; cardata = mycardata; }
    void setRwData( tTrack* t, void **carParmHandle, tSituation *s);
    void setOverrides( LManualOverrideCollection *overrides ) { overrideCollection = overrides; }
    void setSkill( double tskill) { skill = tskill; }

    LManualOverrideCollection *overrideCollection;

    double minTurnInverse;
    double CornerSpeed;
    double turnSpeed;
    double CornerSpeedMid;
    double CornerSpeedSlow;
    double CornerSpeedFactor;
    double outsideCornerSpeed;
    double insideCornerSpeed;
    double offlineTurnSpeed;
    double offlineBrakeDist;
    double brakeDist;
    double brakeDistMid;
    double brakeDistSlow;
    double BrakeDist;
    double IntMargin;
    double ExtMargin;
    double AvoidIntMargin;
    double AvoidExtMargin;
    double speedAdjust;
    double wheelbase;
    double wheeltrack;
    double coldTyreFactor;
    double curveFactor;
    double curveAccel;
    double curveBrake;
    double bumpCaution;
    double offlineBumpCaution;
    double slopeFactor;
    double offlineSlopeFactor;
    double fulltankPercent;
    double midtankPercent;
    double maxfuel;
    double edgeLineMargin;
    double lookAhead;
    double lookAheadOut;
    double lookAheadEmpty;
    double lookAheadColdFactor;
    double outsteerSpeedReducer;
    double steerSkidFactor;
    double steerSkidOfflineFactor;
    double errorCorrectionFactor;
    double outsideSteeringDampener;
    double outsideSteeringDampenerOverlap;
    double outsideSteeringDampenerAccel;
    double steerTimeFactor;
    double skill;

    int iterations;
    int side_iterations;
    int rl_speed_mode;
    int saveTrack;
    int loadTrack;

    int Divs;
    int DivLength;
    int Segs;

    int racelineOverride;

    Driver *driver;

    double Width;
    double Length;
    double TargetSpeed;
    double *tSegDist;
    int *tSegIndex;
    tTrackSeg **tSegment;
    double *tElemLength;
    double **tx;
    double **ty;
    double **tz;
    double **tzd;
    double **tLane;
    double **tRInverse;
    double **tSpeed;
    int **tDivSeg;
    double *tDistance;
    double *tMaxSpeed;
    double *tMaxSpeedCold;
    double **txLeft;
    double **tyLeft;
    double **txRight;
    double **tyRight;
    double *tFriction;
    double *tzLeft;
    double *tzRight;

    double Time;

    int fDirt;
    int Prev;
    int Next;
    int NextNextNext;
    int This;

    int m_raceType;

    tCarElt *car;

    void UpdateTxTy(int i, int rl);
    void SetSegmentInfo(const tTrackSeg *pseg, double d, int i, double l);
    void SplitTrack(tTrack *ptrack, int rl, bool preLoaded);
    double GetRInverse(int prev, double x, double y, int next, double *tX, double *tY);
    double GetRInverse(double prevx, double prevy, double x, double y, double nextx, double nexty);

    double getRInverseWithDiv(int raceline, int div)
    {
        if (div >= 0) return tRInverse[raceline][div];

        return tRInverse[raceline][Next];
    }

    double getRInverse(int raceline = LINE_RL)
    {
        return tRInverse[raceline][Next];
    }

    double getRInverse(double distance) { int d = ((Next + int(distance/DivLength)) % Divs); return tRInverse[LINE_RL][d]; }

    void AdjustRadius(int prev, int i, int next, double TargetRInverse, int rl, double Security = 0);
    void Smooth(int Step, int rl);
    void StepInterpolate(int iMin, int iMax, int Step, int rl);
    void Interpolate(int Step, int rl);
    void InitTrack(tTrack* track, tSituation *p);
    void CreateSideRaceline(int rl);
    void NewRace(tCarElt* newcar, tSituation *s);
    void GetRaceLineData(RaceLineDriveData *data, bool transitioning);
    void GetPoint( float offset, float lookahead, vec2f *rt );

    int isOnLine( int line);
    double correctLimit(int line);
    double getAvoidSpeed( float distance1, float distance2 );
    double getLookAhead(int rl, double leftMargin, double rightMargin, bool coll);
    double getLineSpeed(int Div, int rl)
    {
        switch (rl)
        {
            case LINE_RL:
                return tSpeed[LINE_RL][Div];
            case LINE_RL_MID:
                return tSpeed[LINE_RL_MID][Div];
            case LINE_RL_SLOW:
                return tSpeed[LINE_RL_SLOW][Div];
            case LINE_LEFT:
                return tSpeed[LINE_LEFT][Div];
            case LINE_RIGHT:
                return tSpeed[LINE_RIGHT][Div];
            case LINE_MID:
                return tSpeed[LINE_MID][Div];
            default:
                return 0.0;
        }
        return 0.0;
    }

    double getMaxSpeed(int Div, int rl);
    double getBumpCaution(int Div, int rl);
    double getFriction(int Div);
    double getCurveFactor(int Div, bool isBraking);
    double getCornerSpeed( int Div, int rl);
    double getBrakeDist( int Div, int rl);
    double getIntMargin( int raceline, int Div, double rInverse );
    double getExtMargin( int raceline, int Div, double rInverse );
    double getMinTurnInverse(int raceline);
    double getSlowestSpeedForDistance(double distance, int raceline, double midMargin, int *div = NULL);
    int findNextCorner(tCarElt *ocar, int index = -1, int *apex_div = NULL, double *distance = NULL);
    int findNextBrakingZone();
    double SegCamberForNext();
    int DivIndexForCar(tCarElt *theCar, double catchtime = -1.0);
    int DivIndexForCarDistance(tCarElt *theCar, double distance);
    void slowestSpeedBetweenDivs(int startdiv, int enddiv, double *rlspeed, double *leftspeed, double *rightspeed);
    void ComputeRacelineSpeed(int i, int rl, double **tSpeed, int speedrl);
    void ComputeRacelineBraking(int i, int rl, double **tSpeed, int speedrl);
    void UpdateRacelineSpeeds(int raceType);

    bool InBrakingZone(int rl)
    {
        if (tSpeed[rl][Next] < tSpeed[rl][This])
            return true;
        return false;
    }

    void SmoothSideRacingLines();

#define MAX_TDATA 50
    int turnSpeedSegStart[MAX_TDATA];
    int turnSpeedSegEnd[MAX_TDATA];
    float segTurnSpeed[MAX_TDATA];

    int brakeDistSegStart[MAX_TDATA];
    int brakeDistSegEnd[MAX_TDATA];
    float segBrakeDist[MAX_TDATA];

    int MspeedSegStart[MAX_TDATA];
    int MspeedSegEnd[MAX_TDATA];
    float segMaxSpeed[MAX_TDATA];

    int extMargSegStart[MAX_TDATA];
    int extMargSegEnd[MAX_TDATA];
    float segExtMargin[MAX_TDATA];

    int intMargSegStart[MAX_TDATA];
    int intMargSegEnd[MAX_TDATA];
    float segIntMargin[MAX_TDATA];

    int line_verbose;
    int steer_verbose;
    int LineIndex;
    int LineSave;
    int rlLine;
    int racelineDebug;

    double last_left_steer;
    double last_rl_steer;
    double last_right_steer;
    double last_steer;
    double last_last_steer;
    double last_steer_diff;
    int last_target_raceline;
    double last_lane;
    double cornersteer;

    bool hasSlow;
    bool hasMid;
    int useMergedSpeed;

    tTrack *m_pTrack;

    double lastUpdateDist;
    double stopUpdateDist;
    double resumeUpdateDist;
    bool hasLastUpdate;

    SingleCardata *cardata;
    LLearn *learn;

    double CalculateSpeedAtDiv(LLineMode *lineMode, int div);
    double OfflineLane(int div, double leftMargin, double rightMargin);
    bool IsSlowerThanSpeedToDiv(LLineMode *lineMode, double speed, int div, double *slowSpeed);
    double CalculateOfflineSpeed(int index, int next, double leftMargin, double rightMargin);
    bool ApproachingBrakeZone(tCarElt *pCar = NULL);

  private:
    // Utility functions
    void saveFile();
    void StoreData(tTrack* track);
    void WriteLine(tTrack* track);
    void WriteTrack(tTrack* track);
    int DivIndex(RaceLineDriveData *data, int raceline, double *X, double *Y);
    int DivIndex(RaceLineDriveData *data, double leftMargin, double rightMargin, double *X, double *Y);
    double SegCamber(int rl, int div);
    void CalcZCurvature(int rl);
    float AdjustLookahead(int raceline, float lookahead);
    double CaTT();
    double AdjustTxForMargin(int div, double lane);
    double AdjustTyForMargin(int div, double lane);
    double RInverseForMargin(int div, double leftMargin, double rightMargin);

    double CalculateSpeed(RaceLineDriveData *data, double X, double Y, int index, int next, double leftMargin, double rightMargin, double tLeftMargin, double tRightMargin);
    double CalculateSpeed(RaceLineDriveData *data, double X, double Y, int index, int next, double *tSpd, double *tX, double *tY);
    double CalculateOffset(double *tX, double *tY, int next, int rl);
    double CalculateOffset(int div, double leftMargin, double rightMargin);
    double CalculateCurvature(double c0, double tRINext, double tRIIndex);
    double CalculateMixedCurvature(double c0, int Index, double transition_percentage);
    void SteerTheCar(RaceLineDriveData *data, int raceline);
    void SteerTheCarOffline(RaceLineDriveData *data);
    void CalculateOfflineSpeed(RaceLineDriveData *data);
    void updateRLSpeedMode();
    float smoothSteering(float steercmd, float laststeer);
    bool LoadTrack(tTrack *track, tSituation *s);
    double Point2Lane(int rl, double x, double y);
    void SaveTrack(tTrack *track, tSituation *s);
    int readInt(FILE *fp);
    double readDouble(FILE *fp);
    void removeNewLineCharacters(char *text);
};
#endif // _RACELINE_H_
