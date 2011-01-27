/***************************************************************************
 *   Copyright (C) 2005 by Oleksandr Shneyder   *
 *   oleksandr.shneyder@treuchtlingen.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef USERBUTTON_H
#define USERBUTTON_H

#include "x2goclientconfig.h"
#include <QPushButton>

/**
@author Oleksandr Shneyder
*/

class QPixmap;
class ONMainWindow;
class UserButton : public QPushButton
{
		Q_OBJECT
	public:
		UserButton ( ONMainWindow* wnd, QWidget *parent,
		             QString username, QString fullName,
		             QPixmap& foto, QPalette& backGround,
		             int width=0,int height=0 );
		~UserButton();
		QString username() {return user;}
		QString fullName() {return fname;}
		const QPixmap& foto() {return image;}
		const QPixmap& background() {return bg;}

	private:
		QString user;
		QString fname;
		QPixmap image;
		QPixmap bg;
	private slots:
		void slotClicked();
	signals:
		void userSelected ( UserButton* );
};
#endif
