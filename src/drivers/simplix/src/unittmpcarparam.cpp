//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unittmpcarparam.cpp
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter f�r TORCS-Version 1.3.0
// Zeitlich variable Parameter des Fahrzeugs
//
// Datei    : unittmpcarparam.cpp
// Erstellt : 25.11.2007
// Stand    : 24.11.2008
// Copyright: � 2007-2008 Wolf-Dieter Beelitz
// eMail    : wdb@wdbee.de
// Version  : 1.01.000
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

#include "unitcarparam.h"
#include "unitparam.h"

//==========================================================================*
// Default constructor
//--------------------------------------------------------------------------*
TTmpCarParam::TTmpCarParam():
  oDamage(0),
  oEmptyMass(1000.0),
  oFuel(0),
  oMass(1000.0),
  oSkill(1.0)
{
}
//==========================================================================*

//==========================================================================*
// Destructor
//--------------------------------------------------------------------------*
TTmpCarParam::~TTmpCarParam()
{
}
//===========================================================================

//==========================================================================*
// Initialize
//--------------------------------------------------------------------------*
void TTmpCarParam::Initialize(PtCarElt Car)
{
  oCar = Car;
}
//==========================================================================*

//==========================================================================*
// Recalculation needed?
//--------------------------------------------------------------------------*
bool TTmpCarParam::Needed()
{
  if ((fabs(oFuel - CarFuel) > 5)
    || (fabs(oDamage - CarDamage) > 250))
	return true;
  else
	return false;
}
//==========================================================================*

//==========================================================================*
// Update
//--------------------------------------------------------------------------*
void TTmpCarParam::Update()
{
  oFuel = 5 * floor(CarFuel/5);
  oMass = oEmptyMass + oFuel;
  oDamage = CarDamage;
}
//==========================================================================*

//--------------------------------------------------------------------------*
// end of file unittmpcarparam.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
