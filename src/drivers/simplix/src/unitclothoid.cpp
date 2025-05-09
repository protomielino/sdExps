//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitclothoid.cpp
//--------------------------------------------------------------------------*
// A robot for Speed Dreams-Version 2.X simuV4
//--------------------------------------------------------------------------*
// Lane clothoide like
// Fahrspur clothoiden ähnlich
//
// File         : unitclothoid.cpp
// Created      : 2007.11.25
// Last changed : 2014.11.29
// Copyright    : © 2007-2014 Wolf-Dieter Beelitz
// eMail        : wdbee@users.sourceforge.net
// Version      : 4.05.000
//--------------------------------------------------------------------------*
// Teile diese Unit basieren auf diversen Header-Dateien von TORCS
//
//    Copyright: (C) 2000 by Eric Espie
//    eMail    : torcs@free.fr
//
// dem erweiterten Robot-Tutorial bt
//
//    Copyright: (C) 2002-2004 Bernhard Wymann
//    eMail    : berniw@bluewin.ch
//
// dem Roboter delphin
//
//    Copyright: (C) 2006-2007 Wolf-Dieter Beelitz
//    eMail    : wdbee@users.sourceforge.net
//
// dem Roboter wdbee_2007
//
//    Copyright: (C) 2006-2007 Wolf-Dieter Beelitz
//    eMail    : wdbee@users.sourceforge.net
//
// und dem Roboter mouse_2006
//
//    Copyright: (C) 2006-2007 Tim Foden
//
//--------------------------------------------------------------------------*
// This program was developed and tested on windows XP
// There are no known Bugs, but:
// Who uses the files accepts, that no responsibility is adopted
// for bugs, dammages, aftereffects or consequential losses.
//
// Das Programm wurde unter Windows XP entwickelt und getestet.
// Fehler sind nicht bekannt, dennoch gilt:
// Wer die Dateien verwendet erkennt an, dass für Fehler, Schäden,
// Folgefehler oder Folgeschäden keine Haftung übernommen wird.
//--------------------------------------------------------------------------*
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Im übrigen gilt für die Nutzung und/oder Weitergabe die
// GNU GPL (General Public License)
// Version 2 oder nach eigener Wahl eine spätere Version.
//--------------------------------------------------------------------------*
#include "unitglobal.h"
#include "unitcommon.h"

#include "unitclothoid.h"
#include "unitdriver.h"
#include "unitlinreg.h"
#include "unitparam.h"
#include "unitsysfoo.h"

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <cjson/cJSON.h>

//==========================================================================*
// Default constructor
//--------------------------------------------------------------------------*
TClothoidLane::TClothoidLane(TDriver &driver):
  TLane(driver)
{
}
//==========================================================================*

//==========================================================================*
// Destructor
//--------------------------------------------------------------------------*
TClothoidLane::~TClothoidLane()
{
}
//==========================================================================*

