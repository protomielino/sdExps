/***************************************************************************

    file                 : strategy.cpp
    created              : Wed Sep 22 15:32:21 CET 2004
    copyright            : (C) 2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id$

    ***************************************************************************/

/***************************************************************************
     *                                                                         *
     *   This program is free software; you can redistribute it and/or modify  *
     *   it under the terms of the GNU General Public License as published by  *
     *   the Free Software Foundation; either version 2 of the License, or     *
     *   (at your option) any later version.                                   *
     *                                                                         *
     ***************************************************************************/

/*
         Very simple stategy sample implementation.
         */

//#define STRATEGY_DEBUG

#include <iostream>

#include "strategy.h"
#include "globaldefs.h"

SimpleStrategy::SimpleStrategy()
{
    MAX_FUEL_PER_METER = 0.00068f;        // [liter/m] max fuel consumtion.
    MAX_FUEL_TANK = 100.0f;
    PIT_DAMMAGE = 5000;            // [-] max damage before test_Pitstop
    MAX_DAMAGE = 8000;                  // [-] max damage limit in the last five laps before the pit stop.
    fuel_Strat = 1;                // [-] Pit refuel strategy for Short(1) or Long(2) track.

    needRepair = false;            // [-] Need to repair (amount of damage)
    quickPitstop = false;            // [-] Fast repair (30% of damage)
    fuelChecked = false;            // [-] Fuel statistics updated.
    shortTrack = false;            // [-] Strategy for short track enable/disable.
    qualifRace = false;            // [-] Qualifying race type enable.
    practiceRace = false;          // [-] Practice race type enable.
    m_checkDamage = false;            // [-] Dammage to repair checked.
    m_checkFuel = false;            // [-] Needed fuel checked.
    strategy_verbose = 0;            // [-] Display information about refuel and repair.
    m_lapBuffer = 1;

    fuelPerLap = 0.0f;            // [Kg] The maximum amount of fuel we needed for a lap.
    lastPitFuel = 0.0f;            // [Kg] Amount refueled, special case when we refuel.
    fuelSum = 0.0f;                // [Kg] All the fuel used.
    counterFuelLaps = 0;            // [-] Counter of the total laps.
    countPitStop = 0;            // [-] Counter of the total pitStop.
    avgFuelPerLap = 0.0;            // [Kg] Average fuel per lap.
    m_maxDamage = 0;            // [-] max damage before we request a pit stop.
    m_Fuel = 0;                // [Kg] Security fuel at the strat race.
    m_expectedfuelperlap = 0;        // [Kg] Expected fuel per lap
    maxFuel = 200.0;
    m_tROL = 0;
    m_mWL = 0.95f;
    m_mGL = 0.7f;
    m_tHP = m_nNT = false;
    m_tmCar = NULL;
    stopPenalty = 0;
}

SimpleStrategy::~SimpleStrategy()
{
    // Nothing so far.
}

