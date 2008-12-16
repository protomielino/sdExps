//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitcubicspline.h
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter f�r TORCS-Version 1.3.0
// Zentrale Klasse f�r das Fahren bzw. den Fahrer/Roboter
//
// Datei    : unitcubicspline.h
// Erstellt : 25.11.2007
// Stand    : 24.11.2008
// Copyright: � 2007-2008 Wolf-Dieter Beelitz
// eMail    : wdb@wdbee.de
// Version  : 1.01.000
//--------------------------------------------------------------------------*
// Ein erweiterter TORCS-Roboters
//--------------------------------------------------------------------------*
// Diese Unit basiert auf dem Roboter mouse_2006
//
//    Copyright: (C) 2006-2007 Tim Foden
//
//--------------------------------------------------------------------------*
// Diese Version wurde mit MS Visual C++ 2005 Express Edition entwickelt.
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
#ifndef _UNITCUBICSPLINE_H_
#define _UNITCUBICSPLINE_H_

#include "unitcubic.h"

//==========================================================================*
// Deklaration der Klasse TCubicSpline
//--------------------------------------------------------------------------*
class TCubicSpline  
{
  public:
	TCubicSpline                                 // Constructor
	  (int Count, const double* X, const double* Y, const double* S);
	~TCubicSpline();

	double CalcOffset(double X) const;           // Get offset
	double CalcGradient(double X) const;         // Get gradient

	bool IsValidX(double X) const;               // Is x valid in spline?

  private:
	int	FindSeg(double X) const;                 // Find seg to x

  private:
	int	oCount;                                  // Nbr of segemnts
	double*	oSegs;                               // Segments
	TCubic*	oCubics;                             // Cubics
};
//==========================================================================*
#endif // _UNITCUBICSPLINE_H_
//--------------------------------------------------------------------------*
// end of file unitcubicspline.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
