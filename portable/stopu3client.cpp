/**************************************************************************
*   Copyright (C) 2010 by Oleksandr Shneyder                              *
*   oleksandr.shneyder@obviously-nice.de                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
/* 

     this is helper program to stop running in "portable" mode X2Go Client
     before U3 USB-drive will be unmounted

*/
#include <windows.h>
int main()
{

       HWND wid=FindWindowEx(0,0,0,"X2Go client - U3");
       while(wid)
       {
           HWND prevWid=wid;
           wid=FindWindowEx(0,wid,0,"X2Go client - U3");
           PostMessage(prevWid,WM_CLOSE,0,0);
       }
       return 0;
}
