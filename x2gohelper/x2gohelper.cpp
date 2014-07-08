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
#include <iostream>
using namespace std;
#include <windows.h>
#include <TlHelp32.h>

#include <QStringList>
#include <QProcess>
#include <QDebug>

void killProcess(DWORD pid)
{
    HANDLE handle=OpenProcess(PROCESS_TERMINATE,0,pid);
    if(!handle)
    {
        return;
    }
    if(!TerminateProcess(handle,0))
    {
        return;
    }
    CloseHandle(handle);
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
                killProcess(pentry.th32ProcessID);
            }
        }
    }
    CloseHandle(hndl);
}

int main(int argc, char* argv[])
{
    QStringList args;
    //argv[0] is allways "x2gohelper.exe"
    for(int i=1; i< argc; ++i)
    {
        args<<argv[i];
    }
    QProcess proc;
    QString executable="x2goclient.exe";
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
