//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unittmpcarparam.h
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter f�r TORCS-Version 1.3.0
// Zeitlich variable Parameter des Fahrzeugs
//
// Datei    : unittmpcarparam.h
// Created      : 2007.11.25
// Stand        : 2008.12.06
// Copyright    : � 2007-2008 Wolf-Dieter Beelitz
// eMail        : wdb@wdbee.de
// Version      : 2.00.000
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
#ifndef _UNITTMPCARPARAM_H_
#define _UNITTMPCARPARAM_H_

#include "unitglobal.h"
#include "unitcommon.h"

//==========================================================================*
// Deklaration der Klasse TTmpCarParam
//--------------------------------------------------------------------------*
class TTmpCarParam
{
  private:

  public:
    PtCarElt oCar;                               // Pointer to TORCS data of car

	TTmpCarParam();                              // Default constructor
	~TTmpCarParam();                             // Destructor

	void Initialize(PtCarElt Car);
    bool Needed();
	void Update();

  public:
	double oDamage;                              // Damage of this car
	double oEmptyMass;                           // Mass of car wihtout fuel
	double oFuel;                                // Mass of fuel in car
	double oMass;                                // Mass of car.with fuel
	double oSkill;                               // Skilling

};
//==========================================================================*
#endif // _UNITTMPCARPARAM_H_
//--------------------------------------------------------------------------*
// end of file unittmpcarparam.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
