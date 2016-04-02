/**************************************************************************
*   Copyright (C) 2005-2016 by Oleksandr Shneyder                         *
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

#include "SVGFrame.h"
#include "x2goclientconfig.h"

#include <QPainter>
#include <QTimer>
#include "x2gologdebug.h"
#include <QResizeEvent>



SVGFrame::SVGFrame ( QString fname,bool st,QWidget* parent,
                     Qt::WindowFlags f ) :QFrame ( parent,f )
{
	empty=false;
#ifdef Q_OS_WIN
	parentWidget=0;
#endif
	if ( fname==QString::null )
		empty=true;
	if ( !empty )
	{
		repaint=true;
		drawImg=st;
		renderer=new QSvgRenderer ( this );
		renderer->load ( fname );

		if ( drawImg )
		{
			setAutoFillBackground ( true );
			QPalette pal=palette();
			QImage img ( renderer->defaultSize(),
			             QImage::Format_ARGB32_Premultiplied );
			QPainter p ( &img );
			renderer->render ( &p );
			pal.setBrush ( QPalette::Window,QBrush ( QPixmap::fromImage ( img ) ) );
			setPalette ( pal );
		}
		else
		{
			QTimer *timer = new QTimer ( this );
			connect ( timer, SIGNAL ( timeout() ), this, SLOT ( update() ) );
			if ( renderer->animated() )
			{
				timer->start ( 1000/renderer->framesPerSecond() );
				x2goDebug<<"Animated, fps:"<<renderer->framesPerSecond() <<endl;
			}
		}
	}
}

SVGFrame::SVGFrame ( QWidget* parent,
                     Qt::WindowFlags f ) :QFrame ( parent,f )
{
	repaint=false;
	empty=true;
}

void SVGFrame::paintEvent ( QPaintEvent* event )
{
	if ( repaint && !drawImg && !empty )
	{
		QPainter p ( this );
		p.setViewport ( 0, 0, width(), height() );
		p.eraseRect ( 0, 0, width(), height() );
		renderer->render ( &p );
	}
	QFrame::paintEvent ( event );
}


void SVGFrame::resizeEvent ( QResizeEvent* event )
{
	QFrame::resizeEvent ( event );
	emit resized ( event->size() );
	if ( drawImg && event->size().width() >0 && event->size().height() >0 &&!empty )
	{
		QPalette pal=palette();
		QImage img ( event->size(),QImage::Format_ARGB32_Premultiplied );
		QPainter p ( &img );
		if ( p.isActive() )
			renderer->render ( &p );
		pal.setBrush ( QPalette::Window,QBrush ( QPixmap::fromImage ( img ) ) );
		setPalette ( pal );
	}
}


QSize SVGFrame::sizeHint() const
{
	if ( !empty )
		return renderer->defaultSize();
	return QFrame::sizeHint();
}

void SVGFrame::loadBg ( QString fl )
{
	renderer->load ( fl );
	update();
}

#ifdef Q_OS_WIN
#include "wapi.h"
void SVGFrame::mousePressEvent ( QMouseEvent * event )
{
/*	if ( isVisible() )
	{
		int vBorder;
		int hBorder;
		int barHeight;
		if ( parentWidget )
			wapiGetBorders ( parentWidget->winId(), vBorder, hBorder, barHeight );
		x2goDebug<<"svg frame: "<<event->pos() <<
		":"<<mapFromGlobal ( event->pos() ) <<":"<<barHeight<<":"<<vBorder<<":"
		<<hBorder<<":"<<pos() <<endl;
		QMouseEvent * nevent=new QMouseEvent(event->type(), QPoint(0,0),
				event->button(), event-> buttons(), event->modifiers());
		QFrame::mousePressEvent ( nevent );
		return;
	}*/
	QFrame::mousePressEvent ( event );
}
#endif
