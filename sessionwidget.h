//
// C++ Interface: sessionwidget
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
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

private:
    enum {KDE,GNOME,LXDE,UNITY,RDP,XDMCP,SHADOW,OTHER,APPLICATION,PUBLISHED};
    QLineEdit* sessName;
    QLineEdit* uname;
    QLineEdit* server;
    QSpinBox* sshPort;
    QLineEdit* key;
    QCheckBox* cbAutoLogin;
    QCheckBox* cbKrbLogin;
    QString sessIcon;
    QPushButton* icon;
    QLineEdit* cmd;
    QComboBox* cmdCombo;
    QComboBox* sessBox;
    QLabel* leCmdIp;
    QPushButton* pbAdvanced;
    QString rdpOptions;
    QString rdpServer;
    QString xdmcpServer;
private:
    void readConfig();
signals:
    void nameChanged ( const QString & );
};

#endif