//==========================================================================*
// Create a smooth lane
//--------------------------------------------------------------------------*
void TClothoidLane::MakeSmoothPath(
  TTrackDescription* Track,
  TParam& Param,
  const TOptions& Opts)
{
  TCarParam& CarParam = Param.oCarParam;
  oBase = Opts.Base;
  oBaseFactor = Opts.BaseFactor;

  if (Opts.MaxR < FLT_MAX)
    LaneType = ltLeft;
  else if (Opts.MaxL < FLT_MAX)
    LaneType = ltRight;
  else
    LaneType = ltFree;

  if (Opts.Side)
  {
    LogSimplix.debug("Switch to CarParam2\n");
    CarParam = Param.oCarParam2;
  }
  TLane::Initialise(Track, Param.Fix, CarParam, Opts.MaxL, Opts.MaxR);
  const int Count = Track->Count();

  int FwdRange = 110;
  CalcFwdAbsCrv(FwdRange);

  const int Delta = 25;
  int L = 8;

  int Step = 1;                                  // Initialize step width
  while (Step * 16 < Count)                      // Find largest step width
    Step *= 2;

  LogSimplix.debug("OptimisePath:\n");
  while (Step > 0)
  {
    LogSimplix.debug("Step: %d\n",Step);
    for (int I = 0; I < L; I++)
    {
        OptimisePath(Step, Delta, 0, Param.oCarParam.oUglyCrvZ);
    }
    Step >>= 1;
  }

  if (Opts.BumpMod)
  {
    LogSimplix.debug("AnalyseBumps:\n");
    AnalyseBumps(false);
    //AnalyseBumps(true);

    Step = 1;
    Step <<= ANALYSE_STEPS;

    while (Step > 0)
    {
      LogSimplix.debug("Step: %d\n",Step);
      for (int I = 0; I < L; I++)
      {
        OptimisePath(Step, Delta, Opts.BumpMod, Param.oCarParam.oUglyCrvZ);
        CalcCurvaturesZ();
        CalcFwdAbsCrv(FwdRange);
        CalcMaxSpeeds(Step);
        PropagateBreaking(Step);
        PropagateAcceleration(Step);
      }
      Step >>= 1;
    }
  }
  else
  {
    Step = 1;
    CalcCurvaturesZ();
    CalcMaxSpeeds(Step);
    PropagateBreaking(Step);
    PropagateAcceleration(Step);
  }

}
//==========================================================================*

//==========================================================================*
// Load a smooth lane
//--------------------------------------------------------------------------*
bool TClothoidLane::LoadSmoothPath(
  const char* TrackLoad,
  TTrackDescription* Track,
  TParam& Param,
  const TOptions& Opts)
{
  TCarParam& CarParam = Param.oCarParam;
  oBase = Opts.Base;
  oBaseFactor = Opts.BaseFactor;

  if (Opts.Side)
    CarParam = Param.oCarParam2;
  TLane::Initialise(Track, Param.Fix, CarParam, Opts.MaxL, Opts.MaxR);
  return LoadPointsFromFile(TrackLoad);
}
//==========================================================================*

//==========================================================================*
// Smooth a pitlane
//--------------------------------------------------------------------------*
void TClothoidLane::SmoothPath(
/*  TTrackDescription* Track,*/
  const TParam& Param,
  const TOptions& Opts)
{
  oBase = Opts.Base;
  oBaseFactor = Opts.BaseFactor;

  int FwdRange = 110;
  CalcFwdAbsCrv(FwdRange);

  const int Delta = 25;
  const int L = 8;

  int Step = 1;
  Step <<= ANALYSE_STEPS;
  while (Step > 0)
  {
    LogSimplix.debug("Step: %d\n",Step);
    for (int I = 0; I < L; I++)
    {
      OptimisePath(Step, Delta, Opts.BumpMod, Param.oCarParam.oUglyCrvZ);
      CalcCurvaturesZ();
      CalcFwdAbsCrv(FwdRange);
      CalcMaxSpeeds(Step);
      PropagateBreaking(Step);
      PropagateAcceleration(Step);
    }
    Step >>= 1;
  }
}
//==========================================================================*

