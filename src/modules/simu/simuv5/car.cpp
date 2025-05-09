/***************************************************************************

    file                 : car.cpp
    created              : Sun Mar 19 00:05:43 CET 2000
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

#include <cstring>
#include <cstdio>

#include "sim.h"

const tdble aMax = 1.0f; /*  */
static const char *SuspSect[4] = {SECT_FRNTRGTSUSP, SECT_FRNTLFTSUSP, SECT_REARRGTSUSP, SECT_REARLFTSUSP};

// The "Simu" logger instance
GfLogger* PLogSimu = 0;

static bool
enable_tyreset(const tCar *car, const tCarSetup *setup)
{
    return car->options->tyre_temperature;
}

static bool
enable_compound_set(const tCar *car, const tCarSetup *setup)
{
    return (car->features & FEAT_COMPOUNDS) && car->options->compounds;
}

static bool
enable_front_wing_angle(const tCar *car, const tCarSetup *setup)
{
    return car->wing[0].WingType != -1
        && setup->wingAngle[0].min != setup->wingAngle[0].max;
}

static bool
enable_rear_wing_angle(const tCar *car, const tCarSetup *setup)
{
    return car->wing[1].WingType != -1
        && setup->wingAngle[1].min != setup->wingAngle[1].max;
}

void
SimCarConfig(tCar *car)
{
    void	*hdle = car->params;
    tdble	k;
    t3Dd	Irot;
    tdble	w;
    tdble	gcfrl, gcrrl, gcfr, gcrl;
    tdble	wf0, wr0;
    tdble	overallwidth;
    int		i;
    tCarElt	*carElt = car->carElt;

    car->options = new SimulationOptions;
    car->options->SetFromSkill (carElt->_skillLevel);
    //car->options->LoadFromFile (hdle);

    car->fuel_time = 0.0;
    car->fuel_consumption = 0.0;
    car->carElt->_fuelTotal = 0.0;
    car->carElt->_fuelInstant = 10.0;

    const char *enabling;
    tCarSetupItem *setupGcfr = &(car->carElt->setup.FRWeightRep);
    tCarSetupItem *setupGcfrl = &(car->carElt->setup.FRLWeightRep);
    tCarSetupItem *setupGcrrl = &(car->carElt->setup.RRLWeightRep);
    tCarSetupItem *setupFuel = &(car->carElt->setup.fuel);

    /* get features first */
    car->features = 0;
    car->TCL_BrakeScale = 125.0f;;
    car->TCL_AccelScale = 0.9f;
    car->TCL_SlipScale = 1.0f;
    car->ABS_SlipScale = 0.1f;
    car->ABS_BrakeScale = 1.0f;

    enabling = GfParmGetStr(hdle, SECT_FEATURES, PRM_AEROTOCG, VAL_NO);
    if (strcmp(enabling, VAL_YES) == 0)
    {
        car->features = car->features | FEAT_AEROTOCG;
        GfLogInfo("SimuV5 feature AEROTOCG on\n");
    }

    enabling = GfParmGetStr(hdle, SECT_FEATURES, PRM_SLOWGRIP, VAL_NO);
    if (strcmp(enabling, VAL_YES) == 0)
    {
        car->features = car->features | FEAT_SLOWGRIP;
        GfLogInfo("SimuV5 feature SLOW GRIP on\n");
    }

    enabling = GfParmGetStr(hdle, SECT_FEATURES, PRM_REALGEARCHANGE, VAL_NO);
    if (strcmp(enabling, VAL_YES) == 0)
    {
        car->features = car->features | FEAT_REALGEARCHANGE;
        GfLogInfo("SimuV5 feature REAL GEAR CHANGE on\n");
    }

    enabling = GfParmGetStr(hdle, SECT_FEATURES, PRM_REVLIMIT, VAL_NO);
    if (strcmp(enabling, VAL_YES) == 0)
    {
        car->features = car->features | FEAT_REVLIMIT;
        GfLogInfo("SimuV5 feature REV LIMIT on\n");
    }

    enabling = GfParmGetStr(hdle, SECT_FEATURES, PRM_TIRETEMPDEG, VAL_NO);
    if (strcmp(enabling, VAL_YES) == 0)
    {
        car->features = car->features | FEAT_TIRETEMPDEG;
    }

    enabling = GfParmGetStr(hdle, SECT_FEATURES, PRM_TIRECOMPOUNDS, VAL_NO);
    if (strcmp(enabling, VAL_YES) == 0)
    {
        car->features = car->features | FEAT_COMPOUNDS;
        GfLogInfo("SimuV5 feature COMPOUNDS on\n");
    }

    enabling = GfParmGetStr(hdle, SECT_FEATURES, PRM_FIXEDWHEELFORCE, VAL_NO);
    if (strcmp(enabling, VAL_YES) == 0)
    {
        car->features = car->features | FEAT_FIXEDWHEELFORCE;
        GfLogInfo("SimuV5 feature FIXED WHEEL FORCE on\n");
    }

    enabling = GfParmGetStr(hdle, SECT_FEATURES, PRM_TCLINSIMU, VAL_NO);
    if (strcmp(enabling, VAL_YES) == 0)
    {
        car->features = car->features | FEAT_TCLINSIMU;
        GfLogInfo("SimuV5 feature TCL SIMU on\n");
    }

    enabling = GfParmGetStr(hdle, SECT_FEATURES, PRM_ABSINSIMU, VAL_NO);
    if (strcmp(enabling, VAL_YES) == 0)
    {
        car->features = car->features | FEAT_ABSINSIMU;
        GfLogInfo("SimuV5 feature ABS SIMU on\n");
    }

    enabling = GfParmGetStr(hdle, SECT_FEATURES, PRM_ESPINSIMU, VAL_NO);
    if (strcmp(enabling, VAL_YES) == 0)
    {
        car->features = car->features | FEAT_ESPINSIMU;
        GfLogInfo("SimuV5 feature ESP IMU on\n");
    }

    enabling = GfParmGetStr(hdle, SECT_FEATURES, PRM_LIMITEDGROUNDEFFECT, VAL_NO);
    if (strcmp(enabling, VAL_YES) == 0)
    {
        car->features = car->features | FEAT_LIMITEDGROUNDEFFECT;
        GfLogInfo("SimuV5 feature LIMITED GROUND EFFECT on\n");
    }

    /* continue with car parameters */
    setupGcfr->desired_value = setupGcfr->min = setupGcfr->max = 0.5;
    GfParmGetNumWithLimits(hdle, SECT_CAR, PRM_FRWEIGHTREP, (char*)NULL, &(setupGcfr->desired_value), &(setupGcfr->min), &(setupGcfr->max));
    setupGcfr->changed = true;
    setupGcfr->stepsize = 0.005f;
    gcfr = setupGcfr->desired_value;

    setupGcfrl->desired_value = setupGcfrl->min = setupGcfrl->max = 0.5;
    GfParmGetNumWithLimits(hdle, SECT_CAR, PRM_FRLWEIGHTREP, (char*)NULL, &(setupGcfrl->desired_value), &(setupGcfrl->min), &(setupGcfrl->max));
    setupGcfrl->changed = true;
    setupGcfrl->stepsize = 0.005f;
    gcfrl = setupGcfrl->desired_value;

    setupGcrrl->desired_value = setupGcrrl->min = setupGcrrl->max = 0.5;
    GfParmGetNumWithLimits(hdle, SECT_CAR, PRM_RRLWEIGHTREP, (char*)NULL, &(setupGcrrl->desired_value), &(setupGcrrl->min), &(setupGcrrl->max));
    setupGcrrl->changed = true;
    setupGcrrl->stepsize = 0.005f;
    gcrrl = setupGcrrl->desired_value;

    gcrl = gcfr * gcfrl + (1 - gcfr) * gcrrl;

    car->tank        = GfParmGetNum(hdle, SECT_CAR, PRM_TANK, (char*)NULL, 80);
    if (car->fuel > car->tank)
    {
        car->fuel = car->tank;
    }
    car->fuel_prev = car->fuel;

    car->fuelMass        = GfParmGetNum(hdle, SECT_CAR, PRM_FUELMASS, (char*)NULL, 1.0);

    setupFuel->desired_value = setupFuel->min = setupFuel->max = 80.0;
    GfParmGetNumWithLimits(hdle, SECT_CAR, PRM_FUEL, (char*)NULL, &(setupFuel->desired_value), &(setupFuel->min), &(setupFuel->max));
    setupFuel->min = 0.0;
    setupFuel->max = car->tank;
    setupFuel->changed = true;
    setupFuel->stepsize = 1.0;

    car->dimension.x = GfParmGetNum(hdle, SECT_CAR, PRM_LEN, (char*)NULL, 4.7f);
    car->dimension.y = GfParmGetNum(hdle, SECT_CAR, PRM_WIDTH, (char*)NULL, 1.9f);
    overallwidth     = GfParmGetNum(hdle, SECT_CAR, PRM_OVERALLWIDTH, (char*)NULL, car->dimension.y);
    car->dimension.z = GfParmGetNum(hdle, SECT_CAR, PRM_HEIGHT, (char*)NULL, 1.2f);
    car->mass        = GfParmGetNum(hdle, SECT_CAR, PRM_MASS, (char*)NULL, 1500);
    car->Minv        = (tdble) (1.0 / car->mass);
    car->statGC.y    = (tdble) (- gcrl * car->dimension.y + car->dimension.y / 2.0);
    car->statGC.z    = GfParmGetNum(hdle, SECT_CAR, PRM_GCHEIGHT, (char*)NULL, .5);
    k                = GfParmGetNum(hdle, SECT_CAR, PRM_CENTR, (char*)NULL, 1.0);
    carElt->_drvPos_x = GfParmGetNum(hdle, SECT_DRIVER, PRM_XPOS, (char*)NULL, 0.0);
    carElt->_drvPos_y = GfParmGetNum(hdle, SECT_DRIVER, PRM_YPOS, (char*)NULL, 0.0);
    carElt->_drvPos_z = GfParmGetNum(hdle, SECT_DRIVER, PRM_ZPOS, (char*)NULL, 0.0);
    carElt->_bonnetPos_x = GfParmGetNum(hdle, SECT_BONNET, PRM_XPOS, (char*)NULL, carElt->_drvPos_x);
    carElt->_bonnetPos_y = GfParmGetNum(hdle, SECT_BONNET, PRM_YPOS, (char*)NULL, carElt->_drvPos_y);
    carElt->_bonnetPos_z = GfParmGetNum(hdle, SECT_BONNET, PRM_ZPOS, (char*)NULL, carElt->_drvPos_z);

    /* rotational inertias */
    k = k * k;
    Irot.x = (car->mass * k * (car->dimension.y * car->dimension.y + car->dimension.z * car->dimension.z)) / 12.0;
    Irot.y = (car->mass * k * (car->dimension.x * car->dimension.x + car->dimension.z * car->dimension.z)) / 12.0;
    Irot.z = (car->mass * k * (car->dimension.y * car->dimension.y + car->dimension.x * car->dimension.x)) / 12.0;
    Irot.x = GfParmGetNum(hdle, SECT_CAR, PRM_ROLLROTINERTIA, (char*)NULL, Irot.x);
    Irot.y = GfParmGetNum(hdle, SECT_CAR, PRM_PITCHROTINERTIA, (char*)NULL, Irot.y);
    Irot.z = GfParmGetNum(hdle, SECT_CAR, PRM_YAWROTINERTIA, (char*)NULL, Irot.z);
    car->Iinv.x = (tdble) (1.0 / Irot.x);
    car->Iinv.y = (tdble) (1.0 / Irot.y);
    car->Iinv.z = (tdble) (1.0 / Irot.z);

    /* configure components */
    tCarSetupItem *setupSpring;
    tdble K[4], Kfheave, Krheave;

    for (i = 0; i < 4; i++)
    {
        setupSpring = &(car->carElt->setup.suspSpring[i]);
        setupSpring->desired_value = setupSpring->min = setupSpring-> max = 175000.0f;
        GfParmGetNumWithLimits(hdle, SuspSect[i], PRM_SPR, (char*)NULL, &(setupSpring->desired_value), &(setupSpring->min), &(setupSpring->max));
        setupSpring->changed = true;
        setupSpring->stepsize = 1000;
        K[i] = setupSpring->desired_value;
    }

    setupSpring = &(car->carElt->setup.heaveSpring[0]);
    setupSpring->desired_value = setupSpring->min = setupSpring-> max = 0.0f;
    GfParmGetNumWithLimits(hdle, SECT_FRNTHEAVE, PRM_SPR, (char*)NULL, &(setupSpring->desired_value), &(setupSpring->min), &(setupSpring->max));
    setupSpring->changed = true;
    setupSpring->stepsize = 1000;
    Kfheave = setupSpring->desired_value;
    setupSpring = &(car->carElt->setup.heaveSpring[1]);
    setupSpring->desired_value = setupSpring->min = setupSpring-> max = 0.0f;
    GfParmGetNumWithLimits(hdle, SECT_REARHEAVE, PRM_SPR, (char*)NULL, &(setupSpring->desired_value), &(setupSpring->min), &(setupSpring->max));
    setupSpring->changed = true;
    setupSpring->stepsize = 1000;
    Krheave = setupSpring->desired_value;

    /* wheel force bugfix is needed for heave springs */
    if ( (Kfheave > 0.0f) || (Krheave > 0.0f) )
        {car->features = car->features | FEAT_FIXEDWHEELFORCE;}

    w = car->mass * G;

    wf0 = w * gcfr;
    wr0 = w * (1 - gcfr);

    car->wheel[FRNT_RGT].weight0 = wf0 * gcrl * K[FRNT_RGT] / (K[FRNT_RGT] + 0.5f*Kfheave);
    car->wheel[FRNT_LFT].weight0 = wf0 * (1 - gcrl) * K[FRNT_LFT] / (K[FRNT_LFT] + 0.5f*Kfheave);
    car->wheel[REAR_RGT].weight0 = wr0 * gcrl * K[REAR_RGT] / (K[REAR_RGT] + 0.5f*Krheave);
    car->wheel[REAR_LFT].weight0 = wr0 * (1 - gcrl) * K[REAR_LFT] / (K[REAR_LFT] + 0.5f*Krheave);

    for (i = 0; i < 2; i++)
    {
        SimAxleConfig(car, i);
    }

    for (i = 0; i < 4; i++)
    {
        SimWheelConfig(car, i);
    }

    SimEngineConfig(car);
    SimTransmissionConfig(car);
    SimSteerConfig(car);
    SimBrakeSystemConfig(car);
    SimAeroConfig(car);

    for (i = 0; i < 2; i++)
    {
        SimWingConfig(car, i);
    }

    /* Set the origin to GC */
    car->wheelbase = car->wheeltrack = 0;
    // Old code is misleading, because at this time staticPos.x still is relPos.x
    // car->statGC.x = car->wheel[FRNT_RGT].staticPos.x * gcfr + car->wheel[REAR_RGT].staticPos.x * (1 - gcfr);
    // Use the correct variable name instead
    car->statGC.x = car->wheel[FRNT_RGT].relPos.x * gcfr + car->wheel[REAR_RGT].relPos.x * (1 - gcfr);

    carElt->_dimension = car->dimension;
    carElt->_statGC = car->statGC;
    carElt->_tank = car->tank;
    for (i = 0; i < 4; i++)
    {
        carElt->priv.wheel[i].relPos = car->wheel[i].relPos;
    }

    for (i = 0; i < 4; i++)
    {
        car->wheel[i].staticPos.x -= car->statGC.x;
        car->wheel[i].staticPos.y -= car->statGC.y;
    }
    car->wheelbase = (tdble) ((car->wheel[FRNT_RGT].staticPos.x
                + car->wheel[FRNT_LFT].staticPos.x
                - car->wheel[REAR_RGT].staticPos.x
                - car->wheel[REAR_LFT].staticPos.x) / 2.0);
    car->wheeltrack = (tdble) ((-car->wheel[REAR_LFT].staticPos.y
                - car->wheel[FRNT_LFT].staticPos.y
                + car->wheel[FRNT_RGT].staticPos.y
                + car->wheel[REAR_RGT].staticPos.y) / 2.0);

    /* set corners pos */
    car->corner[FRNT_RGT].pos.x = (tdble) (car->dimension.x * .5 - car->statGC.x);
    car->corner[FRNT_RGT].pos.y = (tdble) (- overallwidth * .5 - car->statGC.y);
    car->corner[FRNT_RGT].pos.z = 0;

    car->corner[FRNT_LFT].pos.x = (tdble) (car->dimension.x * .5 - car->statGC.x);
    car->corner[FRNT_LFT].pos.y = (tdble) (overallwidth * .5 - car->statGC.y);
    car->corner[FRNT_LFT].pos.z = 0;

    car->corner[REAR_RGT].pos.x = (tdble) (- car->dimension.x * .5 - car->statGC.x);
    car->corner[REAR_RGT].pos.y = (tdble) (- overallwidth * .5 - car->statGC.y);
    car->corner[REAR_RGT].pos.z = 0;

    car->corner[REAR_LFT].pos.x = (tdble) (- car->dimension.x * .5 - car->statGC.x);
    car->corner[REAR_LFT].pos.y = (tdble) (overallwidth * .5 - car->statGC.y);
    car->corner[REAR_LFT].pos.z = 0;

    /* set wing positions */
    if (car->features & FEAT_AEROTOCG)
    {
        for (i = 0; i < 2; i++)
        {
            car->wing[i].staticPos.x -= car->statGC.x;
            car->wing[i].staticPos.y -= car->statGC.y;
            car->wing[i].staticPos.z -= car->statGC.z;
        }
    }

    SimCarReConfig(car);

    /* initialize dashboardInstant */
    tPrivCar *priv = &(car->carElt->priv);
    tCarSetup *setup = &(car->carElt->setup);

    for (i = 0; i < NR_DI_INSTANT; i++)
    {
        priv->dashboardInstant[i].type = DI_NONE;
        priv->dashboardInstant[i].setup = NULL;
    }

    i = 0;

    if (setup->brakeRepartition.min != setup->brakeRepartition.max)
    {
        priv->dashboardInstant[i].type = DI_BRAKE_REPARTITION;
        priv->dashboardInstant[i].setup = &(setup->brakeRepartition);
        i++;
    }

    if (setup->arbSpring[0].min != setup->arbSpring[0].max)
    {
        priv->dashboardInstant[i].type = DI_FRONT_ANTIROLLBAR;
        priv->dashboardInstant[i].setup = &(setup->arbSpring[0]);
        i++;
    }

    if (setup->arbSpring[1].min != setup->arbSpring[1].max)
    {
        priv->dashboardInstant[i].type = DI_REAR_ANTIROLLBAR;
        priv->dashboardInstant[i].setup = &(setup->arbSpring[1]);
        i++;
    }

    if (setup->differentialType[0] == DIFF_ELECTRONIC_LSD)
    {
        if (setup->differentialMaxSlipBias[0].min != setup->differentialMaxSlipBias[0].max)
        {
            priv->dashboardInstant[i].type = DI_FRONT_DIFF_MAX_SLIP_BIAS;
            priv->dashboardInstant[i].setup = &(setup->differentialMaxSlipBias[0]);
            i++;
        }

        if (setup->differentialCoastMaxSlipBias[0].min != setup->differentialCoastMaxSlipBias[0].max)
        {
            priv->dashboardInstant[i].type = DI_FRONT_DIFF_COAST_MAX_SLIP_BIAS;
            priv->dashboardInstant[i].setup = &(setup->differentialCoastMaxSlipBias[0]);
            i++;
        }
    }

    if (setup->differentialType[1] == DIFF_ELECTRONIC_LSD)
    {
        if (setup->differentialMaxSlipBias[1].min != setup->differentialMaxSlipBias[1].max)
        {
            priv->dashboardInstant[i].type = DI_REAR_DIFF_MAX_SLIP_BIAS;
            priv->dashboardInstant[i].setup = &(setup->differentialMaxSlipBias[1]);
            i++;
        }

        if (setup->differentialCoastMaxSlipBias[1].min != setup->differentialCoastMaxSlipBias[1].max) {
            priv->dashboardInstant[i].type = DI_REAR_DIFF_COAST_MAX_SLIP_BIAS;
            priv->dashboardInstant[i].setup = &(setup->differentialCoastMaxSlipBias[1]);
            i++;
        }
    }

    if (setup->differentialType[2] == DIFF_ELECTRONIC_LSD)
    {
        if (setup->differentialMaxSlipBias[2].min != setup->differentialMaxSlipBias[2].max)
        {
            priv->dashboardInstant[i].type = DI_CENTRAL_DIFF_MAX_SLIP_BIAS;
            priv->dashboardInstant[i].setup = &(setup->differentialMaxSlipBias[2]);
            i++;
        }

        if (setup->differentialCoastMaxSlipBias[2].min != setup->differentialCoastMaxSlipBias[2].max) {
            priv->dashboardInstant[i].type = DI_CENTRAL_DIFF_COAST_MAX_SLIP_BIAS;
            priv->dashboardInstant[i].setup = &(setup->differentialCoastMaxSlipBias[2]);
            i++;
        }
    }
    priv->dashboardInstantNb = i;

    /* initialize dashboardRequest */
    setup->reqRepair.min = setup->reqRepair.value = setup->reqRepair.max = 0.0;
    setup->reqRepair.desired_value = 0.0;
    setup->reqRepair.stepsize = 500;
    setup->reqRepair.changed = false;

    setup->reqTireset.min = 0.0;
    setup->reqTireset.max = 1.0;
    setup->reqTireset.value = 1.0;
    setup->reqTireset.desired_value = 1.0; //1.0 means change tires, 0.0 keep old tires
    setup->reqTireset.stepsize = 1.0;
    setup->reqTireset.changed = false;

    setup->reqTirecompound.min = 1;
    setup->reqTirecompound.max = 5;
    setup->reqTirecompound.value = 3;
    setup->reqTirecompound.desired_value = 3;
    setup->reqTirecompound.stepsize = 1;
    setup->reqTirecompound.changed = false;

    setup->reqPenalty.min = 0.0;
    setup->reqPenalty.max = 1.0;
    setup->reqPenalty.value = 0.0;
    setup->reqPenalty.desired_value = 0.0; //0.0 means refuel/repair next, 1.0 means serve penalty next
    setup->reqPenalty.stepsize = 1.0;
    setup->reqPenalty.changed = false;

    const struct setup
    {
        int type;
        tCarSetupItem *setup;
        bool (*enable)(const tCar *, const tCarSetup *);
    } setups[] =
    {
        {DI_FUEL, &setup->fuel, NULL},
        {DI_REPAIR, &setup->reqRepair, NULL},
        {DI_TYRE_SET, &setup->reqTireset, enable_tyreset},
        {DI_COMPOUND_SET, &setup->reqTirecompound, enable_compound_set},
        {DI_FRONT_WING_ANGLE, &setup->wingAngle[0], enable_front_wing_angle},
        {DI_REAR_WING_ANGLE, &setup->wingAngle[1], enable_rear_wing_angle},
        {DI_PENALTY, &setup->reqPenalty, NULL}
    };

    size_t enabled = 0;

    for (size_t i = 0; i < sizeof setups / sizeof *setups; i++)
    {
        const struct setup &s = setups[i];

        if (!s.enable || s.enable(car, setup))
        {
            tDashboardItem &it = priv->dashboardRequest[enabled];

            it.type = s.type;
            it.setup = s.setup;
            enabled++;
        }
    }

    priv->dashboardRequestNb = enabled;
    priv->dashboardActiveItem = 0;
}