void SimpleStrategy::setFuelAtRaceStart(tTrack* t, void *carHandle, void **carParmHandle, tSituation *s, int index)
{
    /* Trivial strategy: fill in as much fuel as required for the whole race, or if the tank is
       too small fill the tank completely. */
    // Load and set parameters.
    maxFuel = GfParmGetNum(carHandle, SECT_CAR, PRM_TANK, (char*)NULL, (tdble)MAX_FUEL_TANK);
    maxFuel = GfParmGetNum(*carParmHandle, SECT_PRIVATE, PRV_MAXFUEL, (char*)NULL, (tdble)maxFuel);
    double initialFuel = GfParmGetNum(*carParmHandle, SECT_PRIVATE, PRV_INITIAL_FUEL, (char*)NULL, (tdble)0.0f);
    fuelPerMeter = GfParmGetNum(*carParmHandle, SECT_PRIVATE, BT_ATT_FUELPERMETER, (char*)NULL, MAX_FUEL_PER_METER);
    fuelPerLap = GfParmGetNum(*carParmHandle, SECT_PRIVATE, BT_ATT_FUELPERLAP, (char*)NULL, t->length * fuelPerMeter);
    fuel_Strat = (int)GfParmGetNum(*carParmHandle, SECT_PRIVATE, PRV_PIT_STRATEGY, (char*)NULL, 0.0);
    test_Pitstop = (GfParmGetNum(*carParmHandle, SECT_PRIVATE, PRV_PIT_TEST, (char*)NULL, 0.0) > 0.01f);
    test_qualifTime = (GfParmGetNum(*carParmHandle, SECT_PRIVATE, PRV_QUALIF_TEST, (char*)NULL, 0.0) > 0.01f);
    strategy_verbose = (int)GfParmGetNum(*carParmHandle, SECT_PRIVATE, PRV_STRATEGY_VERBOSE, (char*)NULL, 0.0);
    m_pittime = GfParmGetNum(*carParmHandle, SECT_PRIVATE, PRV_PITSTOP_TIME, (char*)NULL, 30.0);
    m_lapBuffer = (int)GfParmGetNum(*carParmHandle, SECT_PRIVATE, PRV_PIT_LAP_BUFFER, (char*)NULL, 1.0);

    if (fuel_Strat < 1)
    {
        fuel_Strat = 1;
    }

    if (fuel_Strat == 1)
    {
        shortTrack = true;
    }

    m_expectedfuelperlap = fuelPerLap;
    double raceDist = 0.0;
    float fuelForRace = 0.0;
    int numPitstop = 0;
    int raceLaps = s->_totLaps;
    //raceDist = raceLaps * t->length;
    //fuelForRace = raceDist * fuelPerMeter;
    fuelForRace = s->_totLaps * fuelPerLap;
    numPitstop = (int)fabs(fuelForRace / maxFuel);

    if (numPitstop < fuelForRace / maxFuel)
    {
        numPitstop = numPitstop + 1;
    }

    double m_fuel; // + security fuel.

    if (shortTrack)
    {
        m_fuel = fuelPerLap * 4.0;
    }
    else
    {
        m_fuel = fuelPerLap * 2.3;
    }

    // Initial fuel at Start race
    if (fuelForRace > maxFuel * 3)
    {
        // welcome in Endurance race :-)
        m_FuelStart = (float)(m_fuel + calcFuel(fuelForRace));

    }
    else if (fuelForRace < maxFuel)
    {
        //maybe we are in Practice mode
        m_FuelStart = (float)(m_fuel + raceLaps * fuelPerLap);
    }
    else
    {
        m_FuelStart = (float)(m_fuel + fuelForRace);
    }

    if (s->_raceType == RM_TYPE_QUALIF || s->_raceType == RM_TYPE_PRACTICE)
    {
        if (s->_raceType == RM_TYPE_QUALIF)
            qualifRace = true;
        m_fuel = 0;

        if (index == 1)
        {
            m_FuelStart = (float)(s->_totLaps * fuelPerLap) + 0.0f;
        }
        else
        {
            m_FuelStart = s->_totLaps * fuelPerLap;
        }
    }

    /* Tests in Practice mode */
    //maybe we need to check PitStop (eg. corkscrew, e-track-3)
    if (test_Pitstop && s->_raceType == RM_TYPE_PRACTICE)
    {
        m_fuel = 0;
        m_FuelStart = s->_totLaps * fuelPerLap;
        //maybe we need to check Best Lap Time for qualifying
    }
    else if (test_qualifTime && s->_raceType == RM_TYPE_PRACTICE)
    {
        qualifRace = true;
        m_fuel = 0;
        m_FuelStart = s->_totLaps * fuelPerLap;
    }
    else if (s->_raceType == RM_TYPE_PRACTICE)
    {
        practiceRace = true;
    }

    m_FuelStart = (float)MIN(m_FuelStart, maxFuel);

    LogUSR.debug("# USR_2019 Index %d : Laps = %d, Fuel per Lap = %.2f, securityFuel = + %.2f, Fuel at Start Race = %.2f\n",
                 index, s->_totLaps, fuelPerLap, m_fuel, m_FuelStart);

    if (initialFuel > 1.0)
        m_FuelStart = (float)initialFuel;

    GfParmSetNum(*carParmHandle, SECT_CAR, PRM_FUEL, (char*)NULL, m_FuelStart);
}

