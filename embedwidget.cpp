//
// C++ Implementation: embedwidget
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "x2goclientconfig.h"
#ifndef Q_OS_DARWIN

#include <QTimer>
#include <QBoxLayout>
#include "x2gologdebug.h"

#include "embedwidget.h"
#include "onmainwindow.h"



#ifdef Q_OS_LINUX
// #include <QX11EmbedWidget>
#include <QX11EmbedContainer>
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif
#ifdef Q_OS_WIN
#include "wapi.h"
#endif

EmbedWidget::EmbedWidget ()
{
	oldParentSize=QSize ( 0,0 );
	embedContainer=0;
#ifdef Q_OS_WIN
	oldParentPos=QPoint ( 0,0 );
#endif
	childId=0l;
}


EmbedWidget::~EmbedWidget()
{
}

void EmbedWidget::initWidgets()
{
#ifdef Q_OS_LINUX
/*	qx11embedWidget=new QX11EmbedWidget;
	mainLay=new QVBoxLayout ( qx11embedWidget );
	mainLay->setContentsMargins ( 0,0,0,0 );
	( ( ONMainWindow* ) this )->setParent ( qx11embedWidget );
	
	mainLay->addWidget ( ( ONMainWindow* ) this );

	qx11embedWidget->connect ( qx11embedWidget,
	                           SIGNAL ( containerClosed () ),
	                           ( ONMainWindow* ) this,
	                           SLOT ( close() ) );*/

	embedContainer=new QX11EmbedContainer (
	    ( ( ONMainWindow* ) this )->mainWidget() );
	( ( ONMainWindow* ) this )->connect (
	    embedContainer,
	    SIGNAL ( clientClosed() ),
	    ( ONMainWindow* ) this,
	    SLOT ( slotDetachProxyWindow() ) );

	embedContainer->connect (
	    embedContainer,
	    SIGNAL ( clientClosed() ),
	    embedContainer,
	    SLOT ( hide() ) );
#endif
#ifdef Q_OS_WIN
	embedContainer=new QWidget (
	    ( ( ONMainWindow* ) this )->mainWidget() );
#endif
	embedContainer->hide();
	( ( ONMainWindow* ) this )->mainLayout()->addWidget (
	    embedContainer );
}


/*void EmbedWidget::embedInto ( long winId )
{
	parentId=winId;
	oldParentSize=getWindowSize ( parentId );
#ifdef Q_OS_LINUX
	qx11embedWidget->embedInto ( winId );
	QTimer::singleShot ( 100, qx11embedWidget, SLOT ( show() ) );
	qx11embedWidget->move ( 0,0 );
	qx11embedWidget->resize ( oldParentSize );
	x2goDebug<<"embed  window id: "<<qx11embedWidget->winId();
	QString title;
	QTextStream ( &title ) <<"x2goembedded "<<winId;
	qx11embedWidget->setWindowTitle ( title );
#endif
#ifdef Q_OS_WIN
	wapiSetParent (
	    ( HWND ) ( ( ONMainWindow* ) this )->winId(),
	    ( HWND ) winId );
// 	wapiHideFromTaskBar(( HWND ) ( ( ONMainWindow* ) this )->winId());
  	QTimer::singleShot ( 50, ( ( ONMainWindow* ) this ), SLOT ( show() ) );
	QTimer::singleShot ( 100, ( ONMainWindow* ) this,
	                     SLOT ( slotUpdateEmbed() ) );
#endif
	updateTimer = new QTimer ( ( ONMainWindow* ) this );
	( ( ONMainWindow* ) this )->connect ( updateTimer, SIGNAL ( timeout() ),
	                                      ( ONMainWindow* ) this,
	                                      SLOT ( slotUpdateEmbed() ) );
	updateTimer->start ( 500 );
}


void EmbedWidget::slotUpdateEmbed()
{
	QSize sz=getWindowSize ( parentId );
	if ( sz!=oldParentSize )
	{
		oldParentSize=sz;
#ifdef Q_OS_LINUX
		qx11embedWidget->resize ( oldParentSize );
#endif
#ifdef Q_OS_WIN
		moveEmbed ( 0,0 );
		( ( ONMainWindow* ) this )->resize ( oldParentSize );
#endif
	}
#ifdef Q_OS_WIN
	if ( sz==QSize ( 0,0 ) )
	{
		QRect rec;
		if ( !wapiClientRect ( ( HWND ) parentId,rec ) )
		{
			//parent not exist, close x2goclient;
			x2goDebug<<"slotUpdateEmbed: parent closed\n";
			( ( ONMainWindow* ) this )->close();
		}
	}
	if ( ( ( ONMainWindow* ) this )->pos() !=oldParentPos )
	{
		moveEmbed ( 0,0 );
		( ( ONMainWindow* ) this )->resize ( oldParentSize );
	}
	if ( childId )
	{
		QRect rec;
		if ( !wapiWindowRect ( ( HWND ) childId,rec ) )
		{
			x2goDebug<<"slotUpdateEmbed: child closed\n";
			detachClient();
			return;
		}
		if ( ( embedContainer->size() != oldContainerSize ) ||
		        ( rec.topLeft() !=oldChildPos ) )
		{
			moveEmbedChild ( 0,0 );
		}
		wapiUpdateWindow ( ( HWND ) childId );
	}
// 	wapiUpdateWindow ( ( HWND ) ( ( ONMainWindow* ) this )->winId() );
	( ( ONMainWindow* ) this )->update();
#endif
}
*/
QSize EmbedWidget::getWindowSize ( long winId )
{

#ifdef Q_OS_LINUX
	XWindowAttributes atr;
	if ( XGetWindowAttributes ( QX11Info::display(),winId,&atr ) )
		return QSize ( atr.width,atr.height );
	return QSize ( 0,0 );
#endif
#ifdef Q_OS_WIN
	QRect rec;
	if ( wapiClientRect ( ( HWND ) winId,rec ) )
		return rec.size();
	else
		return QSize ( 0,0 );
#endif
}