void
SimCarReConfig(tCar *car)
{
    int i;
    tdble	w;
    tdble	gcfrl, gcrrl, gcfr, gcrl;
    tdble	wf0, wr0;
    tdble K[4], Kfheave, Krheave;
    tCarSetupItem *setupSpring;
    tCarSetupItem *setupGcfr = &(car->carElt->setup.FRWeightRep);
    tCarSetupItem *setupGcfrl = &(car->carElt->setup.FRLWeightRep);
    tCarSetupItem *setupGcrrl = &(car->carElt->setup.RRLWeightRep);
    tCarSetupItem *setupFuel = &(car->carElt->setup.fuel);
    tCarSetupItem *setupCompound = &(car->carElt->setup.tireCompound);

    if (setupFuel->changed)
    {
        car->fuel = MIN(setupFuel->max, MAX(setupFuel->min, setupFuel->desired_value));
        if (car->fuel > car->tank) {car->fuel = car->tank;}
        setupFuel->value = car->fuel;
        setupFuel->changed = false;
    }

    if(setupCompound->changed)
    {
        setupCompound->value = MIN(setupCompound->max, MAX(setupCompound->min, setupCompound->desired_value));
        setupCompound->changed = true;
    }

    if (setupGcfr->changed)
    {
        gcfr = MIN(setupGcfr->max, MAX(setupGcfr->min, setupGcfr->desired_value));
        setupGcfr->value = gcfr;
        setupGcfr->changed = false;
    }
    else
    {
        gcfr = setupGcfr->value;
    }

    if (setupGcfrl->changed)
    {
        gcfrl = MIN(setupGcfrl->max, MAX(setupGcfrl->min, setupGcfrl->desired_value));
        setupGcfrl->value = gcfrl;
        setupGcfrl->changed = false;
    }
    else
    {
        gcfrl = setupGcfrl->value;
    }

    if (setupGcrrl->changed)
    {
        gcrrl = MIN(setupGcrrl->max, MAX(setupGcrrl->min, setupGcrrl->desired_value));
        setupGcrrl->value = gcrrl;
        setupGcrrl->changed = false;
    }
    else
    {
        gcrrl = setupGcrrl->value;
    }

    gcrl = gcfr * gcfrl + (1 - gcfr) * gcrrl;

    for (i = 0; i < 4; i++)
    {
        setupSpring = &(car->carElt->setup.suspSpring[i]);
        K[i] = MIN(setupSpring->max, MAX(setupSpring->min, setupSpring->desired_value));
    }

    setupSpring = &(car->carElt->setup.heaveSpring[0]);
    Kfheave = MIN(setupSpring->max, MAX(setupSpring->min, setupSpring->desired_value));
    setupSpring = &(car->carElt->setup.heaveSpring[1]);
    Krheave = MIN(setupSpring->max, MAX(setupSpring->min, setupSpring->desired_value));

    /* calculate wheel loads */
    w = car->mass * G;
    wf0 = w * gcfr;
    wr0 = w * (1 - gcfr);

    car->wheel[FRNT_RGT].weight0 = wf0 * gcrl * K[FRNT_RGT] / (K[FRNT_RGT] + 0.5f*Kfheave);
    car->wheel[FRNT_LFT].weight0 = wf0 * (1 - gcrl) * K[FRNT_LFT] / (K[FRNT_LFT] + 0.5f*Kfheave);
    car->wheel[REAR_RGT].weight0 = wr0 * gcrl * K[REAR_RGT] / (K[REAR_RGT] + 0.5f*Krheave);
    car->wheel[REAR_LFT].weight0 = wr0 * (1 - gcrl) * K[REAR_LFT] / (K[REAR_LFT] + 0.5f*Krheave);

    /* reconfigure components */
    if (Kfheave > 0.0f)
    {
        wf0 = (wf0 - car->wheel[FRNT_RGT].weight0 - car->wheel[FRNT_LFT].weight0);
        SimAxleReConfig(car, FRNT, wf0);
    }
    else
    {
        SimAxleReConfig(car, FRNT, 0.0f);
    }

    if (Krheave > 0.0f)
    {
        wr0 = (wr0 - car->wheel[REAR_RGT].weight0 - car->wheel[REAR_LFT].weight0);
        SimAxleReConfig(car, REAR, wr0);
    }
    else
    {
        SimAxleReConfig(car, REAR, 0.0f);
    }

    for (i = 0; i < 4; i++)
    {
        SimWheelReConfig(car, i);
    }

    SimEngineReConfig(car);
    SimTransmissionReConfig(car);
    SimSteerReConfig(car);
    SimBrakeSystemReConfig(car);

    for (i = 0; i < 2; i++)
    {
        SimWingReConfig(car, i);
    }
}

