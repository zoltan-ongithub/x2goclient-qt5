//
// C++ Interface: clicklineedit
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CLICKLINEEDIT_H
#define CLICKLINEEDIT_H

#include <QLineEdit>

/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class ClickLineEdit : public QLineEdit
{
	Q_OBJECT
	public:
		ClickLineEdit ( QWidget * parent = 0 );
		~ClickLineEdit();
	signals:
		void clicked();
#ifdef Q_OS_LINUX
	protected:
		virtual void mouseReleaseEvent ( QMouseEvent * event );
#endif
};

#endif