void SimpleStrategy::update(tCarElt* car, tSituation *s)
{
    // Fuel statistics update.
    int id = car->_trkPos.seg->id;
    /* Range must include enough segments to be executed once guaranteed. */
    if (id >= 0 && id < 5 && !fuelChecked)
    {
        if (car->race.laps > 1)
        {
            //fuelPerLap = MAX(fuelPerLap, (lastFuel + lastPitFuel - car->priv.fuel));
            fuelSum += (lastFuel + lastPitFuel - car->priv.fuel);
            fuelPerLap = (fuelSum / (car->race.laps - 1));
            counterFuelLaps++;
            avgFuelPerLap = fuelSum / counterFuelLaps;
            //if (strategy_verbose) {
            // Just display pit refuel messages
            updateFuelStrategy(car, s);
            //}
        }

        lastFuel = car->priv.fuel;
        lastPitFuel = 0.0;
        fuelChecked = true;
    }
    else if (id > 5)
    {
        fuelChecked = false;
    }
}

int SimpleStrategy::calcRepair(tCarElt* car, tSituation *s)
{
    // find out what our lead over next car is.
    float lead = FLT_MAX;
    int pos = 1000, sortedOppCount = 0, i, j;
    bool lastPit = ((car->_remainingLaps + 1) * fuelPerLap > maxFuel ? false : true);
    Opponent *sortedOpp[64];
    Opponent *O = NULL;
    tCarElt *m_tmCar = NULL;

    if (!lastPit)
    {
        return car->_dammage;
    }

#ifdef STRATEGY_DEBUG
    LogUSR.debug("%s calculating damage to repair...\n", car->_name);
#endif
    // sort opponents behind me in order of position
    for (i = 0; i < opp->getNOpponents(); i++)
    {
        Opponent *o = opp->getOpponentPtr() + i;
        tCarElt *ocar = o->getCarPtr();
        if (ocar->_state > RM_CAR_STATE_PIT && ocar->_state != RM_CAR_STATE_OUTOFGAS) continue;
        if (ocar->_pos < car->_pos - 1) continue;
        if (o->getTeam() == TEAM_FRIEND) continue;

        for (j = 0; j < sortedOppCount; j++)
        {
            tCarElt *sortedOcar = sortedOpp[j]->getCarPtr();

            if (ocar->_pos > sortedOcar->_pos)
            {
                for (int k = sortedOppCount; k > j; k--)
                    sortedOpp[k] = sortedOpp[k - 1];

                sortedOppCount++;
                sortedOpp[j] = o;
                break;
            }
        }

        if (j == sortedOppCount)
        {
            sortedOpp[j] = o;
            sortedOppCount++;
        }
    }

    if (!sortedOppCount)
        return car->_dammage;

    // which car is behind me, not including team members?
    for (i = 0; i < sortedOppCount; i++)
    {
        tCarElt *ocar = sortedOpp[i]->getCarPtr();

        int safe_damage = 0;
        float mytime = float((car->_distFromStartLine / track->length) * (car->_bestLapTime*1.2) + (car->_laps - ocar->_laps) * (car->_bestLapTime*1.1));
        float othertime = float((ocar->_distFromStartLine / track->length) * ocar->_bestLapTime);

        // how far behind is it?
        if (ocar->_pos < car->_pos)
        {
            if (ocar->_fuel > fuelPerLap * 2)
                continue;
            lead = mytime - (othertime - m_pittime);
        }
        else
            lead = mytime - othertime;

        if (ocar->_fuel < fuelPerLap)
        {
            // opponent is going to have to pit
            if (ocar->_state < RM_CAR_STATE_PIT)
                lead += m_pittime / 2;
        }

        // how much damage is it safe to fix?
        safe_damage = (int)(lead / 0.007);

        if (car->_dammage - safe_damage >= m_maxDamage)
        {
            if (car->_remainingLaps < 20)
            {
                if (car->_dammage >= m_maxDamage)
                    return MIN(car->_dammage, car->_dammage - (m_maxDamage - 1000));
                return MIN(car->_dammage, 500);
            }

            // he'll be past before we're repaired, look at the next car
            continue;
        }

        return MIN(car->_dammage, safe_damage);
    }

    return car->_dammage;
}

