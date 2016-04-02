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

#ifndef IMGFRAME_H
#define IMGFRAME_H

#include "x2goclientconfig.h"
#include <QFrame>

/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/

class QResizeEvent;

class IMGFrame : public QFrame
{
	Q_OBJECT
public:
	IMGFrame(QImage* img,QWidget* parent=0, Qt::WindowFlags f=0);
    ~IMGFrame();
    void setBg(QImage* img);
    virtual void resizeEvent(QResizeEvent* event);
signals:
    void resized (const QSize);
};

#endif