//==========================================================================*
// Analyse lane to find dangerous bumps
//--------------------------------------------------------------------------*
void TClothoidLane::AnalyseBumps(bool DumpInfo)
{
  // Here we look at the bumps on the track, and increase the buffers
  // from the edge after the bumps

  // Get height profile
  CalcCurvaturesZ();
  // Get an estimate of speeds
  CalcMaxSpeeds();
  PropagateBreaking();
  PropagateAcceleration();

  const int Count = oTrack->Count();

  double Sz = oPathPoints[0].Point.z;
  double Vz = 0;
  double Pz = Sz;

  int I;
  int J;
  int K;
  for (I = 0; I < 2; I++)
  {
    K = Count - 1;

    for (J = 0; J < Count; J++)
    {
      double OldPz = Pz;

      double V = (oPathPoints[J].AccSpd + oPathPoints[K].AccSpd) * 0.5;
      if (V < 1.00)
        V = 1.0;
      double S = TUtils::VecLenXY(oPathPoints[J].Point - oPathPoints[K].Point);
      double Dt = S / V;

      if (Dt > 1.00)
        Dt = 1.00;

      Pz = oPathPoints[J].Point.z;
      Sz += Vz * Dt - 0.5 * G * Dt * Dt;
      Vz -= G * Dt;

      if (Sz <= Pz)
      {
        double NewVz = (Pz - OldPz) / Dt;
        if (Vz < NewVz)
          Vz = NewVz;
        Sz = Pz;
      }

      oPathPoints[J].FlyHeight = Sz - Pz;

      if ((I == 1) && DumpInfo)
      {
        LogSimplix.debug( "%4d v %3.0f crv %7.4f dt %.3f pz %5.2f sz %5.2f vz %5.2f -> h %5.2f\n",
          J, oPathPoints[J].AccSpd * 3.6, oPathPoints[J].Crv, Dt,
          Pz, Sz, Vz, oPathPoints[J].FlyHeight);
      }
      K = J;
    }
  }

  for (I = 0; I < 3; I++)
  {
    for (J = 0; J < Count; J++)
    {
      K = (J + 1) % Count;
      if (oPathPoints[J].FlyHeight < oPathPoints[K].FlyHeight)
        oPathPoints[J].FlyHeight = oPathPoints[K].FlyHeight;
    }
  }
}
//==========================================================================*

//==========================================================================*
// Smooth points between steps
//--------------------------------------------------------------------------*
void TClothoidLane::SmoothBetween(int Step, double BumpMod)
{
  if (Step > 1)
  {
  const int Count = oTrack->Count();

  // Smooth values between Steps
  TPathPt* L0 = 0;
  TPathPt* L1 = &oPathPoints[((Count - 1) / Step) * Step];
  TPathPt* L2 = &oPathPoints[0];
  TPathPt* L3 = &oPathPoints[Step];

  int J = 2 * Step;
  for (int I = 0; I < Count; I += Step)
  {
    L0 = L1;
    L1 = L2;
    L2 = L3;
    L3 = &oPathPoints[J];

    J += Step;
    if (J >= Count)
      J = 0;

    TVec3d P0 = L0->Point;
    TVec3d P1 = L1->Point;
    TVec3d P2 = L2->Point;
    TVec3d P3 = L3->Point;

    double Crv1 = TUtils::CalcCurvatureXY(P0, P1, P2);
    double Crv2 = TUtils::CalcCurvatureXY(P1, P2, P3);

    if (I + Step > Count)
      Step = Count - I;

    for (int K = 1; K < Step; K++)
    {
      TPathPt* P = &(oPathPoints[(I + K) % Count]);
      double Len1 = (P->CalcPt() - P1).len();
      double Len2 = (P->CalcPt() - P2).len();
      Adjust(Crv1, Len1, Crv2, Len2, L1, P, L2, P1, P2, BumpMod);
    }
  }
  }
  else
  {
    const int Count = oTrack->Count();

    // Smooth values between Steps
    TPathPt* L0 = 0;
    TPathPt* L1 = &oPathPoints[Count - 1];
    TPathPt* L2 = &oPathPoints[0];
    TPathPt* L3 = &oPathPoints[1];

    int J = 2;
    for (int I = 0; I < 3*Count; I++)
    {
      L0 = L1;
      L1 = L2;
      L2 = L3;
      L3 = &oPathPoints[J];

      J++;
      if (J >= Count)
        J = 0;

      //TVec3d P0 = L0->Point;
      //TVec3d P1 = L1->Point;
      //TVec3d P2 = L2->Point;
      //TVec3d P3 = L3->Point;

      double T = L0->Offset + L1->Offset + L2->Offset;
      L1->Offset = (float) (T/3);
    }
  }
}
//==========================================================================*