static void
SimCarUpdateForces(tCar *car)
{
    tForces	F;
    int		i;
    tdble	m, w, minv;
    tdble	SinTheta;
    tdble	Cosz, Sinz;
    tdble	v, R, Rv, Rm, Rx, Ry;
    tdble	desiredF, desiredTq;

    Cosz = car->Cosz = cos(car->DynGCg.pos.az);
    Sinz = car->Sinz = sin(car->DynGCg.pos.az);

    car->preDynGC = car->DynGCg;

    /* total mass */
    m = car->mass + (car->fuel * car->fuelMass);
    minv = (tdble) (1.0 / m);
    w = -m * G;

    /* Weight */
    SinTheta = (tdble) ((-car->wheel[FRNT_RGT].zRoad - car->wheel[FRNT_LFT].zRoad
        + car->wheel[REAR_RGT].zRoad + car->wheel[REAR_LFT].zRoad) / (2.0 * car->wheelbase));
    F.F.x = -w * SinTheta;
    SinTheta = (tdble) ((-car->wheel[FRNT_RGT].zRoad - car->wheel[REAR_RGT].zRoad
        + car->wheel[FRNT_LFT].zRoad + car->wheel[REAR_LFT].zRoad) / (2.0 * car->wheeltrack));
    F.F.y = -w * SinTheta;
    F.F.z = (tdble) (w - (F.F.x*F.F.x + F.F.y*F.F.y)/(2.0*w));/*Taylor-polinom of sqrt(w^2-F.F.x^2-F.F.y^2)*/
    F.M.x = F.M.y = F.M.z = 0;

    /* Wheels */
    for (i = 0; i < 4; i++) {
        /* forces */
        F.F.x += car->wheel[i].forces.x;
        F.F.y += car->wheel[i].forces.y;
        F.F.z += car->wheel[i].forces.z;
        /* moments */
        F.M.x += car->wheel[i].forces.z * car->wheel[i].staticPos.y +
            car->wheel[i].forces.y * (car->statGC.z + car->wheel[i].rideHeight);
        F.M.x += car->wheel[i].torques.x;
        F.M.y -= car->wheel[i].forces.z * car->wheel[i].staticPos.x +
            car->wheel[i].forces.x * (car->statGC.z + car->wheel[i].rideHeight);
        F.M.y += car->wheel[i].torques.y;
        F.M.z += -car->wheel[i].forces.x * car->wheel[i].staticPos.y +
            car->wheel[i].forces.y * car->wheel[i].staticPos.x;
        F.M.z += car->wheel[i].torques.z;
    }

    /* Aero Drag */
    F.F.x += car->aero.drag;

    /* Wings & Aero Downforce */
    for (i = 0; i < 2; i++) {
    /* forces */
    F.F.z += car->wing[i].forces.z + car->aero.lift[i];
    F.F.x += car->wing[i].forces.x;
    /* moments */
    F.M.y -= (car->wing[i].forces.z + car->aero.lift[i]) * car->wing[i].staticPos.x +
        car->wing[i].forces.x * car->wing[i].staticPos.z;
    }

    /* Rolling Resistance */
    v = car->DynGC.vel.xy;
    R = 0;
    for (i = 0; i < 4; i++) {
        R += car->wheel[i].rollRes;
    }
    if (v > 0.00001) {
        if (v > 10.0) {
            Rv = R / v;
        } else {
            Rv = (tdble) (R / 10.0);
        }
        if ((Rv * minv * SimDeltaTime) > v) {
            Rv = v * m / SimDeltaTime;
        }
    } else {
        Rv = 0;
    }
    Rx = Rv * car->DynGC.vel.x;
    Ry = Rv * car->DynGC.vel.y;

    F.F.x -= Rx;
    F.F.y -= Ry;

    if ((R * car->wheelbase / 2.0 * car->Iinv.z) > fabs(car->DynGCg.vel.az)) {
        Rm = car->DynGCg.vel.az / car->Iinv.z;
    } else {
        Rm = (tdble) (SIGN(car->DynGCg.vel.az) * R * car->wheelbase / 2.0);
    }
    F.M.z -= Rm;

    /* simulate sticking when car almost stationary */
    if ((car->features & FEAT_SLOWGRIP) && ( v < 0.1 ) )
    {
        w = -w; //make it positive
        /* desired force to stop sideway slide */
        desiredF = - m * car->DynGC.vel.y / SimDeltaTime;
        if ( (fabs(desiredF - F.F.y)) < w )
        {
            F.F.y = desiredF;
        }
        else if ( (desiredF - F.F.y) > 0.0 )
        {
            F.F.y += w;
        }
        else
        {
            F.F.y -= w;
        }

        /* desired torque to stop yaw */
        desiredTq = - car->DynGC.vel.az / ( SimDeltaTime * car->Iinv.z );
        if ( (fabs(desiredTq - F.M.z)) < 0.5 * w * car->wheelbase) {F.M.z = desiredTq;}
        else if ( (desiredTq - F.M.z) > 0.0 ) {F.M.z += 0.5f * w * car->wheelbase;}
        else {F.M.z -= 0.5f * w * car->wheelbase;}
        /* desired force to really stop the car when braking to 0 */
        if ( ((car->ctrl->brakeCmd > 0.05) || (car->ctrl->ebrakeCmd > 0.1) || (car->ctrl->brakeFrontLeftCmd > 0.02)
          || (car->ctrl->brakeFrontRightCmd > 0.02) || (car->ctrl->brakeRearLeftCmd > 0.02) || (car->ctrl->brakeRearRightCmd > 0.02) )
          && (car->ctrl->accelCmd * car->transmission.clutch.transferValue < 0.05) && (fabs(car->DynGC.vel.x) < 0.02) ) {
            desiredF = - m * car->DynGC.vel.x / SimDeltaTime;
            w *= 0.5;
            if ( (fabs(desiredF - F.F.x)) < w ) {F.F.x = desiredF;}
            else if ( (desiredF - F.F.x) > 0.0 ) {F.F.x += w;}
            else {F.F.x -= w;}
        }
    }

    /* compute accelerations */
    car->DynGC.acc.x = F.F.x * minv;
    car->DynGC.acc.y = F.F.y * minv;
    car->DynGC.acc.z = F.F.z * minv;

    car->DynGCg.acc.x = (F.F.x * Cosz - F.F.y * Sinz) * minv;
    car->DynGCg.acc.y = (F.F.x * Sinz + F.F.y * Cosz) * minv;
    car->DynGCg.acc.z = car->DynGC.acc.z;

    car->DynGCg.acc.ax = car->DynGC.acc.ax = F.M.x * car->Iinv.x;
    car->DynGCg.acc.ay = car->DynGC.acc.ay = F.M.y * car->Iinv.y;
    car->DynGCg.acc.az = car->DynGC.acc.az = F.M.z * car->Iinv.z;
}

