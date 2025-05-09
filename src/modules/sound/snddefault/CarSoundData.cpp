// -*- Mode: c++ -*-
/***************************************************************************
    file                 : CarSoundData.cpp
    created              : Tue Apr 5 19:57:35 CEST 2005
    copyright            : (C) 2005 Christos Dimitrakakis
    email                : dimitrak@idiap.ch

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "SoundInterface.h"
#include "CarSoundData.h"

CarSoundData::CarSoundData(int id, SoundInterface* sound_interface)
{
    engine_sound = nullptr;
    eng_pri.id = id;
    eng_pri.a = 1.0f;
    engine.a = 0.0f;
    engine.f = 1.0f;
    engine.lp = 1.0f;
    smooth_accel = 0.0f;
    drag_collision.a = 0.0f;
    drag_collision.f = 0.0f;
    pre_axle = 0.0f;
    axle.a = 0.0f;
    axle.f = 0.0f;
    turbo.a = 0.0f;
    turbo.f = 0.0f;
    engine_backfire.a=0.0f;
    engine_backfire.f = 0.0f;
    curb.a = 0.0f;
    curb.f = 0.0f;
    grass.a = 0.0f;
    grass.f = 0.0f;
    dirt_skid.a = 0.0f;
    dirt_skid.f = 0.0f;
    dirt.a = 0.0f;
    dirt.f = 0.0f;
	snow.a = 0.0f;
	snow.f = 0.0f;
    road.a = 0.0f;
    road.f = 0.0f;
    skid_metal.a = 0.0f;
    skid_metal.f = 0.0f;
    prev_gear = 0;
    gear_changing = false;
    bottom_crash = false;
    bang = false;
    crash = false;
    turbo_on = false;
    turbo_ilag = 0.05f;
    turbo_rpm = 0.0f;
    this->sound_interface = sound_interface;
    for (int i=0; i<4; i++) {
        for (int j=0; j<3; j++) {
            wheel[i].p[j] = 0.0f;
            wheel[i].u[j] = 0.0f;
        }
        wheel[i].skid.a = 0.0f;
        wheel[i].skid.f = 1.0f;
    }
    sgVec3 zeroes = {0.0f, 0.0f, 0.0f};
    setCarPosition(zeroes);
    setCarSpeed(zeroes);
    setListenerPosition(zeroes);
    
    attenuation = 0.0f;
    base_frequency = 0.0f;
}

void CarSoundData::setEngineSound (Sound* engine_sound, float rpm_scale)
{
    this->engine_sound = engine_sound;
    base_frequency = rpm_scale;
}

void CarSoundData::setTurboParameters (bool turbo_on, float turbo_rpm, float turbo_lag)
{
    this->turbo_on = turbo_on;
    this->turbo_rpm = turbo_rpm;
    if (turbo_lag > 0.0f) {
        this->turbo_ilag = exp(-3.0f*turbo_lag);
    } else {
        fprintf (stderr, "warning: turbo lag %f <= 0\n", turbo_lag);
    }
}

Sound* CarSoundData::getEngineSound () const
{
	return engine_sound;
}

void CarSoundData::copyEngPri (SoundPri& epri) const
{
	epri = eng_pri;
}

void CarSoundData::setCarPosition (sgVec3 p)
{
	for (int i=0; i<3; i++) {
		position[i] = p[i];
	}
}

void CarSoundData::setCarSpeed (sgVec3 u)
{
	for (int i=0; i<3; i++) {
		speed[i] = u[i];
	}
}

void CarSoundData::getCarPosition (sgVec3 p) const
{
	for (int i=0; i<3; i++) {
		p[i] = position[i];
	}
}

void CarSoundData::getCarSpeed (sgVec3 u) const
{
	for (int i=0; i<3; i++) {
		u[i] = speed[i];
	}
}

void CarSoundData::setListenerPosition (sgVec3 p)
{
	for (int i=0; i<3; i++) {
		listener_position[i] = p[i];
	}
}

void CarSoundData::update (tCarElt* car)
{
    assert (car->index == eng_pri.id);
    speed[0] = car->pub.DynGCg.vel.x;
    speed[1] = car->pub.DynGCg.vel.y;
    speed[2] = car->pub.DynGCg.vel.z;
    position[0] = car->pub.DynGCg.pos.x;
    position[1] = car->pub.DynGCg.pos.y;
    position[2] = car->pub.DynGCg.pos.z;
    calculateAttenuation (car);
    calculateEngineSound (car);
    calculateBackfireSound (car);
    calculateTyreSound (car);
    calculateCollisionSound (car);
    calculateGearChangeSound (car);
}

/// Use inverse distance to calculate attenuation of sounds originating from this car. Useful for prioritisation of sounds.
void CarSoundData::calculateAttenuation (tCarElt* car)
{
    if (car->_state & RM_CAR_STATE_NO_SIMU) {
        attenuation = 0.0f;
        return;
    }
    float d = 0.0;
    for (int i=0; i<3; i++) {
        float delta = (listener_position[i] - position[i]);
        d += delta*delta;
    }
    attenuation = 1.0f / (1.0f + sqrt(d));
    eng_pri.a = attenuation;
}

/// Calculate characteristics of the engine sound.
void CarSoundData::calculateEngineSound (tCarElt* car)
{
    float mpitch = (tdble)(base_frequency * car->_enginerpm / 600.0);
    engine.f = mpitch;
    engine.a = 1.0f;
    if (car->_state & RM_CAR_STATE_NO_SIMU) {
        engine.a = 0.0f;
        engine.lp = 1.0;
        turbo.a = 0.0f;
        turbo.f = 1.0f;
        return;
    }
    //assert(car->index == eng_pri.id);

    float gear_ratio = car->_gearRatio[car->_gear + car->_gearOffset];
    axle.a = 0.2f*(tanh(100.0f*(fabs(pre_axle - mpitch))));
    axle.f = (pre_axle + mpitch)*0.05f*fabs(gear_ratio);
    pre_axle = (pre_axle + mpitch)*0.5f;

    if (turbo_on) {
        float turbo_target = 0.1f;
        float turbo_target_vol = 0.0f;
        if (car->_enginerpm > turbo_rpm) {
            turbo_target = 0.1f + 0.9f * smooth_accel;
            turbo_target_vol = 0.1f * smooth_accel;
        }
                
        turbo.a += 0.1f * (turbo_target_vol - turbo.a) * (0.1f + smooth_accel);
        float turbo_target_pitch = turbo_target * car->_enginerpm / 600.0f;
        turbo.f += turbo_ilag * (turbo_target_pitch - turbo.f) * (smooth_accel);
        turbo.f -= (tdble)(turbo.f * 0.01 * (1.0-smooth_accel));//.99f;
    } else {
        turbo.a = 0.0;
    }
    smooth_accel = (tdble)(smooth_accel*0.5 + 0.5*(car->ctrl.accelCmd*.99+0.01));

    // engine filter proportional to revcor2.
    // when accel = 0, lp \in [0.0, 0.25]
    // when accel = 1, lp \in [0.25, 1.0]
    // interpolate linearly for values in between.
    float rev_cor = car->_enginerpm/car->_enginerpmRedLine;
    float rev_cor2 = rev_cor * rev_cor;
    engine.lp = (0.75f*rev_cor2 + 0.25f)*smooth_accel
        + (1.0f-smooth_accel)*0.25f*rev_cor2;

    // TODO: filter for exhaust and car body resonance?
                
}


/// Calculate the frequency and amplitude of a looped backfiring sound.
void CarSoundData::calculateBackfireSound (tCarElt* car)
{               
    if (car->_state & RM_CAR_STATE_NO_SIMU) {
        engine_backfire.a = 0.0f;
        engine_backfire.f = 1.0f;
        return;
    }
    if ((car->priv.smoke>0.0)&&(engine_backfire.a<0.5)) {
        engine_backfire.a += .25f*car->priv.smoke;
    }
    engine_backfire.f = (tdble)(car->_enginerpm / 600.0);
    engine_backfire.a *= (tdble)(.9*.5+.5*exp(-engine_backfire.f));
}

void CarSoundData::calculateTyreSound(tCarElt* car)
{
    curb.a = 0.0;
    curb.f = 1.0f;
    grass.a = 0.0;
    grass.f = 1.0f;
    dirt_skid.a = 0.0;
    dirt_skid.f = 1.0f;
    dirt.a = 0.0;
    dirt.f = 1.0f;
    road.a = 0.0;
    road.f = 0.0f;
	road_scrub.a = 0.0;
	road_scrub.f = 0.0f;
	snow.a = 0.0;
	snow.f = 0.0f;
	
    float car_speed2 = car->_speed_x * car->_speed_x + car->_speed_y * car->_speed_y;
    bool flag = false;
    int i;
    for (i = 0; i<4; i++) {
        wheel[i].skid.a = 0.0f;
        wheel[i].skid.f = 1.0f;
		wheel[i].scrub.a = 0.0f;
		wheel[i].scrub.f = 1.0f;
    }
    if (car->_state & RM_CAR_STATE_NO_SIMU) {
        return;
    }

    for (i = 0; i<4; i++) {
        if (car->_wheelSpinVel(i) > 0.1f) {
            flag = true;
            break;
        }
    }
    if (car->_state & RM_CAR_STATE_NO_SIMU
        || 
        (((car->_speed_x*car->_speed_x + car->_speed_y*car->_speed_y) < 0.1f)
        && (flag == false))) {
        return;
    }

    for (i = 0; i<4; i++) {
        const char* s = NULL;
        tdble roughness = 0.0f;
        tdble roughnessFreq = 1.0f;
        float ride  = 0.0001f;
        float tmpvol = sqrt(car_speed2)*0.01f;
        if (car==NULL) {
            fprintf (stderr, "Error: (grsound.c) no car\n");
            continue;
        } else if (car->priv.wheel[i].seg==NULL) {
            fprintf (stderr, "Error: (grsound.c) no seg\n");
            continue;
        } else if (car->priv.wheel[i].seg->surface==NULL) {
            fprintf (stderr, "Error: (grsound.c) no surface\n");
            continue;
        } else if (car->priv.wheel[i].seg->surface->material==NULL) {
            fprintf (stderr, "Error: (grsound.c) no material\n");
            continue;
        } else {
            s = car->priv.wheel[i].seg->surface->material;
            roughness = car->priv.wheel[i].seg->surface->kRoughness;
            roughnessFreq = (tdble)(2.0*EX_PI * car->priv.wheel[i].seg->surface->kRoughWaveLen);
            if (roughnessFreq>2.0f) {
                roughnessFreq = 2.0f + tanh(roughnessFreq-2.0f);
            }
            //ride = car->priv.wheel[i].rollRes;
            ride = 0.001f * car->_reaction[i];
        }

        bool out_of_road = false;

        if ((s)
            &&((strcmp(s, TRK_VAL_GRASS)==0)
               ||(strcmp(s, TRK_VAL_SAND)==0)
               ||(strcmp(s, TRK_VAL_DIRT)==0)
			   ||(strcmp(s, TRK_VAL_SNOW) == 0)
			   || (strstr(s, "snow"))
               ||(strstr(s, "sand"))
               ||(strstr(s, "dirt"))
               ||(strstr(s, "grass"))
               ||(strstr(s, "gravel"))
               ||(strstr(s, "mud"))
               )) {
            out_of_road = true;
        }

        bool on_curb = false;
        if ( (s) && strstr(s, "curb") )
        {
           on_curb = true;
           //printf("On curb wheel %d\n", i);
        }

        wheel[i].skid.a = 0.0f;
        wheel[i].skid.f = 1.0f;

		wheel[i].scrub.a = 0.0f;
		wheel[i].scrub.f = 1.0f;
        if (out_of_road==false) {
            float tmppitch = tmpvol*(0.75f+0.25f*roughnessFreq);
            float wind_noise = 1.0f;
            float road_noise = 0.25f;
            tmpvol = tmpvol*(wind_noise + ride*road_noise);
            if (on_curb) {
               if (curb.a < tmpvol) {
                  curb.a = tmpvol;
                  curb.f = tmppitch;
                  //printf("wheel %d curb vol %f \n", i, curb.a);
               }
            } else {
               if (road.a < tmpvol) {
                  road.a = tmpvol;
                  road.f = tmppitch;
               }
            }
			
			// This is not a smart way to implement tire scrubbing.
			if (car->_skid[i] > 0.05f) {
                //skvol[i] = (float)car->_skid[i];
                //skpitch[i] = 0.7+0.3*roughnessFreq;
                wheel[i].skid.a = (float)car->_skid[i]-0.05f;
				wheel[i].scrub.a = MAX(0.0f, MIN((0.5f - wheel[i].skid.a), wheel[i].skid.a)) * 4;
                float wsa = tanh((car->_wheelSlipAccel(i)+10.0f)*0.01f);
                wheel[i].skid.f = (0.3f - 0.3f*wsa + 0.3f*roughnessFreq)/(1.0f+0.5f*tanh(car->_reaction[i]*0.0001f));
				wheel[i].scrub.f = wheel[i].skid.f;
            } else {
                wheel[i].skid.a = 0.0f;
                wheel[i].skid.f = 1.0f;
				wheel[i].scrub.a = 0.0f;
				wheel[i].scrub.f = 1.0f;
            }
            //printf ("%d %f %f\n", i, wheel[i].skid.a, wheel[i].skid.f);
        } else {
            float tmppitch = tmpvol*(0.5f + 0.5f*roughnessFreq);

            tmpvol = (0.5f+0.2f*tanh(0.5f*roughness))*tmpvol * ride;

            if ((s)
                &&
                ((strstr(s, "sand"))
                    || (strstr(s, "dirt"))
                    || (strstr(s, "gravel")))) {
                if (dirt.a < tmpvol) {
                    dirt.a = tmpvol;
                    dirt.f = tmppitch;
                }
                if (dirt_skid.a < car->_skid[i]) {
                    dirt_skid.a = (float)car->_skid[i];
                    dirt_skid.f = 1.0f;
                }
                //printf("wheel %d roughness %f dirt vol %f dirt_skid vol %f\n", i, roughness, dirt.a, dirt_skid.a);
            } 
			else if ((s)
				&&
				((strstr(s, "snow"))
					)) {
				if (snow.a < tmpvol) {
					snow.a = tmpvol;
					snow.f = tmppitch;
				}
				if (dirt_skid.a < car->_skid[i]) {
					dirt_skid.a = (float)car->_skid[i];
					dirt_skid.f = 1.0f;
				}
			}
			else {
                if (grass.a < tmpvol) {
                    grass.a = tmpvol;
                    grass.f = tmppitch;
                }/*
                if (grass_skid.a < car->_skid[i]) {
                    grass_skid.a = (float)car->_skid[i];
                    grass_skid.f = 1.0f;
                }
                */
                //printf("wheel %d roughness %f grass vol %f \n", i, roughness, grass.a);
            }

        }

    }

    for (i = 0; i<4; i++) {
        tdble az = car->_yaw;
        tdble Sinz = sin(az);
        tdble Cosz = cos(az);
                
        tdble x = car->priv.wheel[i].relPos.x;
        tdble y = car->priv.wheel[i].relPos.y;
                
        tdble dx = x * Cosz - y * Sinz;
        tdble dy = x * Sinz + y * Cosz;
                
        tdble dux = -car->_yaw_rate * y;
        tdble duy = car->_yaw_rate * x;
                
        dux = dux * Cosz - duy * Sinz;
        duy = dux * Sinz + duy * Cosz;
                
        wheel[i].u[0] = car->pub.DynGCg.vel.x + dux;
        wheel[i].u[1] = car->pub.DynGCg.vel.y + duy;
        wheel[i].u[2] = car->pub.DynGCg.vel.z;
        wheel[i].p[0] = car->pub.DynGCg.pos.x + dx;
        wheel[i].p[1] = car->pub.DynGCg.pos.y + dy;
        wheel[i].p[2] = car->pub.DynGCg.pos.z;
    }
}


