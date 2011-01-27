//
// C++ Interface: sharewidget
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
