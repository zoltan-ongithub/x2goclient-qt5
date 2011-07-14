//
// C++ Implementation: x2gosettings
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "x2gosettings.h"
#include "x2goclientconfig.h"
#include "x2gologdebug.h"
#include "onmainwindow.h"
#include <QTemporaryFile>

X2goSettings::X2goSettings(QString fileContent, QSettings::Format format)
{
    cfgFile=new QTemporaryFile();
    cfgFile->open();
    QTextStream out(cfgFile);
    out<<fileContent;
    cfgFile->close();
    set=new QSettings ( cfgFile->fileName(),
                        format );
}


X2goSettings::X2goSettings ( QString group )
{
    cfgFile=0l;
#ifndef Q_OS_WIN
    set=new QSettings ( ONMainWindow::getHomeDirectory() +
                        "/.x2goclient/"+group,
                        QSettings::NativeFormat );
#else
    if ( !ONMainWindow::getPortable() )
    {
        set=new QSettings ( "Obviously Nice","x2goclient" );
        set->beginGroup ( group );
// 		x2goDebug<<"settings in reg";
    }
    else
    {
        set=new QSettings ( ONMainWindow::getHomeDirectory() +
                            "/.x2goclient/"+group,
                            QSettings::IniFormat );
// 		x2goDebug<<"settings in ini:"<<
        ONMainWindow::getHomeDirectory() +
        "/.x2goclient/"+group;

    }
#endif

}


X2goSettings::~X2goSettings()
{
    delete set;
    if (cfgFile)
        delete cfgFile;
}