static void
SimCarUpdateSpeed(tCar *car)
{
    // fuel consumption
    tdble delta_fuel = car->fuel_prev - car->fuel;
    car->fuel_prev = car->fuel;
    if (delta_fuel > 0) {
        car->carElt->_fuelTotal += delta_fuel;
    }
    tdble fi;
    tdble as = sqrt(car->airSpeed2);
    if (as<0.1) {
        fi = 99.9f;
    } else {
        fi = 100000 * delta_fuel / (as*SimDeltaTime);
    }
    tdble alpha = 0.1f;
    car->carElt->_fuelInstant = (tdble)((1.0-alpha)*car->carElt->_fuelInstant + alpha*fi);

    tdble	Cosz, Sinz;
    //tdble	mass;

    //mass = car->mass + car->fuel;

    Cosz = car->Cosz;
    Sinz = car->Sinz;

    car->DynGCg.vel.x += car->DynGCg.acc.x * SimDeltaTime;
    car->DynGCg.vel.y += car->DynGCg.acc.y * SimDeltaTime;
    car->DynGCg.vel.z += car->DynGCg.acc.z * SimDeltaTime;

    car->DynGCg.vel.ax += car->DynGCg.acc.ax * SimDeltaTime;
    car->DynGCg.vel.ay += car->DynGCg.acc.ay * SimDeltaTime;
    car->DynGCg.vel.az += car->DynGCg.acc.az * SimDeltaTime;

    /* spin limitation */
    if (fabs(car->DynGCg.vel.az) > 9.0)
    {
        car->DynGCg.vel.az = (tdble) (SIGN(car->DynGCg.vel.az) * 9.0);
    }

    car->DynGC.vel.ax = car->DynGCg.vel.ax;
    car->DynGC.vel.ay = car->DynGCg.vel.ay;
    car->DynGC.vel.az = car->DynGCg.vel.az;

    car->DynGC.vel.x = car->DynGCg.vel.x * Cosz + car->DynGCg.vel.y * Sinz;
    car->DynGC.vel.y = -car->DynGCg.vel.x * Sinz + car->DynGCg.vel.y * Cosz;
    car->DynGC.vel.z = car->DynGCg.vel.z;

    /* 2D speed */
    car->DynGC.vel.xy = sqrt(car->DynGCg.vel.x * car->DynGCg.vel.x  +
         car->DynGCg.vel.y * car->DynGCg.vel.y);
}

