/**************************************************************************
*   Copyright (C) 2005-2017 by Oleksandr Shneyder                         *
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

#include "printdialog.h"
#include <QPushButton>
#include "printwidget.h"
#include <QBoxLayout>
#include "x2gologdebug.h"
#include <QMessageBox>

PrintDialog::PrintDialog ( QWidget* parent, Qt::WindowFlags f )
		: QDialog ( parent,f )
{
	x2goDebug<<"Starting print dialog."<<endl;
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
	x2goDebug<<"Closing print dialog."<<endl;
}



void PrintDialog::accept()
{
	pwidg->saveSettings();
	QDialog::accept();
}

void PrintDialog::slot_dlgShowEnabled ( bool enable )
{
	if ( !enable )
		QMessageBox::warning ( this, tr ( "You've deactivated the X2Go "
		                                  "Client printing dialog." ),
		                       tr ( "You may reactivate this dialog "
		                            "using the X2Go Client settings "
		                            "dialog. To do so, follow this path "
		                            "in the menu bar: Options -> "
		                            "Settings" ) );
}
