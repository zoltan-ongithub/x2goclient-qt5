//
// C++ Implementation: x2gologdebug
//
// Description: 
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@treuchtlingen.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "x2gologdebug.h"
#include "x2goclientconfig.h"
#ifdef LOGFILE
#include <QDir>
X2goLogDebug::X2goLogDebug():QTextStream()
{
    logFile.setFileName(LOGFILE);
    if(logFile.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append));
    {
          setDevice(&logFile);
    }
}


X2goLogDebug::~X2goLogDebug()
{
 	logFile.close();
}
#endif
