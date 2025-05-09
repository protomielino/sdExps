/***************************************************************************

    file        : openglconfig.cpp
    created     : Fri Jun 3 12:52:07 CET 2004
    copyright   : (C) 2005 Bernhard Wymann
    email       : berniw@bluewin.ch

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

*/

#ifndef _OPENGLCONFIG_H_
#define _OPENGLCONFIG_H_

#include "confscreens.h"


extern void *OpenGLMenuInit(void *prevMenu);

extern void OpenGLLoadSelectedFeatures();
extern void OpenGLStoreSelectedFeatures();

#endif // _OPENGLCONFIG_H_
