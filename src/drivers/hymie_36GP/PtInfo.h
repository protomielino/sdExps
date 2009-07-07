/***************************************************************************

    file        : PtInfo.h
    created     : 9 Apr 2006
    copyright   : (C) 2006 Tim Foden

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// PtInfo.h: interface for the PtInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PTINFO_H__31BBC6A3_FE90_497D_BD8F_BA41011DDD46__INCLUDED_)
#define AFX_PTINFO_H__31BBC6A3_FE90_497D_BD8F_BA41011DDD46__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class PtInfo  
{
public:
	PtInfo();
	~PtInfo();

public:
	int			idx;	// index of seg.
	double		t;		// parametric distance to next seg [0..1]
	double		offs;	// offset from middle for the path.
	double		oang;	// global angle.
	double		toL;	// distance to edge of track on left.
	double		toR;	// distance to edge of track on right.
	double		k;		// curvature at point.
	double		spd;	// speed.
};

#endif // !defined(AFX_PTINFO_H__31BBC6A3_FE90_497D_BD8F_BA41011DDD46__INCLUDED_)