void
SimCarUpdateWheelPos(tCar *car)
{
    int i;
    tdble vx;
    tdble vy;
    tdble Cosz, Sinz;

    Cosz = car->Cosz;
    Sinz = car->Sinz;
    vx = car->DynGC.vel.x;
    vy = car->DynGC.vel.y;

    /* Wheels data */
    for (i = 0; i < 4; i++) {
        tdble x = car->wheel[i].staticPos.x;
        tdble y = car->wheel[i].staticPos.y;
        tdble dx = x * Cosz - y * Sinz;
        tdble dy = x * Sinz + y * Cosz;

        car->wheel[i].pos.x = car->DynGCg.pos.x + dx;
        car->wheel[i].pos.y = car->DynGCg.pos.y + dy;
        car->wheel[i].pos.z = car->DynGCg.pos.z - car->statGC.z - x * sin(car->DynGCg.pos.ay) + y * sin(car->DynGCg.pos.ax);

        car->wheel[i].bodyVel.x = vx - car->DynGC.vel.az * y;
        car->wheel[i].bodyVel.y = vy + car->DynGC.vel.az * x;
    }
}

static void
SimCarUpdatePos(tCar *car)
{
    tdble vx, vy;
    //tdble accx, accy;

    vx = car->DynGCg.vel.x;
    vy = car->DynGCg.vel.y;

    //accx = car->DynGCg.acc.x;
    //accy = car->DynGCg.acc.y;

    car->DynGCg.pos.x += vx * SimDeltaTime;
    car->DynGCg.pos.y += vy * SimDeltaTime;
    car->DynGCg.pos.z += car->DynGCg.vel.z * SimDeltaTime;

    car->DynGCg.pos.ax += car->DynGCg.vel.ax * SimDeltaTime;
    car->DynGCg.pos.ay += car->DynGCg.vel.ay * SimDeltaTime;
    car->DynGCg.pos.az += car->DynGCg.vel.az * SimDeltaTime;

    FLOAT_NORM_PI_PI(car->DynGCg.pos.az);

    if (car->DynGCg.pos.ax > aMax) {
        car->DynGCg.pos.ax = aMax;
        car->DynGCg.vel.ax = 0;
    }
    if (car->DynGCg.pos.ax < -aMax) {
        car->DynGCg.pos.ax = -aMax;
        car->DynGCg.vel.ax = 0;
    }
    if (car->DynGCg.pos.ay > aMax) {
        car->DynGCg.pos.ay = aMax;
        car->DynGCg.vel.ay = 0;
    }
    if (car->DynGCg.pos.ay < -aMax) {
        car->DynGCg.pos.ay = -aMax;
        car->DynGCg.vel.ay = 0;
    }

    car->DynGC.pos.x = car->DynGCg.pos.x;
    car->DynGC.pos.y = car->DynGCg.pos.y;
    car->DynGC.pos.z = car->DynGCg.pos.z;

    car->DynGC.pos.ax = car->DynGCg.pos.ax;
    car->DynGC.pos.ay = car->DynGCg.pos.ay;
    car->DynGC.pos.az = car->DynGCg.pos.az;

    RtTrackGlobal2Local(car->trkPos.seg, car->DynGCg.pos.x, car->DynGCg.pos.y, &(car->trkPos), TR_LPOS_MAIN);
}

