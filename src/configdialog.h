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
class MediaWidget;
class QTabWidget;
class QGroupBox;
#if defined (Q_OS_WIN) && defined (CFGCLIENT )
class XSettingsWidget;
#endif
/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class ConfigDialog : public QDialog
{
    Q_OBJECT
public:
    ConfigDialog ( QWidget * parent,
                   Qt::WindowFlags f = 0 );
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
    MediaWidget* mediaWidget;

#if defined (Q_OS_WIN) && defined (CFGCLIENT)
    XSettingsWidget* xsetWidg;
#endif

#if defined (Q_OS_WIN) || defined (Q_OS_DARWIN)
    QCheckBox *cbNoRecord;
#endif /* defined (Q_OS_WIN) || defined (Q_OS_DARWIN) */

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
