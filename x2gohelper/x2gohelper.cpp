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
#include <windows.h>
#include <TlHelp32.h>
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
        //Failed to get system snapshot
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
                if(pentry.th32ProcessID == GetCurrentProcessId())
                {
                    //skip own process
                    continue;
                }
                enumerateFromParent(pentry.th32ProcessID);
                killProcess(pentry.th32ProcessID);
            }
        }
    }
    CloseHandle(hndl);
}

int main(int argc, char* argv[])
{
    if(argc!=2)
    {
        //wrong number of arguments
        return -1;
    }

    DWORD pid=atoi(argv[1]);
    if(!pid)
    {
        //error converting argument to int;
        return -1;
    }

    HANDLE handle=OpenProcess(SYNCHRONIZE,0,pid);
    if(!handle)
    {
        //error open process
        return -1;
    }

    //Process empty message to stop AppStarting cursor
    MSG message;
    PostMessage(NULL, WM_NULL, 0, 0);
    GetMessage(&message, NULL, 0, 0);

    //waiting for process to finish
    if(WaitForSingleObject(handle,INFINITE)!=WAIT_FAILED)
    {
        enumerateFromParent(pid);
    }
    CloseHandle(handle);

    return 0;
}