//==========================================================================*
// Set offset
//--------------------------------------------------------------------------*
void TClothoidLane::SetOffset
  (double Crv, double T, TPathPt* P, const TPathPt* PP, const TPathPt* PN)
{
  double Margin = oFixCarParam.oWidth / 2;
  double WL = -P->WtoL() + Margin;
  double WR = P->WtoR() - Margin;
  double BorderInner = oFixCarParam.oBorderInner
      + MAX(0.0,MIN(oFixCarParam.oMaxBorderInner,
      oFixCarParam.oBorderScale * fabs(Crv) - 1));
  double BorderOuter = oFixCarParam.oBorderOuter;

  if (Crv >= 0) // turn to left
  {
    if (LaneType == ltLeft)
    {
      T = MAX(T,WL);
      T = MIN(T,WR);
    }
    else if (LaneType == ltRight)
    {
      T = MAX(T,WL);
      T = MIN(T,WR - BorderOuter);
    }
    else
    {
      WL += BorderInner;
      T = MAX(T,WL);
      T = MIN(T,WR - BorderOuter);
    }
  }
  else // turn to right
  {
    if (LaneType == ltLeft)
    {
      T = MIN(T,WR);
      T = MAX(T,WL + BorderOuter);
    }
    else if (LaneType == ltRight)
    {
      T = MIN(T,WR);
      T = MAX(T,WL);
    }
    else
    {
      WR -= BorderInner;
      T = MIN(T,WR);
      T = MAX(T,WL + BorderOuter);
    }
  }

  if (!(P->Fix))
  {
    P->Offset = (float) T;
    P->Point = P->CalcPt();
    P->Crv = (float) TUtils::CalcCurvatureXY(PP->Point, P->Point, PN->Point);
  }
}
//==========================================================================*

//==========================================================================*
// Optimize Line
//--------------------------------------------------------------------------*
void TClothoidLane::OptimiseLine
  (int Index, int Step,	double HLimit,
  TPathPt* L3,	const TPathPt* L2, const TPathPt* L4)
{
  TLinearRegression LR;

  const int Count = oTrack->Count();

  int I = (Index + Count - Step) % Count;
  while (oPathPoints[I].FlyHeight > HLimit)
  {
    LR.Add(oPathPoints[I].Point.GetXY());
    I = (I + Count - Step) % Count;
  }

  LR.Add(oPathPoints[I].Point.GetXY());

  I = Index;
  while (oPathPoints[I].FlyHeight > HLimit)
  {
    LR.Add(oPathPoints[I].Point.GetXY());
    I = (I + Step) % Count;
  }

  LR.Add(oPathPoints[I].Point.GetXY());

  LogSimplix.debug("OptimiseLine Index: %4d", Index);

  TVec2d P, V;
  LR.CalcLine(P, V);

  double T;
  TUtils::LineCrossesLine(L3->Pt().GetXY(), L3->Norm().GetXY(), P, V, T);

  SetOffset(0, T, L3, L2, L4);
}
//==========================================================================*

//==========================================================================*
// Optimize
//--------------------------------------------------------------------------*
void TClothoidLane::Adjust
  (double Crv1, double Len1, double Crv2, double Len2,
  const TPathPt* PP,
  TPathPt* P,
  const TPathPt* PN,
  TVec3d VPP,
  TVec3d VPN,
  double BumpMod)
{
  double T = P->Offset;
  double Crv = (Len2 * Crv1 + Len1 * Crv2) / (Len1 + Len2);
  if (Crv != 0.0)
  {
    if (Crv1 * Crv2 >= 0
      && fabs(Crv1) < MAX_SPEED_CRV
      && fabs(Crv2) < MAX_SPEED_CRV)
    {
      Crv *= 0.9;
    }

    TUtils::LineCrossesLineXY(P->Pt(), P->Norm(), VPP, VPN - VPP, T);

    double Delta = DELTA_T;
    double DeltaCrv = TUtils::CalcCurvatureXY
      (VPP, P->Pt() + P->Norm() * (T + Delta), VPN);

    if (BumpMod > 0 && BumpMod < 2)
      Delta *= 1 - MAX(0.0,MIN(0.5,P->FlyHeight - 0.1)) * BumpMod;


    T += Delta * Crv / DeltaCrv;
  }
  SetOffset(Crv, T, P, PP, PN);
}
//==========================================================================*

