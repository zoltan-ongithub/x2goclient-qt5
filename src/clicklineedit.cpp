/**************************************************************************
*   Copyright (C) 2005-2015 by Oleksandr Shneyder                         *
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

#include "clicklineedit.h"
#include "x2gologdebug.h"

#include <QTimer>

ClickLineEdit::ClickLineEdit(QWidget * parent)
 : QLineEdit(parent)
{
	
}


ClickLineEdit::~ClickLineEdit()
{
}

#ifdef Q_OS_LINUX
void ClickLineEdit::mouseReleaseEvent ( QMouseEvent * event )
{
	QLineEdit::mouseReleaseEvent(event);
 	emit clicked();
 	setFocus(Qt::MouseFocusReason);
}
// void ClickLineEdit::focusInEvent ( QFocusEvent * event )
// {
// 	QLineEdit::focusInEvent(event);
// 	x2goDebug<<"focus in";
// }
// 
// void ClickLineEdit::focusOutEvent ( QFocusEvent * event )
// {
// 	QLineEdit::focusOutEvent(event);
// 	x2goDebug<<"focus out";
// }

#endif
