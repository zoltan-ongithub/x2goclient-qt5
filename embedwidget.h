//
// C++ Interface: embedwidget
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef EMBEDWIDGET_H
#define EMBEDWIDGET_H
#include "x2goclientconfig.h"


#ifndef Q_OS_DARWIN


class QVBoxLayout;
#ifndef Q_OS_WIN
// class QX11EmbedWidget;
class QX11EmbedContainer;
#endif

class QTimer;

class EmbedWidget
{
	public:
		EmbedWidget ();
		~EmbedWidget();
/*		void embedInto ( long winId );
		void slotUpdateEmbed();*/
		long findWindow ( QString text );
		void embedWindow ( long wndId );
		void detachClient();
	private:
		long parentId;
		long childId;
		QSize oldParentSize;
		QTimer *updateTimer;

#ifdef Q_OS_LINUX
// 		QX11EmbedWidget* qx11embedWidget;
		QX11EmbedContainer* embedContainer;
		QVBoxLayout* mainLay;
#endif
#ifdef Q_OS_WIN
		QWidget* embedContainer;
		QPoint oldParentPos;
		QPoint oldChildPos;
		QSize oldContainerSize;
#endif
	private:
		QSize getWindowSize ( long winId );
#ifdef Q_OS_LINUX
		long X11FindWindow ( QString text, long rootWin=0 );
#endif
#ifdef Q_OS_WIN
		void moveEmbed ( int x, int y );
		void moveEmbedChild ( int x, int y );
#endif
	protected:
		void initWidgets();
// 		void closeEmbedWidget();
};
#endif //(Q_OS_DARWIN)
#endif