void CarSoundData::calculateGearChangeSound (tCarElt* car) {
    if (car->_gear != prev_gear) {
        prev_gear = car->_gear;
        gear_changing = true;
    } else {
        gear_changing = false;
    }

}

void CarSoundData::calculateCollisionSound (tCarElt* car)
{
    skid_metal.a = 0.0f;
    skid_metal.f = 1.0f;
    bottom_crash = false;
    bang = false;
    crash = false;
    if (car->_state & RM_CAR_STATE_NO_SIMU) {
        return;
    }
        
    const int collision = car->priv.collision;
    if (collision) {
        if (collision & SEM_COLLISION) {
            skid_metal.a = (tdble)(car->_speed_xy * 0.01);
            skid_metal.f = (tdble)(.5+0.5*skid_metal.a);
            drag_collision.f = skid_metal.f;
        } else {
            skid_metal.a = 0;
        }

        if (collision & SEM_COLLISION_Z_CRASH) {
            bottom_crash = true;
        }

        if (collision & SEM_COLLISION_Z) {
            bang = true;
        }

		// No crash sound if a dragging sound is already playing.
        if (!(collision & SEM_COLLISION)
			|| ((collision & SEM_COLLISION_XYSCENE) && skid_metal.a > drag_collision.a)) {
            crash = true;
        }
    }

    drag_collision.a = 0.9f*drag_collision.a + skid_metal.a;
    if (drag_collision.a > 1.0f) {
        drag_collision.a = 1.0f;
    }
    skid_metal.a = drag_collision.a;
    skid_metal.f = drag_collision.f;
}
