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
class QPushButton;
class QMainWindow;
class QGroupBox;


class SettingsWidget : public ConfigWidget
{
    Q_OBJECT
public:
    SettingsWidget ( QString id, ONMainWindow * mw,
                     QWidget * parent=0, Qt::WindowFlags f=0 );
    ~SettingsWidget();
    void setDefaults();
    void saveSettings();
#ifdef Q_OS_LINUX
    void setDirectRdp(bool direct);
public slots:
    void setServerSettings(QString server, QString port, QString user);
    void updateCmdLine();
#endif

private slots:
    void slot_sndSysSelected ( int system );
    void slot_sndToggled ( bool val );
    void slot_sndStartClicked();
    void slot_sndDefPortChecked ( bool val );
    void slot_identDisplays();
    void slot_hideIdentWins();
    void slot_kbdClicked();
private:
    enum {PULSE,ARTS,ESD};
    QSpinBox* width;
    QSpinBox* height;
    QSpinBox* displayNumber;
    QRadioButton* fs;
    QRadioButton* custom;
    QRadioButton* display;
    QRadioButton* maxRes;
    QRadioButton* arts;
    QRadioButton* pulse;
    QRadioButton* esd;
    QRadioButton* rbKbdAuto;
    QRadioButton* rbKbdNoSet;
    QRadioButton* rbKbdSet;
    QGroupBox* gbKbdString;
    QLineEdit* leModel;
    QLineEdit* leLayout;
    QLineEdit* leVariant;
    QCheckBox* sound;
    QRadioButton* rbStartSnd;
    QRadioButton* rbNotStartSnd;
    QCheckBox* cbSndSshTun;
    QCheckBox* cbClientPrint;
    QCheckBox* cbDefSndPort;
    QCheckBox* cbSetDPI;
    QCheckBox* cbXinerama;
    QLabel* lSndPort;
    QSpinBox* sbSndPort;
    QSpinBox* DPI;
    QLabel* widthLabel;
    QLabel* heightLabel;
    QLabel* layoutLabel;
    QLabel* typeLabel;
    QLabel* lDisplay;
    bool multiDisp;
    QPushButton* pbIdentDisp;
    QList <QMainWindow*> identWins;
    QGroupBox *kgb;
    QGroupBox *sbgr;

#ifdef Q_OS_LINUX
    QGroupBox *rdpBox;
    QRadioButton* rRdesktop;
    QRadioButton* rXfreeRDP;
    QLineEdit* cmdLine;
    QLineEdit* params;
    QString server;
    QString user;
    QString port;
#endif
    QFrame* hLine1;
    QFrame* hLine2;

private:
    void readConfig();
};

#endif
