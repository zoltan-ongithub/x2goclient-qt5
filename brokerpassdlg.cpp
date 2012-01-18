/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2005-2012 Oleksandr Shneyder

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "brokerpassdlg.h"
#include <QPushButton>

BrokerPassDlg::BrokerPassDlg(QWidget* parent, Qt::WindowFlags f): QDialog(parent, f)
{
    setupUi(this);
    statusLabel->setText(QString::null);
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

BrokerPassDlg::~BrokerPassDlg()
{

}

void BrokerPassDlg::slotPassChanged()
{
    bool passEq=false;
    if (lePass1->text()!=lePass2->text())
    {
        passEq=false;
        statusLabel->setText(tr("Passwords do not match"));
    }
    else
    {
        passEq=true;
        statusLabel->setText(QString::null);
    }
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(passEq &&
            lePass1->text().size()>0 &&
            leOldPas->text().size()>0);
}

void BrokerPassDlg::accept()
{
    QDialog::accept();
}

void BrokerPassDlg::reject()
{
    QDialog::reject();
}

QString BrokerPassDlg::newPass()
{
    return lePass1->text();
}

QString BrokerPassDlg::oldPass()
{
    return leOldPas->text();
}
