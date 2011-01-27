#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0500
#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#endif
#include "x2goclientconfig.h"
#ifdef Q_OS_WIN
#include <windows.h>
#include <sddl.h>
#include "wapi.h"
#include "x2gologdebug.h"


long wapiSetFSWindow ( HWND hWnd, const QRect& desktopGeometry )
{
	long style=GetWindowLong ( hWnd,GWL_STYLE );
	SetWindowPos ( hWnd, HWND_TOP, desktopGeometry.x(),
	               desktopGeometry.y(),
	               desktopGeometry.width(),
	               desktopGeometry.height(),
	               SWP_FRAMECHANGED );
	SetWindowLong ( hWnd, GWL_STYLE,
	                WS_VISIBLE | WS_SYSMENU |
	                WS_CLIPCHILDREN | WS_CLIPSIBLINGS );

	SetWindowPos ( hWnd, HWND_TOP, desktopGeometry.x(),
	               desktopGeometry.y(),
	               desktopGeometry.width(),
	               desktopGeometry.height(),
	               SWP_FRAMECHANGED );
	
	SetForegroundWindow ( hWnd );
	return style;
}

void wapiRestoreWindow( HWND hWnd, long style, const QRect& desktopGeometry )
{
	SetWindowLong ( hWnd, GWL_STYLE,style);	
	SetWindowPos ( hWnd, HWND_TOP, desktopGeometry.x(),
	               desktopGeometry.y(),
	               desktopGeometry.width(),
	               desktopGeometry.height(),
	               SWP_FRAMECHANGED );
}

void wapiHideFromTaskBar ( HWND wnd )
{
	ShowWindow ( wnd, SW_HIDE ) ;
	SetWindowLong ( wnd, GWL_EXSTYLE, GetWindowLong ( wnd, GWL_EXSTYLE )  &
	                ~WS_EX_APPWINDOW );
	SetWindowLong ( wnd, GWL_EXSTYLE, GetWindowLong ( wnd, GWL_EXSTYLE )  |
	                WS_EX_TOOLWINDOW );
	ShowWindow ( wnd, SW_SHOW ) ;
}

HWND wapiSetParent ( HWND child, HWND par )
{
	HWND wn=SetParent ( child,par );
	if ( par )
		SetWindowLong ( child, GWL_STYLE,
		                GetWindowLong ( child, GWL_STYLE ) | WS_CHILD );
	else
		SetWindowLong (
		    child, GWL_STYLE,
		    GetWindowLong ( child, GWL_STYLE ) &~ WS_CHILD );
	SetWindowLong ( child, GWL_STYLE,
	                GetWindowLong ( child, GWL_STYLE ) | WS_POPUP );
	return wn;
}

bool wapiClientRect ( HWND wnd, QRect& rect )
{
	RECT rcWindow;
	if ( GetClientRect ( wnd,&rcWindow ) )
	{
		rect.setCoords ( rcWindow.left,
		                 rcWindow.top,
		                 rcWindow.right,rcWindow.bottom );
		return true;
	}
	return false;
}

bool wapiWindowRect ( HWND wnd, QRect& rect )
{
	RECT rcWindow;
	if ( GetWindowRect ( wnd,&rcWindow ) )
	{
		rect.setCoords ( rcWindow.left,
		                 rcWindow.top,
		                 rcWindow.right,rcWindow.bottom );
		return true;
	}
	return false;
}

bool wapiGetBorders ( HWND wnd, int& vBorder, int& hBorder, int& barHeight )
{
	WINDOWINFO wifo;
	wifo.cbSize=sizeof ( WINDOWINFO );
	if ( !GetWindowInfo ( wnd,&wifo ) )
		return false;
	vBorder=wifo.cxWindowBorders;
	hBorder=wifo.cyWindowBorders;
	TITLEBARINFO bifo;
	bifo.cbSize=sizeof ( TITLEBARINFO );
	if ( !GetTitleBarInfo ( wnd,&bifo ) )
		return false;
	barHeight=bifo.rcTitleBar.bottom-bifo.rcTitleBar.top;

	return true;

}