static void
SimCarUpdateCornerPos(tCar *car)
{
    tdble Cosz = car->Cosz;
    tdble Sinz = car->Sinz;
    tdble vx = car->DynGCg.vel.x;
    tdble vy = car->DynGCg.vel.y;
    int i;

    for (i = 0; i < 4; i++) {
        tdble x = car->corner[i].pos.x + car->statGC.x;
        tdble y = car->corner[i].pos.y + car->statGC.y;
        tdble dx = x * Cosz - y * Sinz;
        tdble dy = x * Sinz + y * Cosz;

        car->corner[i].pos.ax = car->DynGCg.pos.x + dx;
        car->corner[i].pos.ay = car->DynGCg.pos.y + dy;
        /*car->corner[i].pos.az = car->DynGC.pos.z - car->statGC.z + x * sin(car->DynGC.pos.ay) + y * sin(car->DynGC.pos.ax);*/

        /* add the body rotation to the wheel        */
        /* the speed is vel.az * r                   */
        /* where r = sqrt(x*x + y*y)                 */
        /* the tangent vector is -y / r and x / r    */
        // compute corner velocity at local frame
        car->corner[i].vel.ax = - car->DynGC.vel.az * y;
        car->corner[i].vel.ay = car->DynGC.vel.az * x;

        // rotate to global and add global center of mass velocity
        // note: global to local.
        car->corner[i].vel.x = vx
            + car->corner[i].vel.ax * Cosz - car->corner[i].vel.ay * Sinz;
        car->corner[i].vel.y = vy
            + car->corner[i].vel.ax * Sinz + car->corner[i].vel.ay * Cosz;

        // add local center of mass velocity
        car->corner[i].vel.ax += car->DynGC.vel.x;
        car->corner[i].vel.ay += car->DynGC.vel.y;
    }
}

