//
// C++ Interface: settingswidget
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <configwidget.h>

/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class QSpinBox;
class QRadioButton;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QLabel;
class SettingsWidget : public ConfigWidget
{
		Q_OBJECT
	public:
		SettingsWidget ( QString id, ONMainWindow * mw,
		                 QWidget * parent=0, Qt::WindowFlags f=0 );
		~SettingsWidget();
		void setDefaults();
		void saveSettings();
	private slots:
		void slot_sndSysSelected ( int system );
		void slot_sndToggled ( bool val );
		void slot_sndStartClicked();
		void slot_sndDefPortChecked ( bool val );
	private:
		enum {PULSE,ARTS,ESD};
		QSpinBox* width;
		QSpinBox* height;
		QRadioButton* fs;
		QCheckBox* kbd;
		QLineEdit* layout;
		QLineEdit* type;
		QRadioButton* custom;
		QRadioButton* arts;
		QRadioButton* pulse;
		QRadioButton* esd;
		QCheckBox* sound;
		QRadioButton* rbStartSnd;
		QRadioButton* rbNotStartSnd;
		QCheckBox* cbSndSshTun;
		QCheckBox* cbClientPrint;
		QCheckBox* cbDefSndPort;
		QCheckBox* cbSetDPI;
		QLabel* lSndPort;
		QSpinBox* sbSndPort;
		QSpinBox* DPI;
		QLabel* widthLabel;
		QLabel* heightLabel;
		QLabel* layoutLabel;
		QLabel* typeLabel;
	private:
		void readConfig();

};

#endif
