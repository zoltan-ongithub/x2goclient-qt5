//
// C++ Implementation: printdialog
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "printdialog.h"
#include <QPushButton>
#include "printwidget.h"
#include <QBoxLayout>
#include "x2gologdebug.h"
#include <QMessageBox>

PrintDialog::PrintDialog ( QWidget* parent, Qt::WindowFlags f )
		: QDialog ( parent,f )
{
	x2goDebug<<"starting print dialog"<<endl;
	ui.setupUi ( this );
	ui.buttonBox->button ( QDialogButtonBox::Ok )->setText (
	    tr ( "Print" ) );
	pwidg=new PrintWidget ( this );
	( ( QVBoxLayout* ) ( layout() ) )->insertWidget ( 0,pwidg );

	//x2goclient can stay under the nxagent window
	//we must start it as toplevel window and be shure
	//that x2goclient window will not be activated
	//so we must start print dialog as window
	setWindowFlags ( Qt::Window|Qt::WindowStaysOnTopHint );
	connect ( pwidg,SIGNAL ( dialogShowEnabled ( bool ) ),
	          this,SLOT ( slot_dlgShowEnabled ( bool ) ) );
}


PrintDialog::~PrintDialog()
{
}



void PrintDialog::accept()
{
	pwidg->saveSettings();
	QDialog::accept();
}

void PrintDialog::slot_dlgShowEnabled ( bool enable )
{
	if ( !enable )
		QMessageBox::warning ( this, tr ( "You've deactivated the x2go "
		                                  "client printing dialog." ),
		                       tr ( "You may reactivate this dialog "
		                            "using the x2goclient settings "
		                            "dialog (Menu -> Options -> "
		                            "Settings)" ) );
}
