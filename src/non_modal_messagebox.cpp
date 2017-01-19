/**************************************************************************
*   Copyright (C) 2015-2017 by Mihai Moldovan <ionic@ionic.de>            *
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

#include <QGridLayout>
#include <QSpacerItem>

#include "non_modal_messagebox.h"
#include "x2goutils.h"

// Please look up the documentation in the header file!
void Non_Modal_MessageBox::critical (QWidget *parent, const QString &title,
                                     const QString &text, const QString &informative_text,
                                     bool rich_text,
                                     QMessageBox::StandardButtons buttons,
                                     QMessageBox::StandardButton defaultButton) {
  QMessageBox *msg_box = new QMessageBox (QMessageBox::Critical, title, text, buttons, parent);

  msg_box->setAttribute (Qt::WA_DeleteOnClose);
  msg_box->setDefaultButton (defaultButton);

  if (rich_text) {
    msg_box->setTextFormat (Qt::RichText);
    msg_box->setInformativeText (convert_to_rich_text (informative_text, true));
  }

  // Set to minimum width of 500px.
  QSpacerItem *horizontal_spacer = new QSpacerItem (500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  QGridLayout *grid_layout = (QGridLayout*) (msg_box->layout ());
  grid_layout->addItem (horizontal_spacer, grid_layout->rowCount (), 0, 1, grid_layout->columnCount ());

  // Here's the real magic.
  msg_box->setModal (false);

  msg_box->show ();
  msg_box->raise ();
  msg_box->activateWindow ();
}

void Non_Modal_MessageBox::critical (QWidget *parent, const QString &title,
                                     const QString &text, const QString &informative_text,
                                     QMessageBox::StandardButtons buttons,
                                     QMessageBox::StandardButton defaultButton) {
  Non_Modal_MessageBox::critical (parent, title, text, informative_text, false, buttons, defaultButton);
}

void Non_Modal_MessageBox::critical (QWidget *parent, const QString &title,
                                     const QString &text,
                                     QMessageBox::StandardButtons buttons,
                                     QMessageBox::StandardButton defaultButton) {
  Non_Modal_MessageBox::critical (parent, title, text, QString (), buttons, defaultButton);
}

