//
// C++ Implementation: imgframe
//
// Description: 
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@treuchtlingen.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
