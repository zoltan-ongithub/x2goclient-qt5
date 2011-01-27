//
// C++ Implementation: clicklineedit
//
// Description: 
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
