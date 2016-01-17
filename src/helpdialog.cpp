/**************************************************************************
*   Copyright (C) 2005-2016 by Oleksandr Shneyder                         *
*   o.shneyder@phoca-gmbh.de                                              *
*   Copyright (C) 2015-2016 by Mihai Moldovan <ionic@ionic.de>            *
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

#include <QtGlobal>
#include "helpdialog.h"
#include "x2goutils.h"

HelpDialog::HelpDialog (QWidget *parent): QDialog (parent) {
  setupUi (this);
}

void HelpDialog::setText (QString text) {
  /* Try real hard to get a monospaced font. */
  QFont font ("monospace");

  if (!font_is_monospaced (font)) {
#if QT_VERSION >= 0x040700
    font.setStyleHint (QFont::Monospace);
#else
    font.setStyleHint (QFont::TypeWriter);
#endif
  }

  if (!font_is_monospaced (font)) {
    font.setFamily ("Courier New");
  }

  if (!font_is_monospaced (font)) {
    font.setFamily ("Courier");
  }

  /* If the font is not monospaced by now, there's not much else we can do... */
  font.setPointSize (10);

  plainTextEdit->setFont (font);

  plainTextEdit->setTabStopWidth (30);
  plainTextEdit->setWordWrapMode (QTextOption::NoWrap);
  plainTextEdit->setPlainText (text);
}

