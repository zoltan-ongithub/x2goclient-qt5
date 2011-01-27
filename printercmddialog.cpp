//
// C++ Implementation: printercmddialog
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "printercmddialog.h"
#include "x2goclientconfig.h"
#ifdef WINDOWS
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
#ifdef WINDOWS
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
