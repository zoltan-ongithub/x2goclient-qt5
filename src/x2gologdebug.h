/**************************************************************************
*   Copyright (C) 2005-2017 by Oleksandr Shneyder                         *
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

#ifndef X2GOLOGDEBUG_H
#define X2GOLOGDEBUG_H
#include <QTextStream>
#include <QFile>
#include "x2goclientconfig.h"
#include "onmainwindow.h"
/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/

#define __x2goPrefix      "x2go-"
#define __x2goDebugPrefix "DEBUG-"
#define __x2goInfoPrefix  "INFO-"
#define __x2goWarningPrefix  "WARNING-"
#define __x2goErrorPrefix "ERROR-"
#define __x2goPostfix     "> "

#ifdef LOGFILE
class X2goLogDebug: public QTextStream
{
public:
    X2goLogDebug();
    ~X2goLogDebug();

 private:
     QFile logFile;
};

#define __x2goDebug   X2goLogDebug()<<"\n"
#define __x2goInfo    X2goLogDebug()<<"\n"
#define __x2goWarning X2goLogDebug()<<"\n"
#define __x2goError   X2goLogDebug()<<"\n"

#else

#include <QDebug>

#define __x2goDebug   qDebug().nospace()
#define __x2goInfo    qDebug().nospace()
#define __x2goWarning qWarning().nospace()
#define __x2goError   qCritical().nospace()

#endif //LOGFILE

#define x2goDebugf        __x2goDebug  <<__x2goPrefix<<__x2goDebugPrefix  <<__FILE__<<":"<<__LINE__<<__x2goPostfix
#define x2goInfof(NUM)    __x2goInfo   <<__x2goPrefix<<__x2goInfoPrefix   <<NUM                    <<__x2goPostfix
#define x2goWarningf(NUM) __x2goWarning<<__x2goPrefix<<__x2goWarningPrefix<<NUM                    <<__x2goPostfix
#define x2goErrorf(NUM)   __x2goError  <<__x2goPrefix<<__x2goErrorPrefix  <<NUM                    <<__x2goPostfix

#define x2goDebug      if (ONMainWindow::debugging) x2goDebugf

#endif //X2GOLOGDEBUG_H