void SimpleStrategy::updateFuelStrategy(tCarElt* car, tSituation *s)
{
    double requiredfuel;

    /* Required additional fuel for the rest of the race. +1 because
       the computation happens right after crossing the start line. */

    //requiredfuel = ((car->_remainingLaps + 1) - ceil(car->_fuel/fuelPerLap))*fuelPerLap;
    //requiredfuel = ((car->_remainingLaps) *fuelPerLap) - ceil(car->_fuel);
    requiredfuel = ((car->_remainingLaps) *fuelPerLap) - car->_fuel;

    if (!m_checkFuel && requiredfuel <= 0.0f && car->_remainingLaps <= 5 && car->_remainingLaps > 0)
    {
        // We have enough fuel to end the race, no further stop required.
        LogUSR.debug("%s No Pitstop required > carFuel:%.2f, remLap:%d\n", car->_name, car->_fuel, car->_remainingLaps);
        m_checkFuel = false;

        return;
    }

    // We don't have enough fuel to end the race need at least one stop.
    if (!m_checkFuel && requiredfuel > 0.0f && car->_fuel < fuelPerLap * 3.0 && car->_remainingLaps > 0)
    {
        LogUSR.debug("%s Pitstop needed to refuel >> reqFuel: %.2f, carFuel: %.2f, remLap: %d\n", car->_name, requiredfuel, car->_fuel, car->_remainingLaps);
        m_checkFuel = true;
    }

    return;
}

