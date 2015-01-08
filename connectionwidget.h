/**************************************************************************
*   Copyright (C) 2005-2015 by Oleksandr Shneyder                         *
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
