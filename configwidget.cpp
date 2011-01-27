//
// C++ Implementation: configwidget
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "configwidget.h"
#include "onmainwindow.h"

ConfigWidget::ConfigWidget ( QString id, ONMainWindow * mw,
                             QWidget * parent, Qt::WindowFlags f )
		: QFrame ( parent,f )
{
	sessionId=id;
	mainWindow=mw;
	miniMode=mw->retMiniMode();	
	embedMode=embedMode= mw->getEmbedMode();
	if(embedMode)
		sessionId="embedded";
}


ConfigWidget::~ConfigWidget()
{
}


