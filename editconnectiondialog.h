//
// C++ Interface: editconnectiondialog
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

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
		void slot_directRDP(bool direct);
};

#endif