bool SimpleStrategy::needPitstop(tCarElt* car, tSituation *s)
{
    if (qualifRace)
        return false;

    bool pitNeeded = false;

    m_maxDamage = PIT_DAMMAGE;
    float attvalue = 0.0f;
    // load defined value in xml file of Max Dammage before pitstops for this track
    attvalue = GfParmGetNum(car->_carHandle, SECT_PRIVATE, BT_ATT_MAXDAMAGE, (char*)NULL, (tdble)PIT_DAMMAGE);
    m_maxDamage = (int)attvalue;
    // Estimated average fuel per lap
    m_Fuel = GfParmGetNum(car->_carHandle, SECT_PRIVATE, BT_ATT_FUELPERLAP, (char*)NULL, m_expectedfuelperlap);

    double minFuelFactor = 1.02, teamFuelFactor = ((double)m_lapBuffer) + 0.02;

    if (qualifRace || practiceRace)
    {
        minFuelFactor = 0.99;
    }

    /* Question makes only sense if there is a pit. */
    if (car->_pit != NULL)
    {
        /* Ideally we shouldn't pit on the last lap...
            just get to the finish line somehow. */
        int lapsToEnd = car->_remainingLaps - car->_lapsBehindLeader;
        if (lapsToEnd > 1 || (lapsToEnd > 0 && car->_fuel < fuelPerLap * 0.8))
        {
            // Is there a penalty to serve?
            if (lapsToEnd > 4)
            {
                tCarPenalty *penalty = GF_TAILQ_FIRST(&(car->_penaltyList));
                if (penalty)
                {
                    if (penalty->penalty == RM_PENALTY_DRIVETHROUGH ||
                            penalty->penalty == RM_PENALTY_STOPANDGO)
                    {
                        stopPenalty = penalty->penalty;
                        pitNeeded = true;

                        LogUSR.debug("%s serving %s penalty\n", car->_name, (stopPenalty == RM_PENALTY_STOPANDGO ? "Stop & Go" : "Drivethrough"));

                        return pitNeeded;
                    }
                }
            }

            stopPenalty = 0;

            // Do we need to refuel?
            double cmpfuel = (fuelPerLap == 0.0f) ? m_expectedfuelperlap : fuelPerLap;
            double reqfuel = lapsToEnd * cmpfuel;
            double mff = minFuelFactor;
            if (m_tmCar && !(m_tmCar->_state & RM_CAR_STATE_NO_SIMU))
            {
                if (fabs(m_tmCar->_fuel - car->_fuel) < cmpfuel * m_lapBuffer)
                {
                    if (m_tmCar->_fuel < car->_fuel)
                    {
                        // give team-mate priority
                        mff /= 2;
                        LogUSR.debug("%s: Giving priority to team-mate mff=%.1f, myfuel=%.2f his=%.2f\n", car->_name, mff, car->_fuel, m_tmCar->_fuel);
                    }
                    else
                    {
                        // make sure we have priority
                        mff = teamFuelFactor;
                        LogUSR.debug("%s: Taking pit priority mff=%.1f (%.1f), myfuel=%.2f his=%.2f\n", car->_name, mff, mff*cmpfuel, car->_fuel, m_tmCar->_fuel);
                    }
                }
            }

            //if (car->_fuel < fuelPerLap * minFuelFactor && car->_fuel < fuelPerLap * lapsToEnd * minFuelFactor)
            if ((lapsToEnd >= 4 && car->_fuel < cmpfuel * mff && car->_fuel < cmpfuel * lapsToEnd * minFuelFactor) ||
                    (lapsToEnd < 4 && car->_fuel < cmpfuel * (lapsToEnd + 1) && car->_fuel < cmpfuel))
            {
                if (!m_checkFuel)
                {
                    LogUSR.debug("%s Go to Pit the next lap to refuel: reqFuel=%.2f, carFuel=%.2f, remLap=%d\n",
                                 car->_name, reqfuel, car->_fuel, car->_remainingLaps);

                    m_checkFuel = true;
                }

                pitNeeded = true;
            }
            /*
            else if (car->_fuel < fuelPerLap * minFuelFactor)
            {
                fprintf(stderr, "%s Not pitting as car fuel %.2f >= lapsToEnd (%d) * fuelPerLap %.3f * factor %.3f = %.3f\n",car->_name, car->_fuel, lapsToEnd, fuelPerLap, minFuelFactor, fuelPerLap * lapsToEnd * minFuelFactor);fflush(stderr);
            }
            */

            // Do we need to repair and is the pit free?
            m_nNT = m_tHP = false;
            needRepair = false;

            if (car->_dammage > m_maxDamage && isPitFree(car))
            {
                needRepair = true;
                pitNeeded = true;

                if (laps_to_go(car) > 5)
                {
                    if (!m_checkDamage)
                    {
                        LogUSR.debug("%s >> Max_damage: %d Car_damage: %d Laps_toGo: %d\n", car->_name, m_maxDamage, car->_dammage, laps_to_go(car));

                        m_checkDamage = true;
                        m_nNT = true;
                    }
                }
                else if (laps_to_go(car) <= 5)
                {
                    if (car->_dammage > MAX_DAMAGE)
                    {
                        quickPitstop = true;
                    }
                    else
                    {
                        if (!m_checkDamage)
                        {
                            LogUSR.debug("%s Dont Stop for Damage! Laps_toGo:%d  Car_damage: %d Max_damage: %d\n", car->_name, laps_to_go(car), car->_dammage, m_maxDamage);

                            m_checkDamage = true;
                            needRepair = false;
                        }
                    }
                }
            }
            else if (car->_dammage > m_maxDamage)
            {
                LogUSR.debug("%s >> NEED TO PIT FOR DAMAGE BUT ITS IN USE!\n", car->_name);
            }

            if (test_Pitstop)
                pitNeeded = true;
        }

        if (isPitFree(car) && lapsToEnd > 2)
        {
            if (cardata->HasTYC)
            {
                float mW = 0.0f, mG = 0.0f, aFT = 0.0;

                for (int i = 0; i < 4; i++)
                {
                    mW = MAX(car->_tyreTreadDepth(i), car->_tyreCritTreadDepth(i));
                    mG = MAX(car->_tyreCondition(i), 0.5);
                    if (i < 2)
                        aFT += 1.0;
                }

                float tWR = mW / (car->_laps - m_tROL) * 1.02f;
                int raceRemainingLaps = car->_remainingLaps - (car->_pos == 1 ? 0 : car->_lapsBehindLeader);
                int tRL = int((MIN(0.98, 1.0 - tWR * 3) - mW) / tWR);

                aFT = aFT / 2;

                if (tRL < raceRemainingLaps && raceRemainingLaps > 1 &&
                        (/*mW > m_mWL || */mW < 0.5f && cardata->aFTT < 70.0f) || m_checkFuel
                        || (m_checkDamage && mW > 0.4))
                {
                    pitNeeded = true;
                    m_nNT = true;
                    LogUSR.debug("%s >> new boots\n", car->_name);

                }
                else if (mW > 0.5f && aFT < 85.0f)
                    m_tHP = true;
            }
            else
            {
                if (m_checkFuel || (m_checkDamage))
                {
                    pitNeeded = true;
                    m_nNT = true;
                    LogUSR.debug("%s >> new boots\n", car->_name);
                }
            }
        }

        return pitNeeded;
    }
}

bool SimpleStrategy::isPitFree(tCarElt* car)
{
    if (car->_pit != NULL)
    {
        if (car->_pit->pitCarIndex == TR_PIT_STATE_FREE)
        {
            LogUSR.debug("%s Pit is free\n", car->_name);

            return true;
        }
    }

    LogUSR.debug("%s Pit is NOT free\n", car->_name);

    return false;
}

