/***************************************************************************

    file                 : atmosphere.cpp
    created              : Sun Feb 12 17:37:50 CET 2017
    copyright            : (C) 2017-2017 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: atmosphere.cpp,v 1.1.2.1 2017/02/12 17:52:38 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sim.h"

static int SimClouds = 0;

void SimAtmosphereConfig(tTrack *track)
{
    SimRain = track->local.rain;
    SimTimeOfDay = track->local.timeofday;
    SimClouds = track->local.clouds;

    if (SimTimeOfDay < 6.00 * 60 *60 && SimTimeOfDay > 19 * 60 * 60)
        Tair -= 6.75;
    else if (SimTimeOfDay > 6.00 * 60 * 60 && SimTimeOfDay < 10 * 60 * 60)
        Tair += 5.75;
    else if (SimTimeOfDay > 10.00 * 60 * 60 && SimTimeOfDay < 14.00 * 60 *60)
        Tair += 10.25;
    else if (SimTimeOfDay > 14.00 * 60 * 60 && SimTimeOfDay < 16.00 * 60 * 60)
        Tair += 15.75;
    else if (SimTimeOfDay > 16.00 * 60 * 60 && SimTimeOfDay < 18.00 * 60 * 60)
        Tair += 20.75;
    else Tair +=5.75;

    if (SimClouds == 0 || SimClouds == 1)
        Tair = Tair;
    else if (SimClouds == 2 || SimClouds == 3 )
        Tair -= 1.55;
    else if (SimClouds == 4 )
        Tair -= 3.05;
    else
        Tair -= 5.75;

    if(SimRain == 0)
        Tair = Tair;
    else if (SimRain == 1)
        Tair -= 1.55;
    else if (SimRain == 2)
        Tair -= 2.75;
    else
        Tair -= 3.50;
    //Tair = 297.15;  // 24 degree celsius
}


void SimAtmosphereUpdate(tCar *car, tSituation *s)
{
    // TODO: get this later form the situation, weather simulation.
    //car->localTemperature = 273.15f + 20.0f;
    //car->localPressure = 101300.0f;
}
