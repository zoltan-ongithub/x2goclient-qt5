/**************************************************************************
*   Copyright (C) 2005-2015 by Oleksandr Shneyder                         *
*   o.shneyder@phoca-gmbh.de                                              *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
***************************************************************************/

#if !defined(_X2GOCLIENT_CONFIG_H_)
#define _X2GOCLIENT_CONFIG_H_

#include <stdio.h>
#include <qconfig.h>
#include <qglobal.h>


//#define LOGFILE QDir::homePath()+"/x2goclient.log"

#if !defined Q_OS_WIN
#define USELDAP
#endif

#ifdef Q_OS_WIN
#undef USELDAP
#endif

#if defined Q_WS_HILDON
#undef USELDAP
#endif

#endif
