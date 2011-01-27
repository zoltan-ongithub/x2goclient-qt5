//
// C++ Interface: configdialog
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H
#include "x2goclientconfig.h"

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QSpinBox;
class QPushButton;
class QRadioButton;
class QButtonGroup;
class PrintWidget;
class ConnectionWidget;
class SettingsWidget;
class QTabWidget;
class QGroupBox;
/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class ConfigDialog : public QDialog
{
		Q_OBJECT
	public:
		ConfigDialog ( QWidget * parent,
		               Qt::WFlags f = 0 );
		~ConfigDialog();
#ifdef Q_OS_DARWIN
		static    QString findXDarwin ( QString& version,
		                                QString path="" );
		static    QString retMaxXDarwinVersion ( QString v1,
		        QString v2 );
		static    QString getXDarwinDirectory();
		void    printXDarwinVersionWarning ( QString version );
#endif
#ifdef Q_OS_WIN
		static    QString getCygwinDir ( const QString& dir );
#endif

	private:
		QTabWidget* tabWidg;
		QCheckBox* cbStartEmbed;
		QLineEdit* ldapBase;
		QLineEdit* ldapServer;
		QSpinBox*  port;
		QLineEdit* ldapServer1;
		QSpinBox*  port1;
		PrintWidget* pwid;
		QLineEdit* ldapServer2;
		bool embedMode;
		QSpinBox*  port2;
		QSpinBox*  clientSshPort;
		QPushButton* ok;
		bool advOptionsShown;
		QGroupBox* gbLDAP;

		QPushButton* defaults;
		QPushButton* advancedOptions;

		QLineEdit* leXexec;
		QLineEdit* leCmdOpt;
		QSpinBox* sbDisp;
		QLineEdit* leXexecDir;
		QRadioButton* rbX[3];
		QPushButton* pbOpenExec;
		QButtonGroup* bgRadio;
		ConnectionWidget* conWidg;
		SettingsWidget* setWidg;

		QGroupBox *gbTrayIcon;
		QCheckBox *cbMinimizeTray;
		QCheckBox *cbMaxmizeTray;
		QCheckBox *cbNoClose;
		QCheckBox *cbMinToTray;

	public slots:
		void slot_accepted();
		void slot_checkOkStat();
	private slots:
#ifdef Q_OS_DARWIN
		void slot_selectXDarwin();
		void slot_findXDarwin();
#endif
	private slots:
		void slotAdvClicked();
		void slotDefaults();
};

#endif
