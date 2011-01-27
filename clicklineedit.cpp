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
	setFocus();
}
#endif
