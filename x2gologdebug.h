//
// C++ Interface: x2gologdebug
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef X2GOLOGDEBUG_H
#define X2GOLOGDEBUG_H
#include <QTextStream>
#include <QFile>
#include "x2goclientconfig.h"
/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
#ifdef LOGFILE
class X2goLogDebug: public QTextStream
{
public:
    X2goLogDebug();
    ~X2goLogDebug();
 private:
     QFile logFile;
};
#define x2goDebug X2goLogDebug()
#else
#include <QDebug>
#define x2goDebug qDebug()
#endif //LOGFILE
#endif //X2GOLOGDEBUG_H