bool wapiShowWindow ( HWND wnd, wapiCmdShow nCmdShow )
{
	int cmd=WAPI_SHOWNORMAL;
	switch ( nCmdShow )
	{
		case WAPI_FORCEMINIMIZE:cmd=SW_FORCEMINIMIZE;break;
		case WAPI_HIDE:cmd=SW_HIDE;break;
		case WAPI_MAXIMIZE:cmd=SW_MAXIMIZE;break;
		case WAPI_MINIMIZE:cmd=SW_MINIMIZE;break;
		case WAPI_RESTORE:cmd=SW_RESTORE;break;
		case WAPI_SHOW:cmd=SW_SHOW;break;
		case WAPI_SHOWDEFAULT:cmd=SW_SHOWDEFAULT;break;
		case WAPI_SHOWMAXIMIZED:cmd=SW_SHOWMAXIMIZED;break;
		case WAPI_SHOWMINIMIZED:cmd=SW_SHOWMINIMIZED;break;
		case WAPI_SHOWMINNOACTIVE:cmd=SW_SHOWMINNOACTIVE;break;
		case WAPI_SHOWNA:cmd=SW_SHOWNA;break;
		case WAPI_SHOWNOACTIVATE:cmd=SW_SHOWNOACTIVATE;break;
		case WAPI_SHOWNORMAL:cmd=SW_SHOWNORMAL;break;
	}

	return ShowWindow ( wnd, cmd );
}

bool wapiUpdateWindow ( HWND wnd )
{
	return RedrawWindow ( wnd,0,0,RDW_INVALIDATE );
}

bool wapiMoveWindow ( HWND wnd, int x, int y, int width, int height,
                      bool repaint )
{
	return MoveWindow ( wnd, x, y, width, height, repaint );
}

HWND wapiFindWindow ( const ushort * className, const ushort * text )
{
	return FindWindowEx ( 0,0, ( LPCTSTR ) className, ( LPCTSTR ) text );
}


QString wapiShortFileName ( const QString& longName )
{
	long     length = 0;
	TCHAR*   buffer = NULL;

	length = GetShortPathName ( ( LPCTSTR ) longName.utf16(), NULL, 0 );
	if ( !length )
	{
		return QString::null;
	}

	buffer = new TCHAR[length];
	length = GetShortPathName ( ( LPCTSTR ) longName.utf16(),
	                            buffer,length );
	if ( !length )
	{
		delete []buffer;
		return QString::null;
	}
	QString spath=QString::fromUtf16 ( ( const ushort* ) buffer );
	delete []buffer;
	return spath;
}


QString getNameFromSid ( PSID psid, QString* systemName )
{
	DWORD length=0;
	DWORD dlength=0;
	TCHAR* name=0;
	TCHAR* sysName=0;
	SID_NAME_USE eUse;

	LookupAccountSid ( 0,psid,
	                   name,&length,sysName,&dlength,&eUse );
	if ( !length )
	{
		return QString::null;
	}

	name=new TCHAR[length];
	sysName=new TCHAR[dlength];

	if ( ! LookupAccountSid ( 0,psid,
	                          name,&length,sysName,
	                          &dlength,&eUse ) )
	{
		delete []name;
		delete []sysName;
		return QString::null;
	}

	QString strName=QString::fromUtf16 (
	                    ( const ushort* ) name );
	if ( systemName )
		*systemName=QString::fromUtf16 (
		                ( const ushort* ) sysName );
	delete []sysName;
	delete []name;
	return strName;
}

QString getStringFromSid ( PSID psid )
{
	LPTSTR stringSid;
	ConvertSidToStringSid ( psid,
	                        &stringSid );
	QString str=QString::fromUtf16 (
	                ( const ushort* ) stringSid );
	LocalFree ( stringSid );
	return str;
}