void
SimTelemetryOut(tCar *car)
{
    int i;
    tdble Fzf, Fzr;

    if (car->ctrl->telemetryMode == 1) // Full data output
    {
        printf("-----------------------------\nCar: %d %s ---\n", car->carElt->index, car->carElt->_name);
        printf("Seg: %d (%s)  Ts:%f  Tr:%f\n",
            car->trkPos.seg->id, car->trkPos.seg->name, car->trkPos.toStart, car->trkPos.toRight);
        printf("---\nMx: %f  My: %f  Mz: %f (N/m)\n", car->DynGC.acc.ax, car->DynGC.acc.ay, car->DynGC.acc.az);
        printf("Wx: %f  Wy: %f  Wz: %f (rad/s)\n", car->DynGC.vel.ax, car->DynGC.vel.ay, car->DynGC.vel.az);
        printf("Ax: %f  Ay: %f  Az: %f (rad)\n", car->DynGCg.pos.ax, car->DynGCg.pos.ay, car->DynGCg.pos.az);
        printf("---\nAx: %f  Ay: %f  Az: %f (Gs)\n", car->DynGC.acc.x/9.81, car->DynGC.acc.y/9.81, car->DynGC.acc.z/9.81);
        printf("Vx: %f  Vy: %f  Vz: %f (m/s)\n", car->DynGC.vel.x, car->DynGC.vel.y, car->DynGC.vel.z);
        printf("Px: %f  Py: %f  Pz: %f (m)\n---\n", car->DynGCg.pos.x, car->DynGCg.pos.y, car->DynGCg.pos.z);
        printf("As: %f\n---\n", sqrt(car->airSpeed2));
        for (i = 0; i < 4; i++)
        {
            printf("wheel %d - RH:%f susp:%f zr:%.2f ", i, car->wheel[i].rideHeight, car->wheel[i].susp.x, car->wheel[i].zRoad);
            printf("sx:%f sa:%f w:%f ", car->wheel[i].sx, car->wheel[i].sa, car->wheel[i].spinVel);
            printf("fx:%f fy:%f fz:%f\n", car->wheel[i].forces.x, car->wheel[i].forces.y, car->wheel[i].forces.z);
        }
        Fzf = (tdble) ((car->aero.lift[0] + car->wing[0].forces.z) / 9.81);
        Fzr = (tdble) ((car->aero.lift[1] + car->wing[1].forces.z) / 9.81);
        printf("Aero Fx:%f Fz:%f Fzf=%f Fzr=%f ratio=%f\n", car->aero.drag / 9.81, Fzf + Fzr,
            Fzf, Fzr, (Fzf + Fzr) / (car->aero.drag + 0.1) * 9.81);
    }
    else if (car->ctrl->telemetryMode == 2)
    {
        // Mass from total and mass from wheels
        float MassTotal = car->mass + (car->fuel * car->fuelMass);
        float mass = car->wheel[FRNT_RGT].weight0 + car->wheel[FRNT_LFT].weight0 + car->wheel[REAR_RGT].weight0 + car->wheel[REAR_LFT].weight0;
        float massfactor = (mass + car->fuel*G) / mass;
        float MassTotal2 = massfactor/G * (car->wheel[0].weight0 + car->wheel[1].weight0 + car->wheel[2].weight0 + car->wheel[3].weight0);

        // Measurements of downforce at axles (Forces without part caused by weight)
        float MeasurementFront = car->wheel[0].forces.z + car->wheel[1].forces.z - massfactor * (car->wheel[0].weight0 + car->wheel[1].weight0);
        float MeasurementRear = car->wheel[2].forces.z + car->wheel[3].forces.z - massfactor * (car->wheel[2].weight0 + car->wheel[3].weight0);

        float RideHeightFront = (car->wheel[0].rideHeight + car->wheel[1].rideHeight)/2;
        float RideHeightRear = (car->wheel[2].rideHeight + car->wheel[3].rideHeight)/2;
        float hm = 3 * (RideHeightFront + RideHeightRear);
        hm = hm * hm;
        hm = hm * hm;
        hm = 2 * exp(-3 * hm);


        printf("Car spd:%.1f km/h %.2f m/s air spd:%.2f m/s spd2:%.2f m2/s2\n", car->DynGC.vel.x*3.6f, car->DynGC.vel.x, sqrt(car->airSpeed2), car->airSpeed2);
        printf("Car x:%.3f m z:%.3f m r:%.3f m zr:%.3f m\n", car->statGC.x, car->statGC.z, RideHeightRear, car->statGC.z + RideHeightRear);

        printf("Mass:%.0f kg fuel:%.2f kg total: %.2f kg / %.2f N\n", car->mass, car->fuel, MassTotal, MassTotal*G);
        printf("Mass:%.2f kg Mass:%.2f kg Delta:%.5f kg\n", MassTotal, MassTotal2, MassTotal - MassTotal2);

        printf("Ride height factor:%.3f\n", hm);

        printf("Wheel f x:%.3f m z:%.3f m\n", car->wheel[0].staticPos.x, car->wheel[0].staticPos.z);
        printf("Wheel r x:%.3f m z:%.3f m\n", car->wheel[2].staticPos.x, car->wheel[2].staticPos.z);

        printf("Wheel f - RH:%.3f m ", RideHeightFront);
        printf("Fx:%.3f Fz:%.3f N\n", car->wheel[0].forces.x + car->wheel[1].forces.x, MeasurementFront);
        printf("Wheel r - RH:%.3f m ", RideHeightRear);
        printf("Fx:%.3f Fz:%.3f N\n", car->wheel[2].forces.x + car->wheel[3].forces.x, MeasurementRear);

        printf("Wheel f - Tq:%.3f Nm\n", car->wheel[0].torques.y + car->wheel[1].torques.y);
        printf("Wheel r - Tq:%.3f Nm\n", car->wheel[2].torques.y + car->wheel[3].torques.y);

        printf("Wing f x:%.3f m z:%.3f m\n", car->wing[0].staticPos.x,car->wing[0].staticPos.z);
        printf("Wing r x:%.3f m z:%.3f m\n", car->wing[1].staticPos.x,car->wing[1].staticPos.z);

        tdble WFxf = (tdble) (car->wing[0].forces.x);
        tdble WFzf = (tdble) (car->wing[0].forces.z);
        printf("Wing f Fx:%.3f N Fz:%.3f N Fx:%.3f kg Fz:%.3f kg\n", WFxf, WFzf, WFxf/G, WFzf/G);

        tdble WFxr = (tdble) (car->wing[1].forces.x);
        tdble WFzr = (tdble) (car->wing[1].forces.z);
        printf("Wing r Fx:%.3f N Fz:%.3f N Fx:%.3f kg Fz:%.3f kg\n", WFxr, WFzr, WFxr/G, WFzr/G);

        Fzf = (tdble) ((car->aero.lift[0]*G + car->wing[0].forces.z));
        Fzr = (tdble) ((car->aero.lift[1]*G + car->wing[1].forces.z));
        tdble AFzf = (tdble) (car->aero.lift[0]*G);
        tdble AFzr = (tdble) (car->aero.lift[1]*G);
        printf("Aero Lift  Fzf=%.3f N Fzr=%.3f N Fz:%.3f N\n", AFzf, AFzr, AFzf + AFzr);
        printf("Aero Wing  Fzf=%.3f N Fzr=%.3f N Fz:%.3f N\n", Fzf-AFzf, Fzr-AFzr, Fzf + Fzr - AFzf - AFzr);
        printf("Aero Force Fzf=%.3f N Fzr=%.3f N Fz:%.3f N\n", Fzf, Fzr, Fzf + Fzr);

        printf("Aero Drag  Fx:%.3f N\n", car->aero.drag);

        printf("Downforce front:%.3f N\n", MeasurementFront);
        printf("Downforce rear:%.3f N\n", MeasurementRear);
        printf("Downforce total:%.3f N\n", MeasurementFront + MeasurementRear);

    }
    else if (car->ctrl->telemetryMode == 3)
    {
      for (i = 0; i < 4; i++)
      {
        /* forces */
        fprintf(stderr,"%d: fx:%.1f N  fy:%.1f N  fz:%.1f N\n", i, car->wheel[i].forces.x, car->wheel[i].forces.y, car->wheel[i].forces.z);
      }
    }
    else if (car->ctrl->telemetryMode == 4)
    {
        /* Balances */
        double ForceFront = car->wheel[0].forces.x + car->wheel[1].forces.x;
        double ForceRear  = car->wheel[2].forces.x + car->wheel[3].forces.x;
        double ForceRight = car->wheel[0].forces.x + car->wheel[2].forces.x;
        double ForceLeft  = car->wheel[1].forces.x + car->wheel[3].forces.x;
        double ForceTotal1 = MAX(0.1,fabs(ForceFront) + fabs(ForceRear));
        double ForceTotal2 = MAX(0.1,fabs(ForceLeft) + fabs(ForceRight));
        fprintf(stderr,"BxFR%+7.1f%% BxLR%+7.1f%% ", 100.0 * ForceFront / (ForceTotal1), 100.0 * ForceLeft / (ForceTotal2));

        ForceFront = car->wheel[0].forces.y + car->wheel[1].forces.y;
        ForceRear  = car->wheel[2].forces.y + car->wheel[3].forces.y;
        ForceRight = car->wheel[0].forces.y + car->wheel[2].forces.y;
        ForceLeft  = car->wheel[1].forces.y + car->wheel[3].forces.y;
        ForceTotal1 = MAX(0.1,fabs(ForceFront) + fabs(ForceRear));
        ForceTotal2 = MAX(0.1,fabs(ForceLeft) + fabs(ForceRight));
        fprintf(stderr,"ByFR%+7.1f%% ByLR%+7.1f%% ", 100.0 * ForceFront / (ForceTotal1), 100.0 * ForceLeft / (ForceTotal2));

        ForceFront = car->wheel[0].forces.z + car->wheel[1].forces.z;
        ForceRear  = car->wheel[2].forces.z + car->wheel[3].forces.z;
        ForceRight = car->wheel[0].forces.z + car->wheel[2].forces.z;
        ForceLeft  = car->wheel[1].forces.z + car->wheel[3].forces.z;
        ForceTotal1 = MAX(0.1,fabs(ForceFront) + fabs(ForceRear));
        ForceTotal2 = MAX(0.1,fabs(ForceLeft) + fabs(ForceRight));
        fprintf(stderr,"BzFR%+7.1f%% BzLR%+7.1f%%\n", 100.0 * ForceFront / (ForceTotal1), 100.0 * ForceLeft / (ForceTotal2));
    }
}

void
SimCarUpdate(tCar *car, tSituation * /* s */)
{
    SimCarUpdateForces(car);
    CHECK(car);
    SimCarUpdateSpeed(car);
    CHECK(car);
    SimCarUpdateCornerPos(car);
    CHECK(car);
    SimCarUpdatePos(car);
    CHECK(car);
    SimCarCollideZ(car);
    CHECK(car);
    SimCarCollideXYScene(car);
    CHECK(car);

    /* update car->carElt->setup.reqRepair with damage */
    tCarSetupItem *repair = &(car->carElt->setup.reqRepair);
    if ((repair->desired_value > 0.0) && (repair->max == repair->desired_value))
    {
        repair->max = repair->desired_value = (tdble) car->dammage;
    }
    else
    {
        repair->max = (tdble) car->dammage;
    }
}

void
SimCarUpdate2(tCar *car, tSituation * /* s */)
{
    if ((SimTelemetry == car->carElt->index)
        || (car->ctrl->telemetryMode > 0))
        SimTelemetryOut(car);
}
