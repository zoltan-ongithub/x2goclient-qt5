/**************************************************************************
*   Copyright (C) 2005-2015 by Oleksandr Shneyder                         *
*   o.shneyder@phoca-gmbh.de                                              *
*   Copyright (C) 2015 by Mihai Moldovan <ionic@ionic.de>                 *
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

#include "helpdialog.h"

HelpDialog::HelpDialog(QWidget* parent): QDialog(parent)
{
    setupUi(this);
}

void HelpDialog::setText(QString text)
{
    QFont font ("monospace");
    font.setStyleHint (QFont::Monospace);
    plainTextEdit->setFont (font);

    plainTextEdit->setTabStopWidth(30);
    plainTextEdit->setWordWrapMode(QTextOption::NoWrap);
    plainTextEdit->setPlainText(text);
}

