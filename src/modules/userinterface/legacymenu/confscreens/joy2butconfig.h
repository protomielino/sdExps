/***************************************************************************

    file                 : joy2butconfig.h
    created              : Wed Mar 21 23:06:29 CET 2001
    copyright            : (C) 2001 by Eric Espie
    email                : Eric.Espie@torcs.org

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

    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
*/

#ifndef _JOY2BUTCONFIG_H_
#define _JOY2BUTCONFIG_H_

#include "confscreens.h"


/* nextMenu : the menu to go to when "next" button is pressed */
extern void *Joy2butCalMenuInit(void *prevMenu, void *nextMenu, tCmdInfo *cmd, int maxcmd);

#endif /* _JOY2BUTCONFIG_H_ */
