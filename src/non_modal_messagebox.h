/**************************************************************************
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

#ifndef NON_MODAL_MESSAGEBOX_H
#define NON_MODAL_MESSAGEBOX_H

#include <QMessageBox>

class Non_Modal_MessageBox {
  public:
    // Use this instead QMessageBox::critical for a non-modal variant.
    // Caveat: this function does return immediately and will NOT return the clicked button.
    static void critical (QWidget *parent, const QString &title,
                          const QString &text,
                          QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                          QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

};

#endif /* !defined (NON_MODAL_MESSAGEBOX_H) */
