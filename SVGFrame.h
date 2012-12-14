/**************************************************************************
*   Copyright (C) 2005-2012 by Oleksandr Shneyder                         *
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

#ifndef SVGFRAME_H
#define SVGFRAME_H
#include "x2goclientconfig.h"


#include <QFrame>
#include <QtSvg/QSvgRenderer>

class SVGFrame: public QFrame
{

		Q_OBJECT
	public:
		SVGFrame ( QString fname, bool st, QWidget* parent=0, Qt::WFlags f=0 );
		SVGFrame ( QWidget* parent=0, Qt::WFlags f=0 );
		void setRepaintable ( bool val )
		{
			repaint=val;
		}
		void loadBg ( QString fl );
		virtual QSize sizeHint() const;
	private:
		QSvgRenderer* renderer;
		bool repaint;
		bool drawImg;
		bool empty;
	protected:
		virtual void paintEvent ( QPaintEvent* event );
		virtual void resizeEvent ( QResizeEvent* event );
#ifdef Q_OS_WIN
		virtual void mousePressEvent ( QMouseEvent * event );
	private:
		QWidget* parentWidget;
	public:
		void setMainWidget ( QWidget* widg )
		{
			parentWidget=widg;
		}

#endif

	signals:
		void resized ( const QSize );
};

#endif

