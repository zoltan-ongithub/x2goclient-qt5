//
// C++ Interface: printwidget
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
#ifdef WINDOWS
		static bool gsInfo ( QString& version, QString& pdf2ps );
		static bool gsViewInfo ( QString& version, QString& gsprint );
		static QString getPdfCmd();
		static QString getAppCmd ( const QString& app );
#endif
	private:
		Ui::PrintWidget ui;
		bool printPs;
		bool printStdIn;
#ifndef WINDOWS
		CUPSPrintWidget* pwid;
#endif

	private slots:
		void slot_editPrintCmd();
	private:
		void loadSettings();
	signals:
		void dialogShowEnabled ( bool );
};

#endif
