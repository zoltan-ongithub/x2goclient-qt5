/**************************************************************************
*   Copyright (C) 2005-2017 by Oleksandr Shneyder                         *
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

#ifndef MEDIAWIDGET_H
#define MEDIAWIDGET_H

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


class MediaWidget : public ConfigWidget
{
    Q_OBJECT
public:
    MediaWidget ( QString id, ONMainWindow * mw,
                  QWidget * parent=0, Qt::WindowFlags f=0 );
    ~MediaWidget();
    void setDefaults();
    void saveSettings();

private slots:
    void slot_sndSysSelected ( int system );
    void slot_sndToggled ( bool val );
    void slot_sndStartClicked();
    void slot_sndDefPortChecked ( bool val );
private:
    enum {PULSE,ARTS,ESD};
    QRadioButton* arts;
    QRadioButton* pulse;
    QRadioButton* esd;
    QCheckBox* sound;
    QRadioButton* rbStartSnd;
    QRadioButton* rbNotStartSnd;
    QCheckBox* cbSndSshTun;
    QCheckBox* cbClientPrint;
    QCheckBox* cbDefSndPort;
    QLabel* lSndPort;
    QSpinBox* sbSndPort;
    QGroupBox *sbgr;
private:
    void readConfig();
};

#endif