int SimpleStrategy::pitRepair(tCarElt* car, tSituation *s)
{
    m_checkDamage = false;
    int damRepair = 0;
    //if (needRepair)
    if (car->_dammage > 0)
    {
        damRepair = calcRepair(car, s);

        if (quickPitstop)
        {
            damRepair = (int)(0.3 * car->_dammage);
        }

        float refuel = pitRefuel(car, s);

        LogUSR.debug("# %s refuel=%.1f tank=%.1f\n", car->_name, refuel + car->_fuel, maxFuel);

        if (refuel < maxFuel * 0.5)
        {
            // last pitstop for the race, limit how much damage we repair
            int repair = 0;

            if (car->_dammage > m_maxDamage)
                repair = MIN(car->_dammage, (car->_dammage - m_maxDamage) + 500);

            double ratio = (double)((double)(refuel + car->_fuel) / (double)(maxFuel / 2) - 0.2);

            if (ratio > 0.0)
                repair += (int)(((double)(car->_dammage - repair)) * ratio);

            LogUSR.debug("# %s ratio=%.1f damage=%d repair=%d\n", car->_name, ratio, car->_dammage, repair);
            repair = MIN(car->_dammage, repair);
            damRepair = MIN(damRepair, repair);
        }

        needRepair = false;
    }

    LogUSR.debug("# %s repairing %d dammage\n", car->_name, damRepair);

    return damRepair;
}

float SimpleStrategy::pitRefuel(tCarElt* car, tSituation *s)
{
    float fuel;
    float fuelToEnd;
    int lapsToEnd;

    lapsToEnd = car->_remainingLaps - car->_lapsBehindLeader + 1;
    int inLap = s->_totLaps - car->_remainingLaps;
    //fuelToEnd = MIN(getRefuel1(lapsToEnd), getRefuel2(lapsToEnd));
    fuelToEnd = (float)getRefuel1(lapsToEnd);

    //m_remainingstops = int(floor(fuelToEnd / maxFuel));
    m_remainingstops = (int)fabs(fuelToEnd / MIN(maxFuel, car->_tank));
    int num_remStops = m_remainingstops + 1;
    m_fuelperstint = (float)(MIN(maxFuel, car->_tank) - car->_fuel);
    fuel = m_fuelperstint * 0.90f;
    double addFuel = fuelPerLap;
    double addMinFuel = fuelPerLap * 0.80;

    if (shortTrack && fuelPerLap < 3.50)
    {
        addFuel = fuelPerLap * 2.0;
        addMinFuel = fuelPerLap;
    }

    countPitStop++;

    fuel = 0.0f;

    if ((car->_remainingLaps + 1.0f) * fuelPerLap > car->_fuel)
        fuel = MAX(MIN(((car->_remainingLaps + 1.0f) * fuelPerLap + 1.0f) - car->_fuel, (float)maxFuel - car->_fuel), 0.0f);

    lastPitFuel = fuel;
    m_checkFuel = false;

    return fuel;
}

tCarPitCmd::TireChange SimpleStrategy::pitTyres(tCarElt *car, tSituation *s)
{
    if (cardata->HasTYC)
    {
        // called when car arrives in pits.
        if (m_nNT || m_tHP)
        {
            m_tROL = car->_laps;

            return tCarPitCmd::ALL;
        }
    }

    return tCarPitCmd::NONE;
}

float SimpleStrategy::calcFuel(double totalFuel)
{
    float fuelAtStart;
    float m_lastfuel;
    int nb_pitstop = 0;
    int nb_laps = 0;

    nb_pitstop = 1 + (int)fabs(totalFuel / maxFuel);
    m_lastfuel = (float)(totalFuel / nb_pitstop);  //Max refuel per pit stop
    nb_laps = 1 + (int)floor(m_lastfuel / fuelPerLap);
    fuelAtStart = nb_laps * fuelPerLap;

    return fuelAtStart;
}

double SimpleStrategy::getRefuel1(int laps)
{
    double refuelforrace = laps * fuelPerLap;

    return refuelforrace;
}

double SimpleStrategy::getRefuel2(int laps)
{
    double refuelforrace = laps * avgFuelPerLap;

    return refuelforrace;
}
