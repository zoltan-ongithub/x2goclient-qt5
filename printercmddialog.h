//
// C++ Interface: printercmddialog
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
