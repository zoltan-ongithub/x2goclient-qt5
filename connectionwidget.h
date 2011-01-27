//
// C++ Interface: connectionwidget
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include "configwidget.h"
#include <QStringList>
/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class QPushButton;
class QLabel;
class QSlider;
class QStringList;
class ONMainWindow;
class QComboBox;
class QSpinBox;
class ConnectionWidget : public ConfigWidget
{
		Q_OBJECT
	public:
		ConnectionWidget ( QString id, ONMainWindow * mw,
		                   QWidget * parent=0, Qt::WindowFlags f=0 );

		~ConnectionWidget();
		void setDefaults();
		void saveSettings();
	private slots:
		void slot_changePack ( const QString& pc );
	private:
		void loadPackMethods();
	private:
		QLabel* qualiLabel;
		QSlider *spd;
		QStringList qualiList;
		QComboBox* packMethode;
		QSpinBox* quali;
	private:
		void readConfig();
};

#endif
