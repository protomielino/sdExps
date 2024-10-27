/***************************************************************************

    file        : simuv41.h
    copyright   : (C) 2011 by Jean-Philippe Meuret
                  (C) 2013 by Wolf-Dieter Beelitz
                  (C) 2023 by Xavier Bertaux
    email       : bertauxx@gmail.com

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
            The "Simu V4.1" physics engine module
*/

#ifndef _SIMUV41_H_
#define _SIMUV41_H_

#include <iphysicsengine.h>

#include <tgf.hpp>


// DLL exported symbols declarator for Windows.
#ifdef _WIN32
# ifdef SIMUV41_DLL
#  define SIMUV41_API __declspec(dllexport)
# else
#  define SIMUV41_API __declspec(dllimport)
# endif
#else
# define SIMUV41_API
#endif


// The C interface of the module.
extern "C" int SIMUV41_API openGfModule(const char* pszShLibName, void* hShLibHandle);
extern "C" int SIMUV41_API closeGfModule();

// The module main class (Singleton, inherits GfModule, and implements IPhysicsEngine).
class SIMUV41_API Simuv41 : public GfModule, public IPhysicsEngine
{
 public:

    // Implementation of IPhysicsEngine.
    virtual void initialize(int nCars, struct Track* pTrack);
    virtual void configureCar(struct CarElt* pCar);
    virtual void reconfigureCar(struct CarElt* pCar);
    virtual void toggleCarTelemetry(int nCarIndex, bool bOn = true);
    virtual void updateSituation(struct Situation *pSituation, double fDeltaTime);
    virtual void updateCar(struct Situation *pSituation, double fDeltaTime, int nCarIndex);
    virtual void setCar(const struct DynPt& dynGCG, int nCarIndex);
    virtual struct DynPt* getCar(int nCarIndex);
    virtual void shutdown();

    // Accessor to the singleton.
    static Simuv41& self();

    // Destructor.
    virtual ~Simuv41();

 protected:

    // Protected constructor to avoid instanciation outside (but friends).
    Simuv41(const std::string& strShLibName, void* hShLibHandle);

    // Make the C interface functions nearly member functions.
    friend int openGfModule(const char* pszShLibName, void* hShLibHandle);
    friend int closeGfModule();

 protected:

    // The singleton.
    static Simuv41* _pSelf;
};

#endif /* _SIMUV41_H_ */
