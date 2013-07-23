/**************************************************************************
*   Copyright (C) 2005-2012 by Oleksandr Shneyder                         *
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

#ifndef SESSIONWIDGET_H
#define SESSIONWIDGET_H

#include "configwidget.h"

/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class QLineEdit;
class QSpinBox;
class QPushButton;
class QComboBox;
class QLabel;
class QCheckBox;
class QGroupBox;
class QRadioButton;
class SessionWidget : public ConfigWidget
{
    Q_OBJECT
public:
    SessionWidget ( QString id, ONMainWindow * mv,
                    QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~SessionWidget();
    void setDefaults();
    void saveSettings();
    QString sessionName();
private slots:
    void slot_getIcon();
    void slot_getKey();
    void slot_changeCmd ( int var );
    void slot_rdpOptions();
    void slot_proxyOptions();
    void slot_proxyType();
    void slot_proxySameLogin();
    void slot_proxyGetKey();
public slots:
#ifdef Q_OS_LINUX
    void slot_rdpDirectClicked();
    void slot_emitSettings();
#endif

private:
    enum {KDE,GNOME,LXDE,XFCE,MATE,UNITY,RDP,XDMCP,SHADOW,OTHER,APPLICATION,PUBLISHED};
    QLineEdit* sessName;
    QLineEdit* uname;
    QLineEdit* server;
    QSpinBox* sshPort;
#ifdef Q_OS_LINUX
    QSpinBox* rdpPort;
#endif
    QLineEdit* key;
    QCheckBox* cbAutoLogin;
    QCheckBox* cbKrbLogin;
#ifdef Q_OS_LINUX
    QCheckBox* cbDirectRDP;
#endif
    QString sessIcon;
    QPushButton* icon;
    QLineEdit* cmd;
    QComboBox* cmdCombo;
    QComboBox* sessBox;
    QLabel* leCmdIp;
    QLabel* lPort;
    QLabel* lKey;
    QPushButton* pbAdvanced;
    QString rdpOptions;
    QString rdpServer;
    QString xdmcpServer;
    QPushButton* openKey;
    QGroupBox* proxyBox;
    QCheckBox* cbProxy;
    QRadioButton* rbSshProxy;
    QRadioButton* rbHttpProxy;
    QLineEdit* proxyHost;
    QSpinBox* proxyPort;
    QLineEdit* proxyLogin;
    QLabel* lProxyLogin;
    QCheckBox* cbProxySameUser;
    QCheckBox* cbProxySamePass;
    QCheckBox* cbProxyAutologin;
    QLineEdit* proxyKey;
    QPushButton* pbOpenProxyKey;
    QLabel* proxyKeyLabel;
private:
    void readConfig();
signals:
    void nameChanged ( const QString & );
#ifdef Q_OS_LINUX
    void directRDP(bool);
    void settingsChanged(const QString &, const QString &, const QString &);
#endif
};

#endif