//==========================================================================*
// Optimize
//--------------------------------------------------------------------------*
void TClothoidLane::Optimise
  (double Factor, TPathPt* L3,
  const TPathPt* L0, const TPathPt*	L1, const TPathPt*	L2,
  const TPathPt* L4, const TPathPt*	L5,	const TPathPt*	L6,
  double BumpMod)
{
  TVec3d P0 = L0->Point;
  TVec3d P1 = L1->Point;
  TVec3d P2 = L2->Point;
  TVec3d P3 = L3->Point;
  TVec3d P4 = L4->Point;
  TVec3d P5 = L5->Point;
  TVec3d P6 = L6->Point;

  double Crv1 = TUtils::CalcCurvatureXY(P1, P2, P3);
  double Crv2 = TUtils::CalcCurvatureXY(P3, P4, P5);

  double Len1 = myhypot(P3.x - P2.x, P3.y - P2.y);
  double Len2 = myhypot(P4.x - P3.x, P4.y - P3.y);

  if (Crv1 * Crv2 > 0)
  {
    double Crv0 = TUtils::CalcCurvatureXY(P0, P1, P2);
    double Crv3 = TUtils::CalcCurvatureXY(P4, P5, P6);
    if (Crv0 * Crv1 > 0 && Crv2 * Crv3 > 0)
    {
      if (fabs(Crv0) < fabs(Crv1) && fabs(Crv1) * 1.02 < fabs(Crv2))
      {
        Crv1 *= Factor/oBase;
        Crv0 *= Factor/oBase;
      }
      else if(fabs(Crv0) > fabs(Crv1) * 1.02 && fabs(Crv1) > fabs(Crv2))
      {
        Crv1 *= Factor*oBase;
        Crv0 *= Factor*oBase;
      }
    }
  }
  else if(Crv1 * Crv2 < 0)
  {
    double Crv0 = TUtils::CalcCurvatureXY(P0, P1, P2);
    double Crv3 = TUtils::CalcCurvatureXY(P4, P5, P6);
    if (Crv0 * Crv1 > 0 && Crv2 * Crv3 > 0)
    {
      if(fabs(Crv1) < fabs(Crv2) && fabs(Crv1) < fabs(Crv3))
      {
        Crv1 = (Crv1 * 0.25 + Crv2 * 0.75);
        Crv0 = (Crv0 * 0.25 + Crv3 * 0.75);
      }
      else if(fabs(Crv2) < fabs(Crv1) && fabs(Crv2) < fabs(Crv0))
      {
        Crv2 = (Crv2 * 0.25 + Crv1 * 0.75);
        Crv3 = (Crv3 * 0.25 + Crv0 * 0.75);
      }
    }
  }
  Adjust(Crv1, Len1, Crv2, Len2, L2, L3, L4, P2, P4, BumpMod);
}
//==========================================================================*

//==========================================================================*
// Optimize Lane
//--------------------------------------------------------------------------*
void TClothoidLane::OptimisePath
  (int Step, int NIterations, double BumpMod, double UglyCrvZ/*, bool Smooth */)
{
  const int Count = oTrack->Count();

  for (int I = 0; I < NIterations; I++)
  {
    TPathPt* L0 = NULL;
    TPathPt* L1 = &oPathPoints[Count - 3 * Step];
    TPathPt* L2 = &oPathPoints[Count - 2 * Step];
    TPathPt* L3 = &oPathPoints[Count - Step];
    TPathPt* L4 = &oPathPoints[0];
    TPathPt* L5 = &oPathPoints[Step];
    TPathPt* L6 = &oPathPoints[2 * Step];
    TPathPt* LFly;

    // Go forwards
    int	K = 3 * Step;
    int	N = (Count + Step - 1) / Step;
    for (int J = 0; J < N; J++)
    {
      L0 = L1;
      L1 = L2;
      L2 = L3;
      L3 = L4;
      L4 = L5;
      L5 = L6;
      L6 = &oPathPoints[K];
      LFly = L3;

      int Index = (K + Count - 3 * Step) % Count;
      double Factor = oBaseFactor;

      if (LFly->CrvZ < UglyCrvZ)
      {
        Optimise(Factor/10, L3, L0, L1, L2, L4, L5, L6, BumpMod);
      }
      else if (LFly->FlyHeight > 0.035)
      {
        Optimise(Factor/100, L3, L0, L1, L2, L4, L5, L6, BumpMod);
      }
      else if (BumpMod == 2 && L3->FlyHeight > 0.1)
      {
        LogSimplix.debug("OptimiseLine Index: %d\n",Index);
        OptimiseLine(Index, Step, 0.1, L3, L2, L4);
      }
      else
      {
        Optimise(Factor, L3, L0, L1, L2, L4, L5, L6, BumpMod);
      }
      //K = (K + Step) % Count;
      K += Step;
      if (K >= Count)
        K = 0;
    }
  }

  // Smooth values between Steps
  //if (Step > 1)
    SmoothBetween(Step,BumpMod);
}
//==========================================================================*

