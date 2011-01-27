//
// C++ Interface: cupsprintwidget
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
