//
// C++ Interface: imgframe
//
// Description: 
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@treuchtlingen.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IMGFRAME_H
#define IMGFRAME_H

#include "x2goclientconfig.h"
#include <QFrame>

/**
	@author Oleksandr Shneyder <oleksandr.shneyder@treuchtlingen.de>
*/

class QResizeEvent;

class IMGFrame : public QFrame
{
	Q_OBJECT
public:
	IMGFrame(QImage* img,QWidget* parent=0, Qt::WFlags f=0);
    ~IMGFrame();
    void setBg(QImage* img);
    virtual void resizeEvent(QResizeEvent* event);
signals:
    void resized (const QSize);
};

#endif
