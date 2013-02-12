/**************************************************************************
*   Copyright (C) 2005-2012 by Oleksandr Shneyder                         *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ongetpass.h"

#include "x2goclientconfig.h"

#include <QApplication>
#include <QLocale>

#ifdef CFGCLIENT
#include <onmainwindow.h>
#endif

#ifndef Q_OS_WIN
#include <sys/types.h>
#include <signal.h>
#endif


#include <QPlastiqueStyle>
#include <QMessageBox>
#include <iostream>
#include <QFile>
#include <QProcess>
#include <QLocalSocket>
#include "x2gologdebug.h"
using namespace std;

int x2goMain ( int argc, char *argv[] )
{

    QApplication app ( argc,argv );

#ifndef Q_WS_HILDON
#ifdef Q_OS_LINUX
    app.setStyle ( new QPlastiqueStyle() );
#endif
#endif
    QStringList args;
    if ( argc > 1 )
        args=app.arguments();
    if ( args.count() >1 && args[1]=="--dialog" )
    {
        ONMainWindow::installTranslator();
        QString type=args[2];
        QString caption=args[4];
        caption=caption.replace ( "NX","X2Go" );
        QString text=args[6];
        if ( type=="error" || type=="panic" )
            return QMessageBox::critical ( 0, caption,text );
        if ( type=="ok" )
            return QMessageBox::information ( 0, caption,text );
        if ( type=="yesno" )
        {
            if(text.indexOf("No response received from the remote server")!=-1 &&
                    text.indexOf("Do you want to terminate the current session")!=-1)
            {
                text=QObject::tr("No response received from the remote server. Do you want to terminate the current session?");
                int rez=QMessageBox::question ( 0, caption,text,
                                                QMessageBox::Yes,
                                                QMessageBox::No );
                if(rez==QMessageBox::Yes && args.count()>9)
                {
#ifndef Q_OS_WIN
                    int pid=args[9].toUInt();
                    kill(pid, SIGKILL);
#else
                    QProcess::execute("bash -c \"kill -9 "+args[9]+"\"");
#endif
                }
                return rez;
            }
            else
                return  QMessageBox::question ( 0, caption,text,
                                                QMessageBox::Yes,
                                                QMessageBox::No );
        }
        return -1;
    }

#ifdef CFGCLIENT
    else
    {
        ONMainWindow* mw = new ONMainWindow;
        mw->show();
        return app.exec();
    }
#endif
    return 0;
}