//==========================================================================*
// Save lane to file
//--------------------------------------------------------------------------*
bool TClothoidLane::SaveToFile(const char* Filename)
{
#ifdef mysecure
  FILE* F;
  int err = myfopen(&F, Filename, "w");
#else
  FILE* F = fopen(Filename, "w");
#endif
  if (F == 0)
    return false;

  fprintf(F, "%d\n",oTrack->Count());
  fprintf(F, "%g\n",oTrack->Length());
  fprintf(F, "%g\n",oTrack->Length()/oTrack->Count());
  fprintf(F, "%g\n",oTrack->Width());

  for (int I = 0; I < oTrack->Count(); I++)
  {
    TPathPt& P = oPathPoints[I];                 // Points in this lane
    const TVec3d& C = P.Pt();
    const TVec3d& N = P.Norm();
    fprintf(F, "%d\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\n",
      I,C.x, C.y, C.z, N.x, N.y, N.z, P.WToL, P.Offset, P.WToR,
      P.Point.x, P.Point.y, P.AccSpd);
  }

  fclose(F);

  return true;
}
//==========================================================================*

//==========================================================================*
// Called for every track change or new race.
//--------------------------------------------------------------------------*
int TClothoidLane::GetWeather() const
{
  const TSection& Sec = (*oTrack)[0];
  const tTrackSeg* Seg = Sec.Seg;
  return (int) (100000 * Seg->surface->kFriction);
};
//==========================================================================*

int TClothoidLane::DumpFile(const char *path, std::string &out) const
{
  std::ifstream f(path, std::ios::binary);

  if (!f.is_open())
    return -1;

  out = std::string(std::istreambuf_iterator<char>(f), {});
  return 0;
}

int TClothoidLane::ParseVec3d(const cJSON *c, const char *key, TVec3d &out) const
{
  struct field
  {
    const char *key;
    double &value;
  } fields[] =
  {
    {"x", out.x},
    {"y", out.y},
    {"z", out.z}
  };

  const cJSON *ck = cJSON_GetObjectItem(c, key);

  if (!ck)
  {
    LogSimplix.error("Failed to get key \"%s\"\n", key);
    return -1;
  }

  for (size_t i = 0; i < sizeof fields / sizeof *fields; i++)
  {
    const struct field &f = fields[i];
    const cJSON *cf = cJSON_GetObjectItem(ck, f.key);

    if (!cf)
    {
      LogSimplix.error("Failed to get key \"%s\"\n", f.key);
      return -1;
    }
    else if (!cJSON_IsNumber(cf))
    {
      LogSimplix.error("Key \"%s\" not a number\n", f.key);
      return -1;
    }

    f.value = cJSON_GetNumberValue(cf);
  }

  return 0;
}

