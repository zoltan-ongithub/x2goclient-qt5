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

#ifndef XSETTINGSWIDGET_H
#define XSETTINGSWIDGET_H

#include <QWidget>

#include "ui_xsettingsui.h"

class XSettingsWidget : public QWidget, private Ui_XSettingsWidgetUI
{
    Q_OBJECT
public:
    XSettingsWidget(QWidget* parent = 0);
    virtual ~XSettingsWidget();
    void setDefaults();
    void saveSettings();
private slots:
    void slotSetExecutable();
};

#endif // XSETTINGSWIDGET_H