#ifdef Q_OS_WIN
void EmbedWidget::moveEmbed ( int x, int y )
{
	int vBorder=0;
	int hBorder=0;
	int barHeight=0;

	if ( wapiGetBorders ( ( HWND ) ( ( ONMainWindow* ) this )->winId(),
	                      vBorder, hBorder, barHeight ) )
	{
		oldParentPos.setX ( x-hBorder );
		oldParentPos.setY ( y-vBorder-barHeight );
		( ( ONMainWindow* ) this )->move ( oldParentPos.x(),
		                                   oldParentPos.y() );
	}
}

void EmbedWidget::moveEmbedChild ( int x, int y )
{
	int vBorder=0;
	int hBorder=0;
	int barHeight=0;
	oldContainerSize=embedContainer->size();

	if ( wapiGetBorders ( ( HWND ) childId,
	                      vBorder, hBorder, barHeight ) )
	{

		oldChildPos.setX ( x-hBorder );
		oldChildPos.setY ( y-vBorder-barHeight );
		wapiMoveWindow ( ( HWND ) childId,oldChildPos.x(),
		                 oldChildPos.y(),
		                 oldContainerSize.width() +2*hBorder,
		                 oldContainerSize.height() +2*vBorder+barHeight,
		                 false );
	}
}
#endif


#ifdef Q_OS_LINUX
long EmbedWidget::X11FindWindow ( QString text, long rootWin )
{
	Window    wParent;
	Window    wRoot;
	Window   *child_list;
	unsigned  nChildren;
	long proxyId=0;
	if ( !rootWin )
		rootWin= XDefaultRootWindow ( QX11Info::display() );

	if ( XQueryTree ( QX11Info::display(),rootWin,&wRoot,&wParent,
	                  &child_list,&nChildren ) )
	{
		for ( uint i=0;i<nChildren;++i )
		{
			char *wname;
			if ( XFetchName ( QX11Info::display(),
			                  child_list[i],&wname ) )
			{
				QString title ( wname );
				XFree ( wname );
				if ( title==text )
				{
					proxyId=child_list[i];
					break;
				}
			}
			proxyId=X11FindWindow ( text, child_list[i] );
			if ( proxyId )
				break;
		}
		XFree ( child_list );
	}
	return proxyId;
}
#endif

long EmbedWidget::findWindow ( QString text )
{
#ifdef Q_OS_LINUX
	return X11FindWindow ( text );
#endif
#ifdef Q_OS_WIN
	return ( long ) wapiFindWindow ( 0,text.utf16() );
#endif
}


void EmbedWidget::embedWindow ( long wndId )
{
	childId=wndId;
	embedContainer->show();
#ifdef Q_OS_LINUX
	embedContainer->embedClient ( wndId );
#endif
#ifdef Q_OS_WIN
	wapiSetParent ( ( HWND ) childId,
	                ( HWND ) ( embedContainer->winId() ) );
	oldContainerSize=QSize ( 0,0 );
	oldChildPos=QPoint ( 0,0 );
// 	slotUpdateEmbed();
#endif
}


void EmbedWidget::detachClient()
{
#ifdef Q_OS_LINUX
	if ( embedContainer )
	{
		embedContainer->discardClient();
	}
#endif
#ifdef Q_OS_WIN
	wapiSetParent ( ( HWND ) childId, ( HWND ) 0 );
	( ( ONMainWindow* ) this )->slotDetachProxyWindow();
	if ( childId )
	{
		wapiMoveWindow ( ( HWND ) childId,0,0,
		                 oldContainerSize.width(),
		                 oldContainerSize.height(),true );
	}
#endif
	childId=0;
}

/*
void EmbedWidget::closeEmbedWidget()
{
#ifdef Q_OS_LINUX
	if ( qx11embedWidget )
	{
		qx11embedWidget->close();
		qx11embedWidget=0;
	}
#endif
}
*/
#endif //(Q_OS_DARWIN)
