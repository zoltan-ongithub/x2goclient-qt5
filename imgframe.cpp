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

#include "imgframe.h"

#include "x2goclientconfig.h"
#include <QResizeEvent>

IMGFrame::IMGFrame(QImage* ,QWidget* parent, Qt::WFlags f) :QFrame(parent,f)
{
	//setBg(img);
}


IMGFrame::~IMGFrame()
{
}

void IMGFrame::setBg(QImage* img)
{
	if(img)
	{
	setAutoFillBackground(true);
	QPalette pal=palette();
	pal.setBrush(QPalette::Window,QBrush(QPixmap::fromImage(*img)));
	setPalette(pal);    
	}
}

void IMGFrame::resizeEvent(QResizeEvent* event)
{
	QFrame::resizeEvent(event);
	emit resized(event->size());
}
