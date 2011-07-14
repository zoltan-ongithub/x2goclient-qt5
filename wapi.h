#ifndef _WAPI_H
#define _WAPI_H
#include "x2goclientconfig.h"
#ifdef Q_OS_WIN
#include <QRect>
#include <QPoint>
#include <QString>
#include <QStringList>
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
bool wapiShowWindow ( HWND wnd, wapiCmdShow nCmdShow );
bool wapiUpdateWindow ( HWND wnd );
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



#endif
#endif
