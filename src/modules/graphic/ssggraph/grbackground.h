/***************************************************************************

    file                 : grbackground.h
    created              : Thu Nov 25 21:09:40 CEST 2010
    copyright            : (C) 2010 by Jean-Philippe Meuret
    email                : http://www.speed-dreams.org

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GRBACKGROUND_H_
#define _GRBACKGROUND_H_

struct Situation;

namespace ssggraph {

#define SGD_2PI      6.28318530717958647692

class cGrCamera;
class cGrBackgroundCam;

void grUpdateLight(void);
void grUpdateFogColor(double sol_angle);

//! Public interface
extern void grLoadBackgroundGraphicsOptions();
extern void grLoadBackground();
extern void grLoadBackgroundSky(void);
extern void grInitBackground();
extern void grUpdateSky(double currentTime, double accelTime);
extern void grPreDrawSky(struct Situation* s, float fogStart, float fogEnd, class cGrCamera *cam);
extern void grPostDrawSky();
extern void grPreDrawBackgroundSky(class cGrCamera *cam);
extern void grDrawBackgroundSky(void);
extern void grDrawStaticBackground(class cGrCamera *cam, class cGrBackgroundCam *bgCam);
extern void grShutdownBackground(void);

extern unsigned grSkyDomeDistance; // 0 means no sky dome (static background).
extern const tdble grSkyDomeNeutralFOVDistance;

} // namespace ssggraph

#endif //_GRBACKGROUND_H_
