//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitparam.cpp
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter f�r TORCS-Version 1.3.0
// Parameter des Fahrzeugs, der Fahrspuren, der Box usw.
//
// Datei    : unitparam.cpp
// Erstellt : 11.04.2008
// Stand    : 06.12.2008
// Copyright: � 2007-2008 Wolf-Dieter Beelitz
// eMail    : wdb@wdbee.de
// Version  : 2.00.000
//--------------------------------------------------------------------------*
// Ein erweiterter TORCS-Roboters
//--------------------------------------------------------------------------*
// Das Programm wurde unter Windows XP entwickelt und getestet.
// Fehler sind nicht bekannt, dennoch gilt:
// Wer die Dateien verwendet erkennt an, dass f�r Fehler, Sch�den,
// Folgefehler oder Folgesch�den keine Haftung �bernommen wird.
//
// Im �brigen gilt f�r die Nutzung und/oder Weitergabe die
// GNU GPL (General Public License)
// Version 2 oder nach eigener Wahl eine sp�tere Version.
//--------------------------------------------------------------------------*
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//--------------------------------------------------------------------------*
#include <math.h>

#include "unitglobal.h"
#include "unitcommon.h"

#include "unitparam.h"

//==========================================================================*
// Default constructor
//--------------------------------------------------------------------------*
TParam::TParam()
{
  Fix.oTmpCarParam = &Tmp;
}
//==========================================================================*

//==========================================================================*
// Destructor
//--------------------------------------------------------------------------*
TParam::~TParam()
{
}
//===========================================================================

//==========================================================================*
// Initialize
//--------------------------------------------------------------------------*
void TParam::Initialize(PtCarElt Car)
{
  oCar = Car;

  Tmp.Initialize(Car);                           // State of the car
  Fix.Initialize(Car);                           // Data of the car
}
//==========================================================================*

//==========================================================================*
// Set Mass of car without fuel
//--------------------------------------------------------------------------*
void TParam::SetEmptyMass(float EmptyMass)
{
  Tmp.oEmptyMass = EmptyMass;
  Fix.oEmptyMass = EmptyMass;
}
//==========================================================================*

//==========================================================================*
// Update
//--------------------------------------------------------------------------*
void TParam::Update()
{
  Tmp.Update();                                  // Update state of car data
}
//==========================================================================*

//--------------------------------------------------------------------------*
// end of file unitparam.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