bool wapiAccountInfo ( QString* retSid, QString* retUname,
                       QString* primaryGroupSID, QString* primaryGroupName,
                       QString* retSysName )
{
	HANDLE hToken;
	if ( !OpenProcessToken ( GetCurrentProcess(),
	                         TOKEN_QUERY, &hToken ) )
	{
		return false;
	}
	if ( primaryGroupSID || primaryGroupName )
	{
		PTOKEN_PRIMARY_GROUP pGroupInfo=0;
		DWORD dwResult=0;
		DWORD dwSize=0;
		if ( !GetTokenInformation ( hToken, TokenPrimaryGroup,
		                            NULL, dwSize, &dwSize ) )
		{
			dwResult = GetLastError();
			if ( dwResult != ERROR_INSUFFICIENT_BUFFER )
			{
				CloseHandle ( hToken );
				return false;
			}
		}
		pGroupInfo = ( PTOKEN_PRIMARY_GROUP ) GlobalAlloc ( GPTR,
		             dwSize );

		if ( ! GetTokenInformation ( hToken, TokenPrimaryGroup,
		                             pGroupInfo,
		                             dwSize, &dwSize ) )
		{
			if ( pGroupInfo )
				GlobalFree ( pGroupInfo );
			CloseHandle ( hToken );
			return false;
		}

		if ( primaryGroupSID )
		{
			*primaryGroupSID=getStringFromSid (
			                     pGroupInfo->PrimaryGroup );
		}
		if ( primaryGroupName )
		{
			*primaryGroupName=getNameFromSid (
			                      pGroupInfo->PrimaryGroup,
			                      retSysName );
		}

		if ( pGroupInfo )
			GlobalFree ( pGroupInfo );
	}
	if ( retSid || retUname )
	{
		PTOKEN_USER pUserInfo=0;
		DWORD dwResult=0;
		DWORD dwSize=0;

		if ( !GetTokenInformation ( hToken, TokenUser,
		                            NULL, dwSize, &dwSize ) )
		{
			dwResult = GetLastError();
			if ( dwResult != ERROR_INSUFFICIENT_BUFFER )
			{
				CloseHandle ( hToken );
				return false;
			}
		}
		pUserInfo = ( PTOKEN_USER ) GlobalAlloc ( GPTR,
		            dwSize );

		if ( ! GetTokenInformation ( hToken, TokenUser,
		                             pUserInfo,
		                             dwSize, &dwSize ) )
		{
			if ( pUserInfo )
				GlobalFree ( pUserInfo );
			CloseHandle ( hToken );
			return false;
		}

		if ( retSid )
		{
			*retSid=getStringFromSid (
			            pUserInfo->User.Sid );
		}
		if ( retUname )
		{
			*retUname=getNameFromSid (
			              pUserInfo->User.Sid,
			              retSysName );
		}
		if ( pUserInfo )
			GlobalFree ( pUserInfo );
	}
	CloseHandle ( hToken );
	return true;
}

void wapiShellExecute ( const QString& operation, const QString& file,
                        const QString& parameters,
                        const QString& dir, HWND win )
{
	if ( parameters==QString::null )
		ShellExecute ( win, ( LPCTSTR ) ( operation.utf16() ),
		               ( LPCTSTR ) ( file.utf16() ),0,
		               ( LPCTSTR ) ( dir.utf16() ),SW_SHOWNORMAL );
	else
		ShellExecute ( win, ( LPCTSTR ) ( operation.utf16() ),
		               ( LPCTSTR ) ( file.utf16() ),
		               ( LPCTSTR ) ( parameters.utf16() ),
		               ( LPCTSTR ) ( dir.utf16() ),SW_SHOWNORMAL );
}

QString wapiGetDefaultPrinter()
{
	TCHAR *prName;
	DWORD length;
	GetDefaultPrinter ( 0,&length );
	if ( !length )
		return QString::null;
	prName=new TCHAR[length];
	GetDefaultPrinter ( prName,&length );
	if ( !length )
	{
		delete []prName;
		return QString::null;
	}
	QString printer=QString::fromUtf16 ( ( const ushort* ) prName );
	delete []prName;
	return printer;

}

QStringList wapiGetLocalPrinters()
{
	QStringList printers;
	PRINTER_INFO_4 *info_array;
	DWORD sizeOfArray;
	DWORD bufSize=0;
	DWORD sizeNeeded=0;
	EnumPrinters ( PRINTER_ENUM_LOCAL,0,4,NULL,bufSize,
	               &sizeNeeded,&sizeOfArray );
	if ( !sizeNeeded )
	{
		return printers;
	}
	info_array= ( PRINTER_INFO_4* ) new char[sizeNeeded];
	if ( !info_array )
		return printers;
	bufSize=sizeNeeded;
	EnumPrinters ( PRINTER_ENUM_LOCAL,0,4, ( LPBYTE ) info_array,bufSize,
	               &sizeNeeded,&sizeOfArray );
	if ( !sizeNeeded || !sizeOfArray )
	{
		delete []info_array;
		return printers;
	}
	for ( uint i=0;i<sizeOfArray;++i )
	{
		printers<<QString::fromUtf16 (
		    ( const ushort* ) ( info_array[i].pPrinterName ) );
	}
	delete []info_array;
	return printers;
}
#endif