int TClothoidLane::ReadPoint(const cJSON *p, TPathPt &out) const
{
  struct field
  {
    const char *key;
    float &value;
  } fields[] =
  {
    {"Offset", out.Offset},
    {"Crv", out.Crv},
    {"CrvZ", out.CrvZ},
    {"NextCrv", out.NextCrv},
    {"WToL", out.WToL},
    {"WToR", out.WToR},
    {"WPitToL", out.WPitToL},
    {"WPitToR", out.WPitToR},
    {"MaxSpeed", out.MaxSpeed},
    {"AccSpd", out.AccSpd},
    {"Speed", out.Speed},
    {"FlyHeight", out.FlyHeight}
  };

  for (size_t i = 0; i < sizeof fields / sizeof *fields; i++)
  {
    const field &f = fields[i];
    const cJSON *it = cJSON_GetObjectItem(p, f.key);

    if (!it)
    {
      LogSimplix.error("Failed to find key \"%s\"\n", f.key);
      return -1;
    }
    else if (!cJSON_IsNumber(it))
    {
      LogSimplix.error("Key \"%s\" not a number\n", f.key);
      return -1;
    }

    f.value = cJSON_GetNumberValue(it);
  }

  if (ParseVec3d(p, "Center", out.Center))
  {
    LogSimplix.error("Failed to parse \"Center\"\n");
    return -1;
  }
  else if (ParseVec3d(p, "Point", out.Point))
  {
    LogSimplix.error("Failed to parse \"Point\"\n");
    return -1;
  }

  const cJSON *fix = cJSON_GetObjectItem(p, "Fix");

  if (!fix)
  {
    LogSimplix.error("Failed to find key \"Fix\"\n");
    return -1;
  }
  else if (!cJSON_IsNumber(fix))
  {
    LogSimplix.error("Key \"Fix\" not a number\n");
    return -1;
  }

  out.Fix = cJSON_GetNumberValue(fix);
  return 0;
}

//==========================================================================*
// Save path points
//--------------------------------------------------------------------------*
bool TClothoidLane::LoadPointsFromFile(const char* TrackLoad)
{
  bool ret = false;
  cJSON *c = nullptr;
  const cJSON *cweather, *points;
  int weather;
  std::string s;
  std::vector<TPathPt> vec;

  if (DumpFile(TrackLoad, s))
    goto end;
  else if (!(c = cJSON_Parse(s.c_str())))
  {
    LogSimplix.error("Failed to parse %s\n", TrackLoad);
    goto end;
  }
  else if (!(cweather = cJSON_GetObjectItem(c, "weather")))
  {
    LogSimplix.error("%s: failed to find \"weather\"\n", TrackLoad);
    goto end;
  }

  weather = cJSON_GetNumberValue(cweather);

  if (weather != GetWeather())
  {
    LogSimplix.error("%s: expected weather %d, got %d\n", TrackLoad,
      GetWeather(), weather);
    goto end;
  }
  else if (!(points = cJSON_GetObjectItem(c, "points")))
  {
    LogSimplix.error("%s: failed to find \"points\"\n", TrackLoad);
    goto end;
  }
  else if (!cJSON_IsArray(points))
  {
    LogSimplix.error("%s: \"points\" not an array\n", TrackLoad);
    goto end;
  }

  for (int i = 0; i < cJSON_GetArraySize(points); i++)
  {
    const cJSON *p = cJSON_GetArrayItem(points, i);
    TPathPt pt((*oTrack)[i]);

    if (ReadPoint(p, pt))
    {
      LogSimplix.error("%s: failed to read point %d\n", TrackLoad, i);
      goto end;
    }

    vec.push_back(pt);
  }

  oPathPoints = vec;
  ret = true;

end:

  if (c)
    cJSON_Delete(c);

  return ret;
}
//==========================================================================*

int TClothoidLane::WriteVec3d
  (cJSON *c, const char *key, const TVec3d &out) const
{
  struct field
  {
    const char *key;
    const double &value;
  } fields[] =
  {
    {"x", out.x},
    {"y", out.y},
    {"z", out.z}
  };

  cJSON *o = cJSON_CreateObject();

  if (!o)
  {
    LogSimplix.error("cJSON_CreateObject failed\n");
    goto failure;
  }

  for (size_t i = 0; i < sizeof fields / sizeof *fields; i++)
  {
    const struct field &f = fields[i];

    if (!cJSON_AddNumberToObject(o, f.key, f.value))
    {
      LogSimplix.error("Failed to set key \"%s\"\n", f.key);
      goto failure;
    }
  }

  if (!cJSON_AddItemToObject(c, key, o))
  {
    LogSimplix.error("cJSON_AddItemToObject failed\n");
    goto failure;
  }

  return 0;

failure:

  if (o)
    cJSON_Delete(o);

  return -1;
}

