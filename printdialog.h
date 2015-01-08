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

#ifndef PRINTDIALOG_H
#define PRINTDIALOG_H

#include <QDialog>
#include "ui_printdialog.h"
/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class PrintWidget;
class PrintDialog : public QDialog
{
		Q_OBJECT
	public:
		PrintDialog ( QWidget* par=0, Qt::WindowFlags f = 0 );
		~PrintDialog();
	private:
		Ui::PrintDialog ui;
		PrintWidget* pwidg;

	public slots:
		void accept();
private slots:
    void slot_dlgShowEnabled(bool);
};

#endif
