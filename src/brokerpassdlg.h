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

#ifndef BROKERPASSDLG_H
#define BROKERPASSDLG_H


#include <QDialog>
#include "ui_brokerpassdialog.h"

class BrokerPassDlg: public QDialog, private Ui_BrokerPassDialogUi
{
    Q_OBJECT
public:
    BrokerPassDlg(QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~BrokerPassDlg();
    virtual void accept();
    virtual void reject();
    QString oldPass();
    QString newPass();
private slots:
    void slotPassChanged();
};

#endif // BROKERPASSDLG_H
