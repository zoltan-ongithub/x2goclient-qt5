/**************************************************************************
*   Copyright (C) 2005-2014 by Oleksandr Shneyder                         *
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
#else
#include <iostream>
#endif


#include <QPlastiqueStyle>
#include <QMessageBox>
#include <iostream>
#include <QFile>
#include <QProcess>
#include <QLocalSocket>
#include "x2gologdebug.h"
using namespace std;


#ifdef Q_OS_WIN
#include <TlHelp32.h>

void killProcess(DWORD pid)
{
    HANDLE handle=OpenProcess(PROCESS_TERMINATE,0,pid);
    if(!handle)
    {
//         qCritical()<<"failed to open process";
        return;
    }
    if(!TerminateProcess(handle,0))
    {
//         qCritical()<<"failed to terminate process";
        return;
    }
    CloseHandle(handle);
//     qCritical()<<pid<<" terminated";
}

void enumerateFromParent(DWORD pid)
{
    HANDLE hndl=CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    if(hndl==INVALID_HANDLE_VALUE)
    {
        qCritical()<<"failed to get system snapshot";
        return;
    }
    PROCESSENTRY32 pentry;
    pentry.dwSize = sizeof( PROCESSENTRY32 );
    if( Process32First(hndl,&pentry))
    {
        while(Process32Next(hndl,&pentry))
        {
            if(pid==pentry.th32ParentProcessID)
            {
                enumerateFromParent(pentry.th32ProcessID);
//                 qCritical()<<"terminating "<<pentry.th32ProcessID<<":"<<QString::fromWCharArray ( pentry.szExeFile);
                killProcess(pentry.th32ProcessID);
            }
        }
    }
    CloseHandle(hndl);
}
#endif

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
#ifdef CFGCLIENT
        ONMainWindow::installTranslator();
#endif
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
#ifdef Q_OS_WIN
        if(args.count()<2 || args[1].indexOf("--child-process")==-1)
        {
            QProcess proc;
            QString executable=args[0];
            args.pop_front();
            args.push_front("--child-process");
            proc.start(executable, args);
            if(!proc.waitForStarted(4000))
            {
                qCritical()<<"Can't start process";
                return -1;
            }
            DWORD pid=proc.pid()->dwProcessId;
            while(!proc.waitForFinished(300))
            {
                QString err=proc.readAllStandardError();
                QString out=proc.readAllStandardOutput();
                std::cerr<<err.toStdString();
                std::cout<<out.toStdString();
            }
            enumerateFromParent(pid);
            return 0;
        }
#endif //Q_OS_WIN
        ONMainWindow* mw = new ONMainWindow;
        mw->show();
        return app.exec();
    }
#endif
    return 0;
}






