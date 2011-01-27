//
// C++ Interface: printdialog
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