int TClothoidLane::StorePoint(const TPathPt &p, cJSON *array) const
{
  struct field
  {
    const char *key;
    const float &value;
  } fields[] =
  {
    {"Offset", p.Offset},
    {"Crv", p.Crv},
    {"CrvZ", p.CrvZ},
    {"NextCrv", p.NextCrv},
    {"WToL", p.WToL},
    {"WToR", p.WToR},
    {"WPitToL", p.WPitToL},
    {"WPitToR", p.WPitToR},
    {"MaxSpeed", p.MaxSpeed},
    {"AccSpd", p.AccSpd},
    {"Speed", p.Speed},
    {"FlyHeight", p.FlyHeight}
  };

  cJSON *it = cJSON_CreateObject();

  if (!it)
  {
    LogSimplix.error("cJSON_CreateObject failed\n");
    goto failure;
  }

  for (size_t i = 0; i < sizeof fields / sizeof *fields; i++)
  {
    const struct field &f = fields[i];

    if (!cJSON_AddNumberToObject(it, f.key, f.value))
    {
      LogSimplix.error("Failed to store \"%s\"\n", f.key);
      goto failure;
    }
  }

  if (!cJSON_AddNumberToObject(it, "Fix", p.Fix))
  {
    LogSimplix.error("Failed to store \"Fix\"\n");
    goto failure;
  }
  else if (!cJSON_AddItemToArray(array, it))
  {
    LogSimplix.error("Failed to add point to array\n");
    goto failure;
  }
  else if (WriteVec3d(it, "Center", p.Center))
  {
    LogSimplix.error("Failed to add Center\n");
    goto failure;
  }
  else if (WriteVec3d(it, "Point", p.Point))
  {
    LogSimplix.error("Failed to add Point\n");
    goto failure;
  }

  return 0;

failure:

  if (it)
    cJSON_Delete(it);

  return -1;
}

//==========================================================================*
// Save path points
//--------------------------------------------------------------------------*
bool TClothoidLane::SavePointsToFile(const char* TrackLoad) const
{
  bool ret = false;
  cJSON *c = nullptr, *array = nullptr;
  char *print = nullptr;
  std::ofstream f(TrackLoad, std::ios::binary);

  if (!f.is_open())
  {
    LogSimplix.error("Could not load %s for writing\n", TrackLoad);
    goto end;
  }
  else if (!(c = cJSON_CreateObject()))
  {
    LogSimplix.error("cJSON_CreateObject failed\n");
    goto end;
  }
  else if (!cJSON_AddNumberToObject(c, "weather", GetWeather()))
  {
    LogSimplix.error("Failed to store \"weather\"\n");
    goto end;
  }
  else if (!(array = cJSON_AddArrayToObject(c, "points")))
  {
    LogSimplix.error("Failed to array \"points\"\n");
    goto end;
  }

  for (const auto &p : oPathPoints)
    if (StorePoint(p, array))
    {
      LogSimplix.error("Failed to store point\n");
      goto end;
    }

  if (!(print = cJSON_Print(c)))
  {
    LogSimplix.error("Failed to print JSON point data\n");
    goto end;
  }

  f.write(print, ::strlen(print));

  if (!f.good())
  {
    LogSimplix.error("Failed to write JSON point data\n");
    goto end;
  }

  ret = true;

end:

  if (c)
    cJSON_Delete(c);

  cJSON_free(print);
  return ret;
}
//==========================================================================*

//==========================================================================*
// Clear old racingline
//--------------------------------------------------------------------------*
void TClothoidLane::ClearRacingline(const char* TrackLoad)
{
  if (::remove(TrackLoad))
    LogSimplix.error("Failed to remove %s: %s\n", TrackLoad, strerror(errno));
}
//==========================================================================*

//==========================================================================*
// end of file unitclothoid.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
