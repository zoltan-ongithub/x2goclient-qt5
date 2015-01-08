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

#include "printercmddialog.h"
#include "x2goclientconfig.h"
#ifdef Q_OS_WIN
      #include "printwidget.h"
#endif
PrinterCmdDialog::PrinterCmdDialog ( QString* cmd, bool* stdinpr,
                                     bool* ps, QWidget* parent )
		: QDialog ( parent )
{
	ui.setupUi ( this );
	printCmd=cmd;
	printStdIn=stdinpr;
	printPs=ps;
	ui.leCmd->setText ( *printCmd );
	if ( *printStdIn )
		ui.rbStdIn->setChecked ( true );
	else
		ui.rbParam->setChecked ( true );
	if ( *printPs )
		ui.rbPS->setChecked ( true );
	else
		ui.rbPDF->setChecked ( true );
	connect ( ui.buttonBox,
	          SIGNAL ( accepted() ),this,SLOT ( slot_ok() ) );
#ifdef Q_OS_WIN
	QString txt=tr ( "Please enter your customized or"
	                         " individual printing command.\n"
	                         "Example:\n");
	
	QString ver,path;
	if(PrintWidget::gsViewInfo(ver,path))
		txt+=path+" -query -color";
	else
		txt+=tr("<Path to gsprint.exe> -query -color");
	ui.label->setText (txt);
	if(!PrintWidget::gsInfo(ver,path))
	{
		ui.rbPDF->setChecked ( true );
		ui.rbPS->setEnabled(false);
	}
#endif

}


PrinterCmdDialog::~PrinterCmdDialog()
{
}



void PrinterCmdDialog::slot_ok()
{
	*printCmd=ui.leCmd->text();
	*printPs=ui.rbPS->isChecked();
	*printStdIn=ui.rbStdIn->isChecked();
	accept();
}
