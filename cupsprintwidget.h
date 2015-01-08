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

#ifndef CUPSPRINTWIDGET_H
#define CUPSPRINTWIDGET_H

#include <QWidget>
#include "x2goclientconfig.h"
#ifndef Q_OS_WIN
#include "cupsprint.h"
#include "ui_cupsprintwidget.h"
/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/

class CUPSPrintWidget : public QWidget
{
		Q_OBJECT
	public:
		CUPSPrintWidget ( QWidget* parent =0 );
		~CUPSPrintWidget();
		void savePrinter();
	private:
		CUPSPrint* m_cups;
		Ui::CUPSPrintWidget ui;

	private slots:
		void slot_printerSelected ( int index );
		void slot_printerSettings();
};

#endif
#endif
