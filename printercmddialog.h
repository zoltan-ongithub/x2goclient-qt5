/**************************************************************************
*   Copyright (C) 2005-2014 by Oleksandr Shneyder                         *
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

#ifndef PRINTERCMDDIALOG_H
#define PRINTERCMDDIALOG_H

#include <QDialog>
#include "ui_printercmddialog.h"
/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class PrinterCmdDialog : public QDialog
{
		Q_OBJECT
	public:
		PrinterCmdDialog ( QString* cmd, bool* stdinpr,
		                   bool* ps, QWidget* parent=0l );

		~PrinterCmdDialog();
	private:
		Ui::PrinterCmdDialog ui;
		bool* printStdIn;
		bool* printPs;
		QString* printCmd;

	private slots:
		void slot_ok();
};

#endif
