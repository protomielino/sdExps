/***************************************************************************

    file                 : steer.cpp
    created              : Sun Mar 19 00:08:20 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr

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

void
SimSteerConfig(tCar *car)
{
	void *hdle = car->params;
	tCarSetupItem *setupStLock = &(car->carElt->setup.steerLock);

	setupStLock->desired_value = setupStLock->min = setupStLock->max = 0.43f;
	GfParmGetNumWithLimits(hdle, SECT_STEER, PRM_STEERLOCK, (char*)NULL, &(setupStLock->desired_value), &(setupStLock->min), &(setupStLock->max));
	setupStLock->changed = true;
	setupStLock->stepsize = DEG2RAD(1.0);

	tCarSetupItem *setupStWheelRot = &(car->carElt->setup.steerWheelRot);

	setupStWheelRot->desired_value = setupStWheelRot->min = setupStWheelRot->max = 2.0f;
	GfParmGetNumWithLimits(hdle, SECT_STEER, PRM_STEERROT, (char *)NULL, &(setupStWheelRot->desired_value), &(setupStWheelRot->min), &(setupStWheelRot->max));
	setupStWheelRot->changed = true;
	setupStWheelRot->stepsize = DEG2RAD(1.0);

	car->steer.maxSpeed  = GfParmGetNum(hdle, SECT_STEER, PRM_STEERSPD, (char*)NULL, 1.0f);
}


void
SimSteerReConfig(tCar *car)
{/* called by SimCarReConfig in car.cpp */
	tCarSetupItem *setupStLock = &(car->carElt->setup.steerLock);

	if (setupStLock->changed) {
		car->steer.steerLock = MIN(setupStLock->max, MAX(setupStLock->min, setupStLock->desired_value));
		car->carElt->_steerLock = car->steer.steerLock;
		setupStLock->value = car->steer.steerLock;
		setupStLock->changed = false;
	}

	tCarSetupItem *setupStWheelRot = &(car->carElt->setup.steerWheelRot);

	if (setupStWheelRot->changed) {
		car->steer.steerWheelRot = MIN(setupStWheelRot->max, MAX(setupStWheelRot->min, setupStWheelRot->desired_value));
		car->carElt->_steerWheelRot = car->steer.steerWheelRot;
		setupStWheelRot->value = car->steer.steerWheelRot;
		setupStWheelRot->changed = false;
	}
}


void
SimSteerUpdate(tCar *car)
{
	tdble steer, steer2;
	tdble stdelta;
	tdble tanSteer;

	/* input control */
	steer = car->ctrl->steer;
	steer *= car->steer.steerLock;
	stdelta = steer - car->steer.steer;

	if ((fabs(stdelta) / SimDeltaTime) > car->steer.maxSpeed) {
		steer = (float)(SIGN(stdelta) * car->steer.maxSpeed * SimDeltaTime + car->steer.steer);
	}

	car->steer.steer = steer;
	tanSteer = fabs(tan(steer));
	steer2 = atan2((car->wheelbase * tanSteer) , (car->wheelbase - tanSteer * car->wheeltrack));

	if (steer > 0) {
		car->wheel[FRNT_RGT].torques.x =
		car->wheel->cosax * (steer2 - car->wheel[FRNT_RGT].steer) * car->wheel[FRNT_RGT].prespinVel * car->wheel[FRNT_RGT].I / SimDeltaTime;
		car->wheel[FRNT_RGT].steer = steer2;
		car->wheel[FRNT_LFT].torques.x =
		car->wheel->cosax * (steer - car->wheel[FRNT_LFT].steer) * car->wheel[FRNT_LFT].prespinVel * car->wheel[FRNT_LFT].I / SimDeltaTime;
		car->wheel[FRNT_LFT].steer = steer;
	} else {
		car->wheel[FRNT_RGT].torques.x =
		car->wheel->cosax * (steer - car->wheel[FRNT_RGT].steer) * car->wheel[FRNT_RGT].prespinVel * car->wheel[FRNT_RGT].I / SimDeltaTime;
		car->wheel[FRNT_RGT].steer = steer;
		car->wheel[FRNT_LFT].torques.x =
		car->wheel->cosax * (-steer2 - car->wheel[FRNT_LFT].steer) * car->wheel[FRNT_LFT].prespinVel * car->wheel[FRNT_LFT].I / SimDeltaTime;
		car->wheel[FRNT_LFT].steer = -steer2;
	}
}
