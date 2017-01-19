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

#ifndef _WAPI_H
#define _WAPI_H
#include "x2goclientconfig.h"
#ifdef Q_OS_WIN
#include <QRect>
#include <QPoint>
#include <QString>
#include <QStringList>
#include <QPixmap>

enum wapiCmdShow
{
    WAPI_FORCEMINIMIZE,
    WAPI_HIDE,
    WAPI_MAXIMIZE,
    WAPI_MINIMIZE,
    WAPI_RESTORE,
    WAPI_SHOW,
    WAPI_SHOWDEFAULT,
    WAPI_SHOWMAXIMIZED,
    WAPI_SHOWMINIMIZED,
    WAPI_SHOWMINNOACTIVE,
    WAPI_SHOWNA,
    WAPI_SHOWNOACTIVATE,
    WAPI_SHOWNORMAL
};

enum wapiBtnEvent
{
    WAPI_LBUTTONUP,
    WAPI_LBUTTONDOWN
};

HWND wapiSetParent ( HWND child, HWND par );
bool wapiClientRect ( HWND wnd, QRect& rect );
bool wapiWindowRect ( HWND wnd, QRect& rect );
bool wapiWindowRectWithoutDecoration(HWND wnd, QRect& rect) ;
bool wapiShowWindow ( HWND wnd, wapiCmdShow nCmdShow );
bool wapiUpdateWindow ( HWND wnd );
bool wapiSetWindowText ( HWND wnd, const QString& text);
void wapiSetWindowIcon ( HWND wnd, const QPixmap& icon);
bool wapiMoveWindow ( HWND wnd, int x, int y, int width, int height,
                      bool repaint );
bool wapiGetBorders ( HWND wnd, int& vBorder, int& hBorder, int& barHeight );
void wapiSetCallBackProc ( void ( *prc ) ( wapiBtnEvent, QPoint ) );
void wapiHideFromTaskBar ( HWND wnd );
HWND wapiFindWindow ( const ushort * className, const ushort * text );
QString wapiShortFileName ( const QString& longName );
bool wapiAccountInfo ( QString* retSid, QString* retUname,
                       QString* primaryGroupSID, QString* primaryGroupName,
                       QString* retSysName );
void wapiShellExecute ( const QString& operation, const QString& file,
                        const QString& parameters,
                        const QString& dir,HWND win=0 );
QString wapiGetDefaultPrinter();
QStringList wapiGetLocalPrinters();
long wapiSetFSWindow ( HWND hWnd, const QRect& desktopGeometry );
void wapiRestoreWindow ( HWND hWnd, long style, const QRect& desktopGeometry );
QString wapiGetDriveByLabel(const QString& label);
QString wapiGetUserName();


#endif
#endif
