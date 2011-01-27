//
// C++ Interface: configwidget
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include <QFrame>
#include "x2goclientconfig.h"

/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class ONMainWindow;
class ConfigWidget : public QFrame
{
	public:
		ConfigWidget ( QString id, ONMainWindow * mv, 
		               QWidget * parent = 0, Qt::WindowFlags f = 0 );

		~ConfigWidget();
	protected:
		bool miniMode;
		bool embedMode;
		QString sessionId;
		ONMainWindow* mainWindow;
};

#endif
