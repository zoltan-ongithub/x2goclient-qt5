/**************************************************************************
*   Copyright (C) 2005-2016 by Oleksandr Shneyder                         *
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

#include "x2gosettings.h"
#include "x2goclientconfig.h"
#include "x2gologdebug.h"
#include "onmainwindow.h"
#include <QTemporaryFile>
#include <QDir>
#include <QProcessEnvironment>

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
    if (group=="sessions" && ONMainWindow::getSessionConf().length()>0)
    {
        set=new QSettings ( ONMainWindow::getSessionConf(),
                            QSettings::IniFormat );
        return;
    }
#ifndef Q_OS_WIN
    if(!centralSettings())
    {
        set=new QSettings ( ONMainWindow::getHomeDirectory() +
                           "/.x2goclient/"+group,
                          QSettings::NativeFormat );
    }
    else
    {
        QString settingPath="/etc/x2goclient/config/"+qgetenv("USER")+"/";
        QDir d(settingPath);
        if(!d.exists())
        {
            settingPath="/etc/x2goclient/config/All Users/";
        }
//         x2goErrorf(99)<<"CFG PATH:"<<settingPath;
        set=new QSettings ( settingPath+group,
                          QSettings::NativeFormat );
    }
#else
    if ( !ONMainWindow::getPortable() )
    {
        if(! centralSettings())
	{
            set=new QSettings ( "Obviously Nice","x2goclient" );
            set->beginGroup ( group );
	}
	else
	{
	    QSettings setroot("HKEY_LOCAL_MACHINE\\SOFTWARE\\x2goclient\\config",QSettings::NativeFormat);
	    QString setPath="HKEY_LOCAL_MACHINE\\SOFTWARE\\x2goclient\\config\\All Users";
	    QString uname=getenv("USERNAME");
            foreach(QString group, setroot.childGroups())
	    {
	        if(group==uname)
		{
	             setPath="HKEY_LOCAL_MACHINE\\SOFTWARE\\x2goclient\\config\\"+uname;
		     break;
		}
            }
            set=new QSettings ( setPath, QSettings::NativeFormat);
            set->beginGroup ( group );
	}
    }
    else
    {
        set=new QSettings ( ONMainWindow::getHomeDirectory() +
                            "/.x2goclient/"+group,
                            QSettings::IniFormat );
    }
#endif

}


X2goSettings::~X2goSettings()
{
    delete set;
    if (cfgFile)
        delete cfgFile;
}

bool X2goSettings::centralSettings()
{
#ifndef Q_OS_WIN
    QDir d("/etc/x2goclient/config");
    return d.exists();
#else
    QSettings set("HKEY_LOCAL_MACHINE\\SOFTWARE\\x2goclient",QSettings::NativeFormat);
    foreach(QString group,set.childGroups())
    {
      if(group=="config")
      {
	return true;
      }
    }
    return false;
#endif
}
