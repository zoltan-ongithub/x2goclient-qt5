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

#ifndef SHAREWIDGET_H
#define SHAREWIDGET_H

#include <configwidget.h>

/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class QTreeView;
class QStandardItemModel;
class QLabel;
class QCheckBox;
class QComboBox;

class ShareWidget : public ConfigWidget
{
		Q_OBJECT
	public:
		ShareWidget ( QString id, ONMainWindow * mw,
		              QWidget * parent=0, Qt::WindowFlags f=0 );
		~ShareWidget();
		void setDefaults();
		void saveSettings();
	private slots:
		void slot_openDir();
		void slot_addDir();
		void slot_delDir();
		void slot_convClicked();
	private:
		QTreeView* expTv;
		QStandardItemModel* model;
		QLabel *ldir;
		QCheckBox* cbFsSshTun;
		QCheckBox* cbFsConv;
		QComboBox* cbFrom;
		QComboBox* cbTo;
		QLabel* lFrom;
		QLabel* lTo;

	private:
		void readConfig();
		void loadEnc ( QComboBox* cb );

};

#endif
