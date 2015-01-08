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

#ifndef PRINTWIDGET_H
#define PRINTWIDGET_H

#include "x2goclientconfig.h"
#include "ui_printwidget.h"
/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class CUPSPrintWidget;
class PrintWidget : public QWidget
{
		Q_OBJECT
	public:
		PrintWidget ( QWidget* parent=0l );

		~PrintWidget();
		void saveSettings();
#ifdef Q_OS_WIN
		static bool gsInfo ( QString& version, QString& pdf2ps );
		static bool gsViewInfo ( QString& version, QString& gsprint );
#endif
	private:
		Ui::PrintWidget ui;
		bool printPs;
		bool printStdIn;
#ifndef Q_OS_WIN
		CUPSPrintWidget* pwid;		
#else
		QStringList printers;
		QString defaultPrinter;
#endif

	private slots:
		void slot_editPrintCmd();
	private:
		void loadSettings();
	signals:
		void dialogShowEnabled ( bool );
};

#endif
