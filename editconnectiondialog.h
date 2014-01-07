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

#ifndef EDITCONNECTIONDIALOG_H
#define EDITCONNECTIONDIALOG_H

#include "x2goclientconfig.h"
#include <QStringList>
#include <QDialog>

class QLineEdit;
class QPushButton;
class QCheckBox;
class QSpinBox;
class QComboBox;
class QRadioButton;
class QSlider;
class QLabel;
class QTabWidget;
class ONMainWindow;
class QStandardItemModel;
class QTreeView;

/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class SessionWidget;
class ConnectionWidget;
class SettingsWidget;
class ShareWidget;

class EditConnectionDialog : public QDialog
{
		Q_OBJECT
	public:
		EditConnectionDialog ( QString id, QWidget * par,  int ind=0,
		                       Qt::WFlags f = 0 );
		~EditConnectionDialog();
	private:
		QTabWidget *fr;
		SessionWidget* sessSet;
		ConnectionWidget* conSet;
		SettingsWidget* otherSet;
		ShareWidget* exportDir;


	private slots:
		void slot_changeCaption ( const QString& newName );
		void slot_accepted();
		void slot_default();
#ifdef Q_OS_LINUX		
		void slot_directRDP(bool direct);
#endif
};

#endif
